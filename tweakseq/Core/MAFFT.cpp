//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017  Michael J. Wouters, Merridee A. Wouters
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <QtDebug>
#include "DebuggingInfo.h"

#include <QDomDocument>
#include <QLabel>
#include <QMenu>
#include <QProcess>
#include <QThread>

#include "BoolProperty.h"
#include "DoubleProperty.h"
#include "FileProperty.h"
#include "FilePropertyInput.h"
#include "IntProperty.h"
#include "MAFFT.h"
#include "PropertiesDialog.h"
#include "XMLHelper.h"

#define NSTRATEGIES 11
static const char *strategies[NSTRATEGIES] =
{
	"Auto",
	"L-INS-i",
	"G-INS-i",
	"E-INS-i",
	"FFT-NS-i",
	"FFT-NS-2 (default)",
	"FFT-NS-1",
	"NW-NS-i",
	"NW-NS-2",
	"NW-NS-PartTree-1",
	"Custom"
};

//
//	Public
//

MAFFT::MAFFT()
{
	init();
}

MAFFT::~MAFFT()
{
}
		
void MAFFT::makeCommand(QString &fin, QString &, QString &exec, QStringList &arglist)
{
	if (!customCommand_.isEmpty()){
		parseCustomCommand(exec,arglist);
		// now replace the input and output files
		for (int a=0;a<arglist.size();a++){
			if (arglist.at(a) == "<fin>")
				arglist[a]=fin;
		}
		return;
	}
	
	exec = executable();
	arglist << "--auto" << "--thread" << "-1" << fin; // output is to stdout
}

void MAFFT::makeDefaultCommand(QString &fin, QString &, QString &exec, QStringList &arglist)
{
	exec = executable();
	arglist << "--auto" << "--thread" << "-1" << fin;
}

void MAFFT::writeSettings(QDomDocument &doc,QDomElement &parentElem)
{
	QDomElement pelem = doc.createElement("alignment_tool");
	parentElem.appendChild(pelem);
	XMLHelper::addElement(doc,pelem,"name",name());
	XMLHelper::addElement(doc,pelem,"preferred",(preferred() ? "yes":"no"));
	XMLHelper::addElement(doc,pelem,"strategy",strategies[strategy_]);
	if (!customCommand_.isEmpty()){
		XMLHelper::addElement(doc,pelem,"customcommand",customCommand_);
	}
	QDomElement propElem = doc.createElement("properties");
	pelem.appendChild(propElem);
	saveXML(doc,propElem);
}

void MAFFT::readSettings(QDomDocument &doc)
{
	QDomNodeList nl = doc.elementsByTagName("alignment_tool");
	for (int i=0;i<nl.count();++i){
		QDomNode gNode = nl.item(i);
		QDomElement elem = gNode.firstChildElement();
		bool foundIt=false;
		
		while (!elem.isNull()){
			if (elem.tagName() == "name"){
				if (elem.text() != name_)
					break;
				else{
					foundIt=true;
				}
			}
			elem=elem.nextSiblingElement();
		}
		
		if (foundIt){
			
			elem = gNode.firstChildElement();
			while (!elem.isNull()){
				if (elem.tagName() == "preferred"){
					setPreferred(elem.text() == "yes");
				}
				else if (elem.tagName() == "customcommand"){
					setCommand(elem.text());
				}
				else if (elem.tagName() == "strategy"){
					for (int a=0;a<NSTRATEGIES;a++){
						if (elem.text()==strategies[a]){
							strategy_=a;
							break;
						}
					}
				}
				else if (elem.tagName()=="properties"){
					qDebug() << trace.header(__PRETTY_FUNCTION__) << "reading properties";
					readXML(doc,elem);
				}
				elem=elem.nextSiblingElement();
			}
		}
		
	}
	
	getVersion();
	
}

PropertiesDialog * MAFFT::propertiesDialog(QWidget *parent)
{
	PropertiesDialog *pd = new PropertiesDialog(parent,this,"MAFFT settings");
	
	pd->addTab("General");
	QLabel *txt = new QLabel(name() + " version " + version(),pd->currContainerWidget());
	pd->addCustomWidget(txt);
	pd->addFileInput("path to executable",executable_,FilePropertyInput::OpenFile,0,0);
	pd->addIntInput("[--thread] number of threads",numThreads_);
	
	pd->addTab("Algorithm");
	pd->addOptionInput("--auto",auto_);
	pd->addOptionInput("--globalpair",globalPair_);
	pd->addOptionInput("--localpair",localPair_);
	pd->addOptionInput("--genafpair",genafpair_);
	pd->addIntInput("--retree",retree_);
	pd->addIntInput("--maxiterate",maxIterate_);
	pd->addOptionInput("--nofft",nofft_);
	pd->addOptionInput("--parttree",parttree_);
	
	pd->addTab("Parameters");
	pd->addDoubleInput("--ep",ep_);
	
	pd->addTab("Output");
	
	return pd;
}

QMenu * MAFFT::createCustomMenu()
{

	if (!customMenu_){
		customMenu_ = new QMenu("MAFFT algorithm");
		QActionGroup *ag = new QActionGroup(this);
		ag->setExclusive(true);
		for (int a=0;a<NSTRATEGIES;a++){
			actions_.append(customMenu_->addAction(strategies[a]));
			ag->addAction(actions_.at(a));
			actions_.at(a)->setCheckable(true);
			actions_.at(a)->setChecked(false);
		}
		connect(customMenu_,SIGNAL(triggered(QAction*)),this,SLOT(setStrategy(QAction*)));
	}
	actions_.at(strategy_)->setChecked(true);
	return customMenu_;
}

void MAFFT::setStrategy(QAction *action)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << action->text();
	for (int a=0;a<NSTRATEGIES;a++){
		if (action->text()==strategies[a]){
			setPropertiesFromStrategy(a);
			break;
		}
	}
	
	
}

//		
//	Private
//	

void MAFFT::init()
{
	name_="MAFFT";
	version_="";
	setExecutable("/usr/local/bin/mafft");
	usesStdOut_=true; // annoying
	customMenu_=NULL;
	idealThreadCount_ = QThread::idealThreadCount();
	strategy_=MAFFT::DEFAULT_FFT_NS_2;
	
	numThreads_=registerIntProperty(NULL,"numThreads",1,999);
	numThreads_->setDefaultValue(-1);
	numThreads_->setValue(idealThreadCount_);
	
	// This is the minimum set of properties to generate the standard strategies
	
	localPair_  = new BoolProperty(NULL,this,"localPair",false);localPair_->setDefaultValue(false);
		strategyProperties_.append(localPair_);
	globalPair_ = new BoolProperty(NULL,this,"globalPair",false);globalPair_->setDefaultValue(false);
		strategyProperties_.append(globalPair_ );
	maxIterate_= new IntProperty(NULL,this,"maxIterate",0);maxIterate_->setDefaultValue(0);
		strategyProperties_.append(maxIterate_);
	ep_= new DoubleProperty(NULL,this,"ep",0.0);ep_->setDefaultValue(0); // > v 6.626
		strategyProperties_.append(ep_);
	genafpair_= new BoolProperty(NULL,this,"genafpair",false) ;genafpair_->setDefaultValue(false);
		strategyProperties_.append(genafpair_);
	retree_= new IntProperty(NULL,this,"retree",2) ;retree_->setDefaultValue(2);
		strategyProperties_.append(retree_);
	nofft_ = new BoolProperty(NULL,this,"nofft",false);nofft_->setDefaultValue(false);
		strategyProperties_.append(nofft_);
	parttree_ = new BoolProperty(NULL,this,"parttree",false);parttree_->setDefaultValue(false);
		strategyProperties_.append(parttree_ );
	auto_ = new BoolProperty(NULL,this,"auto",false);auto_->setDefaultValue(false);
		strategyProperties_.append(auto_);
		
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "threads " << idealThreadCount_;
}

void MAFFT::getVersion()
{
	QProcess getver;
	qDebug() << executable_;
	getver.start(executable(), QStringList() << "--version");
	if (getver.waitForStarted()){
		getver.waitForReadyRead();
		getver.waitForFinished();
		version_ = QString(getver.readAllStandardError());
		qDebug() << version_;
		version_=version_.trimmed();
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << name_ << " " << version_;
}

void MAFFT::setPropertiesFromStrategy(int s)
{
	if (strategy_==s) return;
	
	for (int s=0;s<strategyProperties_.size();s++)
		strategyProperties_.at(s)->setToDefaultValue();
	
	strategy_=s;
	
	switch (strategy_)
	{
		case L_INSI_i:
			localPair_->setValue(true);
			maxIterate_->setValue(1000);
			break;
		case G_INSI_i:
			globalPair_->setValue(true);
			maxIterate_->setValue(1000);
			break;
		case E_INSI_i:
			ep_->setValue(0.0);
			genafpair_->setValue(true);
			maxIterate_->setValue(1000);
			break;
		case FFT_NS_i_2:
			retree_->setValue(2);
			maxIterate_->setValue(2);
			break;
		case DEFAULT_FFT_NS_2:
			retree_->setValue(2);
			maxIterate_->setValue(0);
			break;
		case FFT_NS_1:
			retree_->setValue(1);
			maxIterate_->setValue(0);
			break;
		case NW_NS_i:
			retree_->setValue(2);
			maxIterate_->setValue(2);
			nofft_->setValue(true);
			break;
		case NW_NS_2:
			retree_->setValue(2);
			maxIterate_->setValue(0);
			nofft_->setValue(true);
			break;
		case NW_NS_PartTree_1:
			retree_->setValue(1);
			maxIterate_->setValue(0);
			nofft_->setValue(true);
			parttree_->setValue(true);
			break;
		case Auto:
			auto_->setValue(true);
			break;
		case Custom:
			break;				 
	}
}

	
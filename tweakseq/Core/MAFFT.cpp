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

void MAFFT::writeSettings(QDomDocument &doc,QDomElement &parentElem)
{
	QDomElement pelem = doc.createElement("alignment_tool");
	parentElem.appendChild(pelem);
	XMLHelper::addElement(doc,pelem,"name",name());
	XMLHelper::addElement(doc,pelem,"preferred",(preferred() ? "yes":"no"));
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
				if (elem.tagName() == "customcommand"){
					setCommand(elem.text());
				}
				if (elem.tagName()=="properties"){
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
	pd->endGridLayout();
	
	return pd;
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
	
	idealThreadCount_ = QThread::idealThreadCount();
	
	//reorder_ = registerBoolProperty(NULL,"reorder",0);
	//reorder_->setValue(false);
	
	//gapOpeningPenalty_ = registerDoubleProperty(NULL,"gapOpeningPenalty");
	//gapOpeningPenalty_->setValue(1.53);
	
	//gapExtensionPenalty_ = registerDoubleProperty(NULL,"gapExtensionPenalty");
	//gapExtensionPenalty_->setValue(0.0);
	 
	numThreads_=registerIntProperty(NULL,"numThreads",1,999);
	numThreads_->setValue(idealThreadCount_);
	
	//maxIterations_=registerIntProperty(NULL,"numThreads",0,1000);;
	//maxIterations_->setValue(0);
	
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


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

#include "ClustalO.h"
#include "BoolProperty.h"
#include "FileProperty.h"
#include "FilePropertyInput.h"
#include "IntProperty.h"
#include "PropertiesDialog.h"

#include "XMLHelper.h"
//
//	Public
//

ClustalO::ClustalO()
{
	init();
}

ClustalO::~ClustalO()
{
}
		
void ClustalO::makeCommand(QString &fin, QString &fout, QString &exec, QStringList &arglist)
{
	if (!customCommand_.isEmpty()){
		parseCustomCommand(exec,arglist);
		// now replace the input and output files
		for (int a=0;a<arglist.size();a++){
			if (arglist.at(a) == "<fin>")
				arglist[a]=fin;
			else if (arglist.at(a) == "<fout>")
				arglist[a]=fout;
		}
		return;
	}
	
	// otherwise  ...
	
	exec = executable();
	// The basic command
	arglist << "--force" << "-v" << "--outfmt=fa" << "--output-order=tree-order" << "-i" << fin << "-o" << fout;
	//Optional stuff
	if (pileup_->value()) arglist << "--pileup";
	if (full_->value()) arglist << "--full";
	if (full_iter_->value()) arglist << "--full-iter";
	if (useKimura_->value()) arglist << "--use-kimura";
	if (percentID_->value()) arglist << "--percent-id";
	
	if (!distMatrixIn_->fileName().isEmpty()) arglist << ("--distmat-in="+distMatrixIn_->fileName());
}

void ClustalO::makeDefaultCommand(QString &fin, QString &fout, QString &exec, QStringList &arglist)
{
	exec = executable();
	arglist << "--force" << "-v" << "--outfmt=fa" << "--output-order=tree-order" << "-i" << fin << "-o" << fout;
}


void ClustalO::writeSettings(QDomDocument &doc,QDomElement &parentElem)
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

void ClustalO::readSettings(QDomDocument &doc)
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

PropertiesDialog * ClustalO::propertiesDialog(QWidget *parent)
{
	PropertiesDialog *pd = new PropertiesDialog(parent,this,"ClustalO settings");
	pd->addTab("General");
	QLabel *txt = new QLabel(name() + " version " + version(),pd->currContainerWidget());
	pd->addCustomWidget(txt);
	pd->addFileInput("path to executable",executable_,FilePropertyInput::OpenFile,0,0);
	
	pd->addTab("Sequence input");
	pd->addOptionInput("[--dealign] dealign input sequences",dealign_);
	
	pd->addTab("Clustering");
	
	pd->beginGridLayout();
	
	pd->addFileInput("[--distmat-in] pairwise distance matrix input file",distMatrixIn_,FilePropertyInput::OpenFile,0,0);
	pd->addFileInput("[--distmat-out] pairwise distance matrix output file",distMatrixOut_,FilePropertyInput::OpenFile,0,0);
	pd->addFileInput("[--guidetree-in] guide tree input file",guideTreeIn_,FilePropertyInput::OpenFile,0,0);
	pd->addFileInput("[--guidetree-out] guide tree output file",guideTreeOut_,FilePropertyInput::OpenFile,0,0);
	pd->addFileInput("[--clustering-out] clustering output file",clusteringOut_,FilePropertyInput::OpenFile,0,0);
	pd->addFileInput("[--posterior-out] posterior probability output file",posteriorOut_,FilePropertyInput::OpenFile,0,0);
	
	pd->addIntInput("[--cluster-size] soft maximum of sequences in sub-clusters",clusterSize_,true);
	
	pd->addOptionInput("[--pileup] sequentially align sequences",pileup_);
	pd->addOptionInput("[--full] use full distance matrix for guide-tree calculation",full_);
	pd->addOptionInput("[--full-iter] use full distance matrix for guide-tree calculation during iteration",full_iter_);
	pd->addOptionInput("[--use-kimura] use Kimura distance correction for aligned sequences",useKimura_);
	pd->addOptionInput("[--use-percent-id] convert distances into percent identities",percentID_);
	
	pd->endGridLayout();
	
	return pd;
}


//		
//	Private
//	

void ClustalO::init()
{
	name_="clustalo";
	version_="";
	setExecutable("/usr/local/bin/clustalo");
	
	dealign_ = registerBoolProperty(NULL,"dealign",0);
	dealign_->setValue(false);
	
	// Clustering
	distMatrixIn_=registerFileProperty(NULL,"distMatrixIn",0);
	distMatrixOut_=registerFileProperty(NULL,"distMatrixOut",0);;
	guideTreeIn_=registerFileProperty(NULL,"guideTreeIn",0);;
	guideTreeOut_=registerFileProperty(NULL,"guideTreeOut",0);;
	clusteringOut_=registerFileProperty(NULL,"clusteringOut",0);
	posteriorOut_=registerFileProperty(NULL,"posteriorOut",0);;
	
	clusterSize_=registerIntProperty(NULL,"clusterSize",0,99,0);
	
	pileup_ = registerBoolProperty(NULL,"pileup",0);
	pileup_->setValue(false);
	full_   = registerBoolProperty(NULL,"full",0);
	full_->setValue(false);
	full_iter_   = registerBoolProperty(NULL,"full-iter",0);
	full_iter_->setValue(false);
	useKimura_   = registerBoolProperty(NULL,"useKimura",0);
	useKimura_->setValue(false);
	percentID_   = registerBoolProperty(NULL,"percentID",0);
	percentID_->setValue(false);
}

void ClustalO::getVersion()
{
	QProcess getver;
	getver.start(executable(), QStringList() << "--version");
	if (getver.waitForStarted()){
		getver.waitForReadyRead();
		getver.waitForFinished();
		version_ = QString(getver.readAll());
		version_=version_.trimmed();
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << name_ << " " << version_;
}


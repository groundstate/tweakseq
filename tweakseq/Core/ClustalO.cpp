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
#include <QProcess>

#include "ClustalO.h"
#include "BoolProperty.h"
#include "FileProperty.h"
#include "FilePropertyInput.h"
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
	exec = executable_;
	arglist << "--force" << "-v" << "--outfmt=fa" << "--output-order=tree-order" << "-i" << fin << "-o" << fout;
}

void ClustalO::writeSettings(QDomDocument &doc,QDomElement &parentElem)
{
	QDomElement pelem = doc.createElement("alignment_tool");
	parentElem.appendChild(pelem);
	XMLHelper::addElement(doc,pelem,"name",name());
	XMLHelper::addElement(doc,pelem,"path",executable());
	XMLHelper::addElement(doc,pelem,"preferred",(preferred() ? "yes":"no"));
}

void ClustalO::readSettings(QDomDocument &doc)
{
	QDomNodeList nl = doc.elementsByTagName("alignment_tool");
	for (int i=0;i<nl.count();++i){
		QDomNode gNode = nl.item(i);
		QDomElement elem = gNode.firstChildElement();
		while (!elem.isNull()){
			if (elem.tagName() == "name"){
				if (elem.text() != name_)
					break;
			}
			if (elem.tagName() == "path"){
				executable_=elem.text();
			}
			if (elem.tagName() == "preferred"){
				setPreferred(elem.text() == "yes");
			}
			elem=elem.nextSiblingElement();
		}
	}
	
	getVersion();
	
}

PropertiesDialog * ClustalO::propertiesDialog(QWidget *parent)
{
	PropertiesDialog *pd = new PropertiesDialog(parent,this,"ClustalO settings");
	pd->addTab("General");
	pd->addFileInput("path to executable",execPath_,FilePropertyInput::OpenFile,0,0);
	pd->addTab("Clustering");
	pd->beginGridLayout();
	pd->addOptionInput("[--pileup] sequentially align sequences",pileup_);
	pd->addOptionInput("[--full] use full distance matrix for guide-tree calculation",full_);
	pd->addOptionInput("[--full-iter] use full distance matrix for guide-tree calculation during iteration",full_iter_);
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
	executable_="/usr/local/bin/clustalo";
	execPath_ = registerFileProperty(executable_,"exe",0);
	
	// Clustering
	pileup_ = registerBoolProperty(NULL,"pileup",0);
	pileup_->setValue(false);
	full_   = registerBoolProperty(NULL,"full",0);
	full_->setValue(false);
	full_iter_   = registerBoolProperty(NULL,"full-iter",0);
	full_iter_->setValue(false);
}

void ClustalO::getVersion()
{
	QProcess getver;
	getver.start(executable_, QStringList() << "--version");
	if (getver.waitForStarted()){
		getver.waitForReadyRead();
		getver.waitForFinished();
		version_ = QString(getver.readAll());
		version_=version_.trimmed();
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << name_ << " " << version_;
}


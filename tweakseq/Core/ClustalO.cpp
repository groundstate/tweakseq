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
}

void ClustalO::readSettings(QDomDocument &doc)
{
	QDomNodeList nl = doc.elementsByTagName("alignment_tool");
	if (nl.count() == 1){
		QDomNode gNode = nl.item(0);
		QDomElement elem = gNode.firstChildElement();
		while (!elem.isNull()){
			if (elem.tagName() == "path"){
				executable_=elem.text();
			}
			elem=elem.nextSiblingElement();
		}
	}
	
	getVersion();
	
}

//		
//	Private
//	

void ClustalO::init()
{
	name_="clustalo";
	version_="";
	executable_="/usr/local/bin/clustalo";
	
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
	qDebug() << trace.header() << name_ << " " << version_;
}


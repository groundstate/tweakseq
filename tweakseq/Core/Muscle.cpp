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

#include "Muscle.h"
#include "XMLHelper.h"

//
//	Public
//

Muscle::Muscle()
{
	init();
}

Muscle::~Muscle()
{
}
		
void Muscle::makeCommand(QString &fin, QString &fout, QString &exec, QStringList &arglist)
{
	exec = executable_;
	arglist <<  "-in" << fin << "-out" << fout;
}

void Muscle::writeSettings(QDomDocument &doc,QDomElement &parentElem)
{
	QDomElement pelem = doc.createElement("alignment_tool");
	parentElem.appendChild(pelem);
	XMLHelper::addElement(doc,pelem,"name",name());
	XMLHelper::addElement(doc,pelem,"path",executable());
	XMLHelper::addElement(doc,pelem,"preferred",(preferred() ? "yes":"no"));
}

void Muscle::readSettings(QDomDocument &doc)
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

//		
//	Private
//	

void Muscle::init()
{
	name_="MUSCLE";
	version_="";
	executable_="/usr/local/bin/muscle3.8.31_i86linux64";
	
}

void Muscle::getVersion()
{
	QProcess getver;
	getver.start(executable_, QStringList() << "-version");
	if (getver.waitForStarted()){
		getver.waitForReadyRead();
		getver.waitForFinished();
		version_ = QString(getver.readAll());
		QStringList sl = version_.split(QRegExp("\\s+"));
		if (version_.size() >= 1)
			version_=sl.at(1);
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << name_ << " " << version_;
}
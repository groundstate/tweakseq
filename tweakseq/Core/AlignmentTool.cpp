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

#include <QRegExp>

#include "AlignmentTool.h"
#include "FileProperty.h"

//
// Public
//

AlignmentTool::AlignmentTool()
{
	init();
}

AlignmentTool::~AlignmentTool()
{
}

void AlignmentTool::setExecutable(QString e)
{
	executable_->setFileName(e);
}

QString AlignmentTool::executable()
{
	return executable_->fileName();
}

void AlignmentTool::makeCommand(QString &, QString &, QString &, QStringList &)
{
}

void AlignmentTool::writeSettings(QDomDocument &,QDomElement &)
{
}

void AlignmentTool::readSettings(QDomDocument &)
{
}

//
//
//
void AlignmentTool::parseCustomCommand(QString &exec, QStringList &arglist){
	arglist = customCommand_.split(QRegExp("\\s+"));
	if (arglist.size() > 0){
		exec = arglist.at(0);
		arglist.pop_front();
	}
	// Get an empty item in the list at the end
	if (arglist.at(arglist.size()-1).isEmpty())
		arglist.pop_back();
}


//
//	Private
//	

void AlignmentTool::init()
{
	preferred_=false;
	usesStdOut_=false;
	executable_ = registerFileProperty(NULL,"executable",0);
}
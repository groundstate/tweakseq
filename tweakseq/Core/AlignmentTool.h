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


#ifndef __ALIGNMENT_TOOL_H_
#define __ALIGNMENT_TOOL_H_

#include <QString>

#include "Propertied.h"

class QDomDocument;
class QDomElement;


class AlignmentTool:public Propertied
{
	public:
		
		AlignmentTool();
		virtual ~AlignmentTool();
		
		QString name(){return name_;}
		QString version(){return version_;}
		QString executable(){return executable_;}
		
		void setExecutable(QString e){executable_=e;}
		
		void setPreferred(bool pref){preferred_=pref;}
		bool preferred(){return preferred_;}
		
		bool usesStdOut(){return usesStdOut_;} // the alignment is written to stdout
		
		virtual void makeCommand(QString &, QString &, QString &, QStringList &);
		
		virtual void writeSettings(QDomDocument &,QDomElement &);
		virtual void readSettings(QDomDocument &);
	
	protected:
	
		QString name_;
		QString version_;
		QString executable_;
		bool preferred_;
		bool usesStdOut_;
		
	private:
	
		void init();
		
};

#endif

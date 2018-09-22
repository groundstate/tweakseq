//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2018  Merridee A. Wouters, Michael J. Wouters
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

#ifndef __FILE_PROPERTY_INPUT_H_
#define __FILE_PROPERTY_INPUT_H_

#include <string>

#include "PropertyInput.h"

class QLineEdit;
class QPushButton;

class FileProperty;

class FilePropertyInput:public PropertyInput
{
	Q_OBJECT
	
	public:
	 
	 	FilePropertyInput(QWidget *,FileProperty *,QString,int,const char *,const char *);
		virtual ~FilePropertyInput();
		
		void addFilter(const char *);
		const char * value();
		void setValue(const char *);
		enum InputKind {SaveFile,OpenFile,Directory};
		
	public slots:
	
		virtual void set();
		virtual void restore();
	
	private slots:
	
		void browse();
			
	private:
	
		QLineEdit *le_;
		FileProperty *filevar_;
		
		int inputKind_;
		QString filter_;
		QString dirName_;
};

#endif

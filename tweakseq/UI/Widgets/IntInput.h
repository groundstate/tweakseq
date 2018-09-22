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

#ifndef __INT_INPUT_H_
#define __INT_INPUT_H_

#include <QLineEdit>
#include <QIntValidator>

class QWidget;
class QEvent;

class IntInput:public QLineEdit
{

	Q_OBJECT
	
	public:
	
		IntInput(QWidget *parent=0,QString name = QString());
		IntInput(QWidget *parent,int value,int minVal,int maxVal,QString name = QString());
		~IntInput();
	
		int  value();	
		void setMin(int v);
		int  min();
		void setMax(int v);
		int  max();
	
	public slots:
	
		void setValue(int v);
		
	signals:
		
		void valueChanged(int);
		
	protected:
		
		virtual void leaveEvent(QEvent *);
		
	protected slots:
		
		void handleValueChange();
				
	private:
	
		int v;
		QIntValidator *validator;
		
};

// FIXME no validation
inline int IntInput::value()
	{return text().toInt();}	

inline int IntInput::min()
	{return validator->bottom();}

inline int IntInput::max()
	{return validator->top();}

#endif

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



#include <limits.h> //FIXME Linux specific ???

#include <QWidget>
#include <QString>
#include <QEvent>

#include "IntInput.h"

// Public members

IntInput::IntInput(QWidget *parent,QString name):
	QLineEdit(parent)
{
	setObjectName(name);
	v=0;
	setValue(v);
	setMaxLength(12);
	validator= new QIntValidator(INT_MIN,INT_MAX,this);
	connect(this,SIGNAL(returnPressed()),this,SLOT(handleValueChange()));
}

IntInput::IntInput(QWidget *parent,int value,
	int minVal,int maxVal,QString name):QLineEdit(parent)
{
	setObjectName(name);
	int vmin;
	QString t;
	v=value;
	if (maxVal<minVal){
		vmin=maxVal;
		minVal=maxVal;
		maxVal=vmin;
	}
	if (v < minVal) v = minVal;
	if (v > maxVal) v = maxVal;
	setText(t.setNum(v));
	setMaxLength(12);
	validator= new QIntValidator(minVal,maxVal,this);
	connect(this,SIGNAL(returnPressed()),this,SLOT(handleValueChange()));
}

IntInput::~IntInput()
{
	// Nothing to destroy
}
	
void IntInput::setValue(int val)
{
	QString t;
	// Checking whether v has changed prevents an infinite loop
	// that can occur if this widget is bound up with another widget
	// like a slider
	if (v != val){
		if (val >= validator->bottom() && val <= validator->top()){
			setText(t.setNum(val));
			v=val;
			emit valueChanged(v);
		}
	}
}

void IntInput::setMin(int val)
{
	// If new minimum is greater than the current value
	validator->setBottom(val);
	if (v<val)
		setValue(val);
	
}

void IntInput::setMax(int val)
{
	validator->setTop(val);
	if (v>val)
		setValue(val);
}
// Protected slots

void IntInput::leaveEvent(QEvent *e)
{
	// Dunno if this is standard behaviour but I think if you type
	// in something and then input focus is lost, the current input
	// ought to be validated. 
	 
	QLineEdit::leaveEvent(e);
	handleValueChange();
}

// Private slots

void IntInput::handleValueChange()
{
	bool ok;
	int vtmp = text().toInt(&ok);
	if (ok && vtmp != v){
		v=vtmp;
		emit valueChanged(v);
	}
}

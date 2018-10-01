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


#include <cfloat> 

#include <QWidget>
#include <QString>
#include <QEvent>
#include <QDoubleValidator>

#include "DoubleInput.h"

// Public members

DoubleInput::DoubleInput(QWidget *parent,QString name):
	QLineEdit(parent)
{
	setObjectName(name);
	v=0.0;
	setValue(v);
	setMaxLength(12);
	validator= new QDoubleValidator(-DBL_MAX,DBL_MAX,8,this);
	connect(this,SIGNAL(returnPressed()),this,SLOT(handleValueChange()));
}

DoubleInput::DoubleInput(QWidget *parent,double value,
	double minVal,double maxVal,QString name):QLineEdit(parent)
{
	double vmin;
	QString t;
	v=value;
	setObjectName(name);
	if (maxVal<minVal){
		vmin=maxVal;
		minVal=maxVal;
		maxVal=vmin;
	}
	if (v < minVal) v = minVal;
	if (v > maxVal) v = maxVal;
	setText(t.setNum(v));
	setMaxLength(12);
	validator= new QDoubleValidator(minVal,maxVal,8,this);
	connect(this,SIGNAL(returnPressed()),this,SLOT(handleValueChange()));
}

DoubleInput::~DoubleInput()
{
	// Nothing to destroy
}


double DoubleInput::value()
{return text().toFloat();}	

double DoubleInput::min()
{return (double) validator->bottom();}

double DoubleInput::max()
{return (double) validator->top();}
		
void DoubleInput::setValue(double val)
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

void DoubleInput::setMin(double val)
{
	// If new minimum is greater than the current value
	validator->setBottom(val);
	if (v<val)
		setValue(val);
	
}

void DoubleInput::setMax(double val)
{
	validator->setTop(val);
	if (v>val)
		setValue(val);
}
// Protected slots

void DoubleInput::leaveEvent(QEvent *e)
{
	// Dunno if this is standard behaviour but I think if you type
	// in something and then input focus is lost, the current input
	// ought to be validated. 
	 
	QLineEdit::leaveEvent(e);
	handleValueChange();
}

// Private slots

void DoubleInput::handleValueChange()
{
	bool ok;
	double vtmp = text().toDouble(&ok);
	if (ok && vtmp != v){
		v=vtmp;
		emit valueChanged(v);
	}
}

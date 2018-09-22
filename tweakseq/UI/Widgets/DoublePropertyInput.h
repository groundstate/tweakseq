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

#ifndef __DOUBLE_PROPERTY_INPUT_H_
#define __DOUBLE_PROPERTY_INPUT_H_

#include "PropertyInput.h"

class QLineEdit;
class QPushButton;

class DoubleInput;
class DoubleProperty;
class SuperSlider;

class DoublePropertyInput:public PropertyInput
{
	Q_OBJECT
	
	public:
	 
	 	DoublePropertyInput(QWidget *,DoubleProperty *,QString,bool showSlider=false);
		virtual ~DoublePropertyInput();
		
		double value();
		void setValue(double);
		
	public slots:
	
		virtual void set();
		virtual void restore();
	
	private slots:
	
	private:
	
		DoubleInput *fi_;
		SuperSlider *ss_;
		DoubleProperty *property_;
		
};

#endif

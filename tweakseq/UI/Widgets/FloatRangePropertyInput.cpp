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

//
// public:
//

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>

#include "FloatProperty.h"
#include "FloatPropertyInput.h"
#include "FloatRangePropertyInput.h"

FloatRangePropertyInput::FloatRangePropertyInput(QWidget *parent,
	FloatProperty *min,QString minlabel,
	FloatProperty *max,QString maxlabel):
	PropertyInput(parent,minlabel)
{
	min->saveValue();
	max->saveValue();

	minValue_= min;
	maxValue_=max;

	
	QHBoxLayout *hb = new QHBoxLayout(this);
	
	QLabel *l= new QLabel(minlabel,this);
	hb->addWidget(l,1);
	
	minInput_=new FloatPropertyInput(this,min,minlabel);
	hb->addWidget(minInput_,1);
	
	hb->insertSpacing(3,30);
	
	l = new QLabel(maxlabel,this);
	hb->addWidget(l,1);
	
	maxInput_=new FloatPropertyInput(this,max,maxlabel);
	hb->addWidget(l,1);

}

FloatRangePropertyInput::~FloatRangePropertyInput()
{
}


float FloatRangePropertyInput::minValue()
{
	return minInput_->value();
}

float FloatRangePropertyInput::maxValue()
{
	return maxInput_->value();
}

void FloatRangePropertyInput::setMinValue(float f)
{
	minInput_->setValue( f);
}

void FloatRangePropertyInput::setMaxValue(float f)
{
	maxInput_->setValue( f);
}

//		
// public slots:
//

void FloatRangePropertyInput::set()
{
	minValue_->setValue(minInput_->value());
	maxValue_->setValue(maxInput_->value());
}

void FloatRangePropertyInput::restore()
{
	minValue_->restoreSavedValue();
	maxValue_->restoreSavedValue();
}

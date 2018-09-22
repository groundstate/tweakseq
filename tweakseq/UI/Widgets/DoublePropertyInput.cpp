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
#include <QLineEdit>
#include <QLabel>
#include <QLayout>

#include "DoubleInput.h"
#include "DoubleProperty.h"
#include "DoublePropertyInput.h"
#include "SuperSlider.h"

DoublePropertyInput::DoublePropertyInput(QWidget *parent,DoubleProperty *property,
	QString label,bool showSlider):PropertyInput(parent,label)
{
	property->saveValue();
	property_=property;

	QHBoxLayout *hb = new QHBoxLayout(this);
	hb->setContentsMargins(0,0,0,0);
	fi_ = new DoubleInput(this,property_->value(),property_->min(),property_->max(),label);
	hb->addWidget(fi_,0);
	if (showSlider)
	{
		ss_=new SuperSlider(this,property_->value(),property_->min(),property_->max());
		hb->addWidget(ss_,1);
		connect(ss_,SIGNAL(fValueChanged(double)),this,SLOT(set(double)));
	}
}

DoublePropertyInput::~DoublePropertyInput()
{
}


double DoublePropertyInput::value()
{
	return fi_->value();
}

void DoublePropertyInput::setValue(double f)
{
	fi_->setValue(f);
}


//		
// public slots:
//

void DoublePropertyInput::set()
{
	property_->setValue(fi_->value());
}

void DoublePropertyInput::restore()
{
	property_->restoreSavedValue();
}

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

#include "IntInput.h"
#include "IntProperty.h"
#include "IntPropertyInput.h"
#include "SuperSlider.h"

IntPropertyInput::IntPropertyInput(QWidget *parent,IntProperty *property,
	QString label,bool showSlider):PropertyInput(parent,label)
{
	property->saveValue();
	property_=property;

	QHBoxLayout *hb = new QHBoxLayout(this);
	hb->setContentsMargins(0,0,0,0);
	ii_ = new IntInput(this,property_->value(),property_->min(),property_->max(),label);
	hb->addWidget(ii_,0);
	if (showSlider)
	{
		ss_=new SuperSlider(this,property_->value(),property_->min(),property_->max());
		hb->addWidget(ss_,1);
		connect(ss_,SIGNAL(fValueChanged(float)),this,SLOT(set(float)));
	}
}

IntPropertyInput::~IntPropertyInput()
{
}


int IntPropertyInput::value()
{
	return ii_->value();
}

void IntPropertyInput::setValue(int i)
{
	ii_->setValue(i);
}


//		
// public slots:
//

void IntPropertyInput::set()
{
	property_->setValue(ii_->value());
}

void IntPropertyInput::restore()
{
	property_->restoreSavedValue();
}

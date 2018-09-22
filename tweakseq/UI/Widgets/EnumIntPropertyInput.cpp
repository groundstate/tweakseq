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

#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLayout>

#include "EnumIntPropertyInput.h"
#include "IntProperty.h"
//
// Public members
//

EnumIntPropertyInput::EnumIntPropertyInput(QWidget *parent,IntProperty *property,
	QString label):PropertyInput(parent,label)
{
	property->saveValue();
	property_=property;

	style_=ComboBox;

	QHBoxLayout *hb = new QHBoxLayout(this);
	hb->setContentsMargins(0,0,0,0);

	
	cb_= new QComboBox(this);
	cb_->setObjectName(label);
	hb->addWidget(cb_);
	
	for (int i=0;i<property_->numEnumerations();i++)
		cb_->addItem(property_->enumDescription(i));
	
	for (int i=0;i<property_->numEnumerations();i++)
	{
		if (property_->value() == (property_->enumValue(i)))
		{
			cb_->setCurrentIndex(i);
			break;
		}
	}
	connect(cb_,SIGNAL(activated(int)),this,SIGNAL(changed()));
	connect(cb_,SIGNAL(activated(int)),this,SLOT(selectionChanged(int)));
	
}

EnumIntPropertyInput::EnumIntPropertyInput(QWidget *parent,IntProperty *property,
	QString label,QStringList &icons,int w,int h):PropertyInput(parent,label)
{
	property->saveValue();
	property_=property;

	style_=IconArray;

	gl_ = new QGridLayout(this);
	gl_->setContentsMargins(0,0,0,0);
	gl_->setHorizontalSpacing(0);
	gl_->setVerticalSpacing(0);
	bg_ = new QButtonGroup(this);
	bg_->setExclusive(true);


	for (int i=0;i<property_->numEnumerations();i++)
	{
		int row = i / w;
		int col = i % w;
		QPushButton *pb = new QPushButton(this);
		pb->setIcon(QIcon(icons.at(i)));
		bg_->addButton(pb,i);
		pb->setCheckable(true);
		pb->setObjectName(QString::number(i)); // tag with enumeration value
		if (property_->enumValue(i) == property_->value())
			pb->setChecked(true);
		gl_->addWidget(pb,row,col);
		connect(pb,SIGNAL(clicked()),this,SIGNAL(changed()));
		connect(pb,SIGNAL(clicked()),this,SLOT(select()));
	}
	
}


EnumIntPropertyInput::~EnumIntPropertyInput()
{
}
	

int EnumIntPropertyInput::value()
{
	if (style_ ==IconArray){
		int i = bg_->checkedId();
		if (i>=0)
			return property_->enumValue(i);
		// FIXME problem here ?
	}
	
	return property_->enumValue(cb_->currentIndex());
}

void EnumIntPropertyInput::setValue(int v)
{
	// v is an enumeration value

	for (int i=0;i<property_->numEnumerations();i++)
	{
		if (v== property_->enumValue(i))
		{
			if (style_ ==IconArray)
			{
				QAbstractButton *btn = bg_->button(i);
				if (btn)
					btn->setChecked(true);
			}
			else
				cb_->setCurrentIndex(i);
			break;
		}
	}
}

QSize EnumIntPropertyInput::sizeHint()
{
	if (style_ == IconArray)
	{
		return gl_->sizeHint();
	}
	else
	{
		return cb_->sizeHint();
	}
}

QSizePolicy EnumIntPropertyInput::sizePolicy()
{
	if (style_ == IconArray)
	{
		return PropertyInput::sizePolicy();
	}
	else
	{
		return cb_->sizePolicy();
	}
}
//
// Public slots
//

void EnumIntPropertyInput::set()
{
	if (style_ == IconArray)
	{
		property_->setValue(property_->enumValue(bg_->checkedId()));
	}
	else
	{
		property_->setValue(property_->enumValue(cb_->currentIndex()));
	}
	emit changed(property_->value());
}

void EnumIntPropertyInput::restore()
{
	property_->restoreSavedValue();
}

void EnumIntPropertyInput::selectionChanged(int idx)
{
	
		emit changed(property_->enumValue(idx));
	
}

void EnumIntPropertyInput::select()
{
	QObject *obj = sender();
	emit changed(property_->enumValue(bg_->checkedId()));
}

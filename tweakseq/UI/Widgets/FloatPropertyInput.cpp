
// endfx - a video editor/compositor
// Copyright (C) 2007  Michael Wouters

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Modification history

//
// public:
//

#include <QtDebug>
#include "DebuggingInfo.h"

#include <QLineEdit>
#include <QLabel>
#include <QLayout>

#include "FloatInput.h"
#include "FloatProperty.h"
#include "FloatPropertyInput.h"
#include "SuperSlider.h"

FloatPropertyInput::FloatPropertyInput(QWidget *parent,FloatProperty *property,
	QString label,bool showSlider):PropertyInput(parent,label)
{
	property->saveValue();
	property_=property;

	QHBoxLayout *hb = new QHBoxLayout(this);
	hb->setContentsMargins(0,0,0,0);
	fi_ = new FloatInput(this,property_->value(),property_->min(),property_->max(),label);
	hb->addWidget(fi_,0);
	if (showSlider)
	{
		ss_=new SuperSlider(this,property_->value(),property_->min(),property_->max());
		hb->addWidget(ss_,1);
		connect(ss_,SIGNAL(fValueChanged(float)),this,SLOT(set(float)));
	}
}

FloatPropertyInput::~FloatPropertyInput()
{
}


float FloatPropertyInput::value()
{
	return fi_->value();
}

void FloatPropertyInput::setValue(float f)
{
	fi_->setValue(f);
}


//		
// public slots:
//

void FloatPropertyInput::set()
{
	property_->setValue(fi_->value());
}

void FloatPropertyInput::restore()
{
	property_->restoreSavedValue();
}

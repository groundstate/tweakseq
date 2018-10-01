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


#include <QDomDocument>

#include "DoubleProperty.h"
#include "XMLHelper.h"

DoubleProperty::DoubleProperty( double *theDouble,
	Propertied *o,QString propertyName,double fval,double min,double max,int s):
	Property(o,propertyName,Property::DoubleV,s)
{
	init();
	
	min_=min;
	max_=max;

	if (theDouble)
		theDouble_=theDouble;
	else{
		theDouble_= new double;
		*theDouble_=fval;
		setLocalStorage(true);
	}

}

DoubleProperty::DoubleProperty(DoubleProperty *prop):Property()
{
	*this=*prop;
	if (usesLocalStorage())
	theDouble_= new double;
}

DoubleProperty::DoubleProperty(double *theDouble,Propertied *o,
	QDomElement &elem,double min,double max,int s):
	Property(o,"",Property::DoubleV,s)
{
	init();
	
	min_=min;
	max_=max;

	if (theDouble)
		theDouble_=theDouble;
	else{
		theDouble_= new double;
		setLocalStorage(true);
	}
		
	QString sbuf;
	if (XMLHelper::stringAttribute(elem,"name",&sbuf))
		setName(sbuf);
	*theDouble_= elem.text().toDouble();
}


DoubleProperty::DoubleProperty():Property()
{
	init();
}
		
DoubleProperty::~DoubleProperty()
{
	if (usesLocalStorage()) delete theDouble_;
}

void DoubleProperty::saveXML(QDomDocument &doc,QDomElement &elem)
{
	QStringList attr;
	attr << "type" << "name";
	QStringList vals;
	vals << "double" << name();
	XMLHelper::addElement(doc,elem,"property",
		QString::number(*theDouble_),attr,vals);
}

void DoubleProperty::saveValue()
{
	saved_=*theDouble_;
	last_=*theDouble_;
	setChanged(false);
}

void DoubleProperty::restoreSavedValue()
{
	if (*theDouble_ == saved_) return;
	(*theDouble_)=saved_;
}

void  DoubleProperty::setRange(double minVal,double maxVal)
{
	min_=minVal;
	max_=maxVal;
}

double DoubleProperty::min()
{
	return min_;
}

double DoubleProperty::max()
{
	return max_;
}

void DoubleProperty::setValue(double val)
{
	if (val==*theDouble_) return; // didn't change so no need to shout it out
	*theDouble_=val;
	notifyDependers();
}

void DoubleProperty::setDefaultValue(double val)
{
	defaultValue_=val;
}

double DoubleProperty::defaultValue()
{
	return defaultValue_;
}

bool DoubleProperty::isDefaultValue()
{
	return (*theDouble_ == defaultValue_);
}
	
void DoubleProperty::setToDefaultValue()
{
	*theDouble_ = defaultValue_;
}
//
//
//
void DoubleProperty::init()
{
	theDouble_=NULL;
	defaultValue_=0.0;
}

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

#include <QDomDocument>

#include "FloatProperty.h"
#include "XMLHelper.h"

FloatProperty::FloatProperty( float *theFloat,
	Propertied *o,QString propertyName,float fval,float min,float max,int s):
	Property(o,propertyName,Property::FloatV,s)
{
	init();
	
	min_=min;
	max_=max;

	if (theFloat)
		theFloat_=theFloat;
	else{
		theFloat_= new float;
		*theFloat_=fval;
		setLocalStorage(true);
	}

}

FloatProperty::FloatProperty(FloatProperty *prop):Property()
{
	*this=*prop;
	if (usesLocalStorage())
	theFloat_= new float;
}

FloatProperty::FloatProperty(float *theFloat,Propertied *o,
	QDomElement &elem,float min,float max,int s):
	Property(o,"",Property::FloatV,s)
{
	init();
	
	min_=min;
	max_=max;

	if (theFloat)
		theFloat_=theFloat;
	else{
		theFloat_= new float;
		setLocalStorage(true);
	}
		
	QString sbuf;
	if (XMLHelper::stringAttribute(elem,"name",&sbuf))
		setName(sbuf);
	*theFloat_= elem.text().toFloat();
}


FloatProperty::FloatProperty():Property()
{
	init();
}
		
FloatProperty::~FloatProperty()
{
	if (usesLocalStorage()) delete theFloat_;
}

void FloatProperty::saveXML(QDomDocument &doc,QDomElement &elem)
{
	QStringList attr;
	attr << "type" << "name";
	QStringList vals;
	vals << "float" << name();
	XMLHelper::addElement(doc,elem,"property",
		QString::number(*theFloat_),attr,vals);
}

void FloatProperty::saveValue()
{
	saved_=*theFloat_;
	last_=*theFloat_;
	setChanged(false);
}

void FloatProperty::restoreSavedValue()
{
	if (*theFloat_ == saved_) return;
	(*theFloat_)=saved_;
}

void  FloatProperty::setRange(float minVal,float maxVal)
{
	min_=minVal;
	max_=maxVal;
}

float FloatProperty::min()
{
	return min_;
}

float FloatProperty::max()
{
	return max_;
}

void FloatProperty::setValue(float val)
{
	if (val==*theFloat_) return; // didn't change so no need to shout it out
	*theFloat_=val;
	notifyDependers();
}

void FloatProperty::setDefaultValue(float val)
{
	defaultValue_=val;
}

float FloatProperty::defaultValue()
{
	return *theFloat_;
}

bool FloatProperty::isDefaultValue()
{
	return (defaultValue_=*theFloat_);
}

		
//
//
//
void FloatProperty::init()
{
	theFloat_=NULL;
	defaultValue_=0.0;
}

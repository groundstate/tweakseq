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

#include "IntProperty.h"
#include "XMLHelper.h"

IntProperty::IntProperty(int *theInt,
	Propertied *o,QString propertyName,int val,int min,int max,int s):
	Property(o,propertyName,Property::IntV,s)
{
	init();
	if (theInt)
		theInt_=theInt;
	else
	{
		theInt_= new int;
		*theInt_=val;
		setLocalStorage(true);
	}

	min_=min;
	max_=max;
}

IntProperty::IntProperty(IntProperty *prop):Property()
{
	*this=*prop;
	if (usesLocalStorage())
	{
		theInt_= new int;
		*theInt_= *(prop->theInt_);
	}
}

IntProperty::IntProperty(int *theInt,Propertied *o,
	QDomElement &elem,int min,int max,int s):
	Property(o,"",Property::IntV,s)
{
	init();
	
	min_=min;
	max_=max;

	if (theInt)
		theInt_=theInt;
	else{
		theInt_= new int;
		setLocalStorage(true);
	}
		
	QString sbuf;
	if (XMLHelper::stringAttribute(elem,"name",&sbuf))
		setName(sbuf);
	*theInt_= elem.text().toInt();
}

IntProperty::~IntProperty()
{
	if (usesLocalStorage() && theInt_) delete theInt_;
}

void IntProperty::saveXML(QDomDocument &doc,QDomElement &elem)
{
	QStringList attr;
	attr << "type" << "name";
	QStringList vals;
	vals << "int" << name();
	if (!enumerated_)
		XMLHelper::addElement(doc,elem,"property",QString::number(*theInt_),attr,vals);
	else
		XMLHelper::addElement(doc,elem,"property",descriptionFromEnum(*theInt_),attr,vals);
}

void IntProperty::saveValue()
{
	saved_=*theInt_;
	last_=*theInt_;
	setChanged(false);
}

void IntProperty::restoreSavedValue()
{
	if (*theInt_ == saved_) return;
	(*theInt_)=saved_;
}

void IntProperty::setValue(int val)
{
	if (val==*theInt_) return; // didn't change so no need to shout it out
	*theInt_=val;
	notifyDependers(); // this sets the value of changed() 
}

void  IntProperty::setRange(int minVal,int maxVal)
{
	min_=minVal;
	max_=maxVal;
}

int IntProperty::min()
{
	return min_;
}

int IntProperty::max()
{
	return max_;
}

void IntProperty::setEnumerations(QList<int> &values,QList<QString> &descriptions)
{
	enumValues_=values;
	enumDescriptions_=descriptions;
}

int IntProperty::enumValue(int idx)
{
	if (idx <0 || idx >= enumValues_.size()) return -9999999;
	return enumValues_.at(idx);
}

QString IntProperty::enumDescription(int idx)
{
	if (idx <0 || idx >= enumDescriptions_.size()) return QString("unknown");
	return enumDescriptions_.at(idx);
}

QString IntProperty::descriptionFromEnum(int e)
{
	for (int i=0;i<enumValues_.size();i++)
		if ( enumValues_.at(i) == e)
			return enumDescriptions_.at(i);
	return "Unknown";
}

int IntProperty::enumFromDescription(QString d)
{
	for (int i=0;i<enumDescriptions_.size();i++)
		if ( enumDescriptions_.at(i) == d )
			return enumValues_.at(i);
	return -1;
}

//
//
//

void IntProperty::init()
{
	theInt_=NULL;
	enumerated_=false;
}

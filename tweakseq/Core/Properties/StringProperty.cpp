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

#include <QDomDocument>

#include "StringProperty.h"
#include "XMLHelper.h"

StringProperty::StringProperty(QString *theString,
	Propertied *o,QString propertyName,QString sval,int s):
	Property(o,propertyName,Property::StringV,s)
{
	init();
	
	if (theString)
		theString_=theString;
	else
	{
		theString_= new QString();
		*theString_=sval;
		setLocalStorage(true);
	}
}

StringProperty::StringProperty(StringProperty *prop):Property()
{
	*this=*prop;
	if (usesLocalStorage()){
		theString_= new QString();
		*theString_=*(prop->theString_);
	}
}

StringProperty::StringProperty(QString *theString,
	Propertied *o,QDomElement &elem,int s):
	Property(o,"",Property::StringV,s)
{
	init();
	
	if (theString)
		theString_=theString;
	else{
		theString_= new QString();
		setLocalStorage(true);
	}

	QString sbuf;
	if (XMLHelper::stringAttribute(elem,"name",&sbuf))
		setName(sbuf);
	*theString_= elem.text();

}

StringProperty::~StringProperty()
{
	if (usesLocalStorage() && theString_) delete theString_;
}

void StringProperty::saveXML(QDomDocument &doc,QDomElement &elem)
{
	QStringList attr;
	attr << "type" << "name";
	QStringList vals;
	vals << "string" << name();
	XMLHelper::addElement(doc,elem,"property",
		*theString_,attr,vals);
}

void StringProperty::saveValue()
{
	saved_=*theString_;
	last_=*theString_;
	setChanged(false);
}

void StringProperty::restoreSavedValue()
{
	if (*theString_ == saved_) return;
	(*theString_)=saved_;
	notifyDependers();
}

void StringProperty::setValue(QString val)
{
	if (val==*theString_) return; // didn't change so no need to shout it out
	*theString_=val;
	notifyDependers();
}

void StringProperty::setDefaultValue(QString val)
{
	defaultValue_= val;
}

QString StringProperty::defaultValue()
{
	return defaultValue_;
}

bool StringProperty::isDefaultValue()
{
	return (defaultValue_==*theString_);
}
		
void StringProperty::init()
{
	theString_=NULL;
	defaultValue_="";
}

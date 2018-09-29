//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017  Merridee A. Wouters, Michael J. Wouters
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

#include "BoolProperty.h"
#include "Propertied.h"
#include "XMLHelper.h"

BoolProperty::BoolProperty(bool *b,Propertied *o,QString n,bool val,int s):
	Property(o,n,Property::BoolV,s)
{
	init();
	if (b)
		theBool_=b;
	else{
		theBool_= new bool;
		*theBool_=val;
		setLocalStorage(true);
	
	}
	
}


BoolProperty::BoolProperty(BoolProperty *aBoolProperty):Property(aBoolProperty)
{
	init();
	*this=*aBoolProperty;
	if (usesLocalStorage()){
		theBool_= new bool;
		*theBool_=*(aBoolProperty->theBool_);
	}
}
		
BoolProperty::BoolProperty(bool *b,Propertied *o,QDomElement &elem,int s):
	Property(o,"",Property::BoolV,s)
{
	init();
	if (b)
		theBool_=b;
	else{
		theBool_= new bool;
		setLocalStorage(true);
	}
		
	QString sbuf;
	if (XMLHelper::stringAttribute(elem,"name",&sbuf))
		setName(sbuf);
	*theBool_= XMLHelper::stringToBool(elem.text());
}

BoolProperty::BoolProperty():Property()
{
	init();
}

BoolProperty::~BoolProperty()
{
	if (usesLocalStorage() && (theBool_!=NULL)) delete theBool_;
}

void BoolProperty::saveXML(QDomDocument &doc,QDomElement &elem)
{
	QStringList attr;
	attr << "type" << "name";
	QStringList vals;
	vals << "bool" << name();
	XMLHelper::addElement(doc,elem,"property",
		XMLHelper::boolToString(*theBool_),attr,vals);
}


void BoolProperty::saveCurrentValue()
{
	saved_=*theBool_;
	last_=theBool_;
	setChanged(false);
}

void BoolProperty::restoreSavedValue()
{
	if (*theBool_ == saved_) return;
	*theBool_=saved_;
	notifyDependers();
}
		
void BoolProperty::setValue(bool val)
{
	if (val==*theBool_) return; // didn't change so no need to shout it out
	*theBool_=val;
	notifyDependers();
}

void BoolProperty::setDefaultValue(bool val)
{
	defaultValue_=val;
}

bool BoolProperty::defaultValue()
{
	return defaultValue_;
}

bool BoolProperty::isDefaultValue()
{
	return (*theBool_ == defaultValue_);
}

		
void BoolProperty::applyValue(bool val)
{
	if (val==*theBool_ || val==last_) return;
	last_=*theBool_; // save the current Var so that we can test  for a change 
	*theBool_=val;
	notifyDependers();
}
	
void BoolProperty::init()
{
	theBool_=NULL;
	defaultValue_=false;
}

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

#include "Property.h"

Property::Property(Propertied *n,QString propName,int t,int s)
{
	init();
	owner_=n;
	type_=t;
	signals_=s;
	name_=propName;
}

Property::Property(Property *p)
{
	*this=*p;
}

Property::Property()
{
	init();
}


Property::~Property()
{
}

void Property::lock(bool lockIt)
{
	locked_=lockIt;
}


void Property::block(bool blockIt)
{
	blocked_=blockIt;
}

//void Property::save(ofstream &,SaveFormatter &)
//{
//}

void Property::postLoadTidy()
{
}

void Property::setToDefaultValue()
{
}

void Property::saveValue()
{
}

void Property::restoreSavedValue()
{
}

void Property::notifyDependers()
{
	// Propertys are dirty until
	if (isBlocked())
		changed_=true;
	else{
		changed_=false;
		//if (obj_) {obj_->notify(this,s);}
	}
}

void Property::init()
{
	locked_=false;
	blocked_=true;
	owner_=NULL;
	changed_=false;
	usesLocalStorage_=false;
}
	

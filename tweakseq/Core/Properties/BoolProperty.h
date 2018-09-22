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

#ifndef __BOOL_PROPERTY_H_
#define __BOOL_PROPERTY__H_

#include "Property.h"

class Propertied;

class BoolProperty:public Property
{
	public:
	
		BoolProperty(bool *,Propertied *,QString,bool val=true,int s=0);
		BoolProperty(BoolProperty *);
		BoolProperty(bool *,Propertied *,QDomElement &,int s=0);
		BoolProperty();
		
		virtual ~BoolProperty();
		
		virtual void saveXML(QDomDocument &,QDomElement &);
		
		virtual void saveCurrentValue();
		virtual void restoreSavedValue();
		
		void applyValue(bool);
		void setValue(bool);
		bool value(){return *theBool_;}
	
	private:
		
		void init();
		
		bool *theBool_;
		bool last_;
		bool saved_;
};

#endif

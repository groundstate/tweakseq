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

#ifndef __INT_PROPERTY__H_
#define __INT_PROPERTY__H_

#include <limits.h>

#include "Property.h"

class IntProperty:public Property
{
	public:
	
		IntProperty(int *,Propertied *,QString,int val=0,int minVal = INT_MIN ,int maxVal = INT_MAX,int s=0);
		IntProperty(IntProperty *);
		IntProperty(int *,Propertied *,QDomElement &,int minVal = INT_MIN ,int maxVal = INT_MAX,int s=0);

		virtual ~IntProperty();
		
		virtual void saveXML(QDomDocument &,QDomElement &);
	
		virtual void saveValue();
		virtual void restoreSavedValue();
		
		int value(){return *theInt_;}
		void setValue(int);

		void setRange(int,int);
		int min();
		int max();
		
		bool isEnumerated(){return enumerated_;}
		void setEnumerated(bool e){enumerated_=e;}
		void setEnumerations(QList<int> &,QList<QString> &);
		int enumValue(int);
		QString enumDescription(int);
		QString descriptionFromEnum(int);
		int enumFromDescription(QString);
		
		int numEnumerations(){return enumValues_.size();}

	private:
		
		void init();
		
		int min_,max_;
		
		int *theInt_;
		int last_;
		int saved_;

		bool enumerated_;
		QList<int> enumValues_;
		QList<QString> enumDescriptions_;
		
};

#endif

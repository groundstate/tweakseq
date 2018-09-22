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

#ifndef __FLOAT_PROPERTY__H_
#define __FLOAT_PROPERTY__H_

#include <cfloat>

#include "Property.h"

class FloatProperty:public Property
{
	public:
	
		FloatProperty(float *,Propertied *,QString,float fval=0,float minVal = FLT_MIN ,float maxVal = FLT_MAX,int s=0);
		FloatProperty(FloatProperty *);
		FloatProperty(float *,Propertied *,QDomElement &,float minVal = FLT_MIN ,float maxVal = FLT_MAX,int s=0);
		FloatProperty();

		virtual ~FloatProperty();
		
		virtual void saveXML(QDomDocument &,QDomElement &);
	
		virtual void saveValue();
		virtual void restoreSavedValue();
			
		void setRange(float,float);
		float min();
		float max();
		
		float value(){return *theFloat_;}
		void  setValue(float);

	private:
		
		void init();
		
		float min_,max_;
		
		float *theFloat_;
		float last_;
		float saved_;
};

#endif

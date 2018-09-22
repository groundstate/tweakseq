//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2018  Michael J. Wouters, Merridee A. Wouters
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

#ifndef __PROPERTIED_H_
#define __PROPERTIED_H_

#include <limits.h>
#include <cfloat>

#include <QList>
#include <QString>

class QWidget;

class BoolProperty;
class DoubleProperty;
class FileProperty;
class FloatProperty;
class IntProperty;
class StringProperty;
class Property;
class PropertiesDialog;

class Propertied
{
	public:

		Propertied();
		virtual ~Propertied();

		Property *getProperty(const char *);
		bool locked(){return locked_;}
		void lock(bool l){locked_=l;}

		virtual PropertiesDialog *propertiesDialog(QWidget *){return NULL;}
		virtual void propertiesDialogEvent(int ){;}

	protected:
		
		IntProperty*    registerIntProperty(int *,QString,int min=INT_MIN,int max=INT_MAX,int sigs=0);
		BoolProperty*   registerBoolProperty(bool *,QString,int sigs=0);
		DoubleProperty* registerDoubleProperty(double *,QString,double min=DBL_MIN,double max=DBL_MAX,int sigs=0);
		FileProperty*   registerFileProperty(QString ,QString,int sigs=0);
		FloatProperty*  registerFloatProperty(float *,QString,float min=FLT_MIN,float max=FLT_MAX,int sigs=0);
		StringProperty* registerStringProperty(QString *,QString,int sigs=0);

		bool changed(Property *);
		bool changed();
		void setChanged(bool);
		void setChanged(Property*,bool);

	private:

		void init();
		bool locked_;
		bool changed_;

		// properties
		QList<Property *> properties_;

};

#endif


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

#include <QtDebug>
#include "DebuggingInfo.h"

#include <QDomDocument>

#include "BoolProperty.h"
#include "DoubleProperty.h"
#include "FileProperty.h"
#include "FloatProperty.h"
#include "IntProperty.h"
#include "StringProperty.h"

#include "Propertied.h"

//
//
//

Propertied::Propertied()
{
	init();
}

Propertied::~Propertied()
{
	
}

Property *Propertied::getProperty(QString pname)
{
	for (int i=0;i<properties_.size();i++){
		if (properties_[i]->name() == pname)
			return properties_[i];
	}
	return NULL;
}

void Propertied::saveXML(QDomDocument &doc,QDomElement &pelem)
{
	for (int i=0;i<properties_.size();i++)
		properties_.at(i)->saveXML(doc,pelem);
}

void Propertied::readXML(QDomDocument &,QDomElement &pelem)
{
	QDomElement elem = pelem.firstChildElement();
	while (!elem.isNull()){
		QDomAttr nameAttr = elem.attributeNode("name");
		Property *prop = getProperty(nameAttr.value());
		if (prop){
			switch (prop->type())
			{
				case Property::BoolV:
				{
					BoolProperty *bp = static_cast<BoolProperty *>(prop);
					if (elem.text() == "yes")
						bp->setValue(true);
					else if (elem.text() == "no")
						bp->setValue(false);
					break;
				}
				case Property::DoubleV:
				{
					DoubleProperty *dp = static_cast<DoubleProperty *>(prop);
					dp->setValue(elem.text().toDouble());
					break;
				}
				case Property::FileV:
				{
					FileProperty *fp = static_cast<FileProperty *>(prop);
					fp->setFileName(elem.text());
					break;
				}
				case Property::IntV:
				{
					IntProperty *ip = static_cast<IntProperty *>(prop);
					ip->setValue(elem.text().toInt());
				}
				case Property::StringV:
				{
					StringProperty *sp = static_cast<StringProperty *>(prop);
					sp->setValue(elem.text());
				}
			}
		}
		elem=elem.nextSiblingElement();
	}
			
}
		
DoubleProperty* Propertied::registerDoubleProperty(double *fval,QString n,double min,double max,int sigs)
{
	DoubleProperty *p = new DoubleProperty(fval,this,n,0,min,max,sigs);
	properties_.push_back(p);
	return p;
}


FileProperty* Propertied::registerFileProperty(QString *str,QString n,int sigs)
{
	FileProperty *p = new FileProperty(str,this,n,"",sigs);
	properties_.push_back(p);
	return p;
}

FileProperty* Propertied::registerFileProperty(QString str,QString n,int sigs)
{
	FileProperty *p = new FileProperty(str,this,n,sigs);
	properties_.push_back(p);
	return p;
}


IntProperty* Propertied::registerIntProperty(int *ival,QString n,int min,int max,int sigs)
{
	IntProperty *p = new IntProperty(ival,this,n,0,min,max,sigs);
	properties_.push_back(p);
	return p;
}

BoolProperty* Propertied::registerBoolProperty(bool *b,QString n,int sigs)
{
	BoolProperty *p = new BoolProperty(b,this,n,true,sigs);
	properties_.push_back(p);
	return p;
}

FloatProperty* Propertied::registerFloatProperty(float *fval,QString n,float min,float max,int sigs)
{
	FloatProperty *p = new FloatProperty(fval,this,n,0,min,max,sigs);
	properties_.push_back(p);
	return p;
}

bool Propertied::changed(Property *)
{
	return false;
}

bool Propertied::changed()
{
	return changed_;
}

void Propertied::setChanged(bool c)
{
	changed_=c;
}

void Propertied::setChanged(Property *,bool )
{
	
}

//
//
//

void Propertied::init()
{
	locked_=false;
	changed_=false;
}

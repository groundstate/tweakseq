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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QDomDocument>
#include <QFile>
#include <QFileInfo>

#include "FileProperty.h"
#include "XMLHelper.h"

FileProperty::FileProperty(QString *f,Propertied *o,QString n,QString sval,int s):
	Property(o,n,Property::FileV,s)
{
	init();
	if (f)
		fileName_=f;
	else{
		fileName_= new QString("");
		*fileName_=sval;
		setLocalStorage(true);
	}
}

FileProperty::FileProperty(QString &f,Propertied *o,QString n,int s):
	Property(o,n,Property::FileV,s)
{
	init();
	
	fileName_= new QString(f);
	setLocalStorage(true);
}

FileProperty::FileProperty(FileProperty *aFileProperty):Property(aFileProperty)
{
	*this=*aFileProperty;
	if (aFileProperty->usesLocalStorage()){
		fileName_ = new QString(aFileProperty->fileName());
		setLocalStorage(true);
		*fileName_=*(aFileProperty->fileName_);
	}
}

// FileProperty::FileProperty(QString *f,Propertied *o,QDomElement &elem,int s):
// 	Property(o,"",Property::FileV,s)
// {
// 	init();
// 	if (f)
// 		fileName_=f;
// 	else{
// 		fileName_= new QString("");
// 		setLocalStorage(true);
// 	}
// 	
// 	QString sbuf;
// 	if (XMLHelper::stringAttribute(elem,"name",&sbuf))
// 		setName(sbuf.toStdString());
// 	*fileName_= elem.text().toStdString();
// 
// }

FileProperty::FileProperty():Property()
{
	init();
}

		
FileProperty::~FileProperty()
{
	if (usesLocalStorage() && fileName_) delete fileName_;
}

void FileProperty::saveXML(QDomDocument &doc,QDomElement &elem)
{
	QStringList attr;
	attr << "type" << "name";
	QStringList vals;
	vals << "file" << name();
	XMLHelper::addElement(doc,elem,"property",
		*fileName_,attr,vals);
}

void FileProperty::saveValue()
{
	savedName_=*fileName_;
	lastName_=*fileName_;
	setChanged(false);
}

void FileProperty::restoreSavedValue()
{
	if (*fileName_== savedName_) return;
	*fileName_=savedName_;
	notifyDependers();
}


		
void FileProperty::setFileName(QString n)
{
	if (n==*fileName_ || n==lastName_) return;
	lastName_=*fileName_; // save the current Var so that we can test  for a change 
	*fileName_=n;
	notifyDependers();
	
}

QString FileProperty::fileName(bool fullPath)
{
	if (!fullPath){ // strip path
		QFileInfo fi(*fileName_);
		return fi.fileName();
	}
	return *fileName_;
}

void FileProperty::setDefaultValue(QString val)
{
	defaultValue_=val;
}

QString FileProperty::defaultValue()
{
	return defaultValue_;
}

bool FileProperty::isDefaultValue()
{
	return (*fileName_==defaultValue_);
}

		
bool FileProperty::exists()
{
	if (fileName_->isEmpty()) return false;
	QFile f(*fileName_);
	return f.exists();
}

bool FileProperty::defined()
{
	return !(fileName_->isEmpty());
}

QString FileProperty::path()
{
	return "";
}
		
void FileProperty::init()
{
	fileName_=NULL;
	defaultValue_="";
}

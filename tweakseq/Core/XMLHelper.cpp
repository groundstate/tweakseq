//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017  Michael J. Wouters, Merridee A. Wouters
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

#include <QtXml>

#include "XMLHelper.h"

QString XMLHelper::boolToString(bool b)
{
	if (b) return "yes";
	return "no";
}

bool XMLHelper::stringToBool(QString s)
{
	if (s=="yes") return true;
	if (s=="no") return false;
	// backwards compatibility
	bool ok=false;
	int ival =s.toInt(&ok);
	if (ok) return ival >= 1;
	return false;
}

QDomElement XMLHelper::addElement(QDomDocument &doc,QDomElement &rootElem,
	QString tag,QString txt)
{
	QDomElement elem = doc.createElement(tag);
	rootElem.appendChild(elem);
	QDomText te = doc.createTextNode(txt);
	elem.appendChild(te);
	return elem;
}


QDomElement XMLHelper::addElement(QDomDocument &doc,QDomElement &rootElem,
	QString tag,QString txt,QStringList attributes,QStringList values)
{
	QDomElement elem = doc.createElement(tag);
	int nattributes=attributes.count();
	if (values.count() < attributes.count())
	{
		nattributes=values.count();
		qWarning() <<  warning.header() << "XMLHelper::addTextElement() size mismatch !";
	}
	for (int i=0;i<nattributes;i++)
	{	
		elem.setAttribute(attributes[i],values[i]);
	}
	rootElem.appendChild(elem);
	QDomText te = doc.createTextNode(txt);
	elem.appendChild(te);
	return elem;
}

QDomElement XMLHelper::childElementByTagName(QDomElement &rootElem,QString tagName)
{
	QDomElement elem = rootElem.firstChildElement();
	while (!elem.isNull())
	{
		if (elem.tagName() == tagName)
			return elem;
		elem=elem.nextSiblingElement();
	}
	qWarning() <<  warning.header() << "XMLHelper::childElementByTagName() failed: " << tagName;
	return QDomElement();
}

bool XMLHelper::intList(QDomElement &elem,QList<int> &var)
{
	QString txt = elem.text();
	QTextStream ts(&txt);
	int val;
	while (!ts.atEnd())
	{
		ts >> val;
		var.push_back(val);
	}
	return true;
}

bool XMLHelper::stringAttribute(QDomElement &elem,QString name,QString *var)
{
	if (!elem.hasAttribute(name))
	{
		qWarning() <<  warning.header() << "XMLHelper::stringAttribute " << elem.tagName() << " missing " 
				<< name;
		return false;
	}
	*var= elem.attribute(name);
	return true;
}

bool	XMLHelper::intAttribute(QDomElement &elem,QString name,int *var)
{
	if (!elem.hasAttribute(name))
	{
		qWarning() <<  warning.header() << "XMLHelper::intAttribute " << elem.tagName() << "missing " 
				<< name;
		return false;
	}
	QString res = elem.attribute(name);
	*var = res.toInt();
	return true;
}

bool	XMLHelper::doubleAttribute(QDomElement &elem,QString name,double *var)
{
	if (!elem.hasAttribute(name))
	{
		qWarning() <<  warning.header() << "XMLHelper::doubleAttribute " << elem.tagName() << "missing " 
				<< name; 
		return false;
	}
	QString res = elem.attribute(name);
	*var = res.toDouble();	
	return true;
}

bool	XMLHelper::boolAttribute(QDomElement &elem,QString name,bool *var)
{
	if (!elem.hasAttribute(name))
	{
		qWarning() <<  warning.header() << "XMLHelper::boolAttribute " << elem.tagName() << "missing " 
				<< name; 
		return false;
	}
	QString res = elem.attribute(name);
	*var = stringToBool(res);	
	return true;
}

bool XMLHelper::stringProperty(QDomElement &rootElem,QString nameval,QString *var,QDomElement *theElem)
{
	// searches children of rootelem
	QDomElement elem = rootElem.firstChildElement();
	while (!elem.isNull())
	{
		QString v=elem.attribute("name");
		if (v==nameval)
		{
			qDebug() << trace.header() << "XMLHelper::stringProperty() got " <<  nameval << " " << elem.text();
			*var = elem.text();
			if(theElem != NULL) *theElem=elem;
			return true;
		}
		elem=elem.nextSiblingElement();
	}
	qWarning() <<  warning.header() << "XMLHelper::stringProperty() failed: " << nameval;
	return false;
}

bool XMLHelper::intProperty(QDomElement &rootElem,QString nameval,int *var)
{
	QDomElement elem = rootElem.firstChildElement();
	while (!elem.isNull())
	{
		QString v=elem.attribute("name");
		if (v==nameval)
		{
			qDebug() << trace.header() << "XMLHelper::intProperty() got " <<  nameval << " " << elem.text();
			*var = elem.text().toInt();
			return true;
		}
		elem=elem.nextSiblingElement();
	}
	qWarning() <<  warning.header() << "XMLHelper::intProperty() failed: " << nameval;
	return false;
}

bool XMLHelper::doubleProperty(QDomElement &rootElem,QString nameval,double *var)
{
	QDomElement elem = rootElem.firstChildElement();
	while (!elem.isNull())
	{
		QString v=elem.attribute("name");
		if (v==nameval)
		{
			qDebug() << trace.header() << "XMLHelper::doubleProperty() got " <<  nameval << " " << elem.text();
			*var = elem.text().toDouble();
			return true;
		}
		elem=elem.nextSiblingElement();
	}
	qWarning() <<  warning.header() << "XMLHelper::doubleProperty() failed: " << nameval;
	return false;
}

bool	XMLHelper::doubleListProperty(QDomElement &rootElem,QString nameval,std::vector<double> &var)
{
	QDomElement elem = rootElem.firstChildElement();
	while (!elem.isNull())
	{
		QString v=elem.attribute("name");
		if (v==nameval)
		{
			qDebug() << trace.header() << "XMLHelper::doubleListProperty() got " <<  nameval << " " << elem.text();
			QString txt = elem.text();
			QTextStream ts(&txt);
			double val;
			while (!ts.atEnd())
			{
				ts >> val;
				var.push_back(val);
			}
				qDebug() << trace.header() << "XMLHelper::doubleListProperty() got " <<  nameval << " " << elem.text() << " " << var.size() << " values";
			return true;
		}
		elem=elem.nextSiblingElement();
	}
	qWarning() <<  warning.header() << "XMLHelper::doubleProperty() failed: " << nameval;
	return false;
}

bool	XMLHelper::intListProperty(QDomElement &rootElem,QString nameval,std::vector<int> &var)
{
	QDomElement elem = rootElem.firstChildElement();
	while (!elem.isNull())
	{
		QString v=elem.attribute("name");
		if (v==nameval)
		{
			qDebug() << trace.header() << "XMLHelper::intListProperty() got " <<  nameval << " " << elem.text();
			QString txt = elem.text();
			QTextStream ts(&txt);
			int val;
			while (!ts.atEnd())
			{
				ts >> val;
				var.push_back(val);
			}
				qDebug() << trace.header() << "XMLHelper::intListProperty() got " <<  nameval << " " << elem.text() << " " << var.size() << " values";
			return true;
		}
		elem=elem.nextSiblingElement();
	}
	qWarning() <<  warning.header() << "XMLHelper::intListProperty() failed: " << nameval;
	return false;
}
		
bool XMLHelper::boolProperty(QDomElement &rootElem,QString nameval,bool *var)
{
	QDomElement elem = rootElem.firstChildElement();
	while (!elem.isNull())
	{
		QString v=elem.attribute("name");
		if (v==nameval)
		{
			qDebug() << trace.header() << "XMLHelper::boolProperty() got " <<  nameval << " " << elem.text();
			*var = stringToBool(elem.text());
			return true;
		}
		elem=elem.nextSiblingElement();
	}
	qWarning() <<  warning.header() << "XMLHelper::boolProperty() failed: " << nameval;
	
	return false;
}



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

#ifndef __XML_HELPER_H_
#define __XML_HELPER_H_

#include <vector>

class QDomElement;
class QDomDocument;
class QString;
class QStringList;

class XMLHelper
{
	public:

		static QString boolToString(bool);
		static bool stringToBool(QString);

		static QDomElement addElement(QDomDocument &,QDomElement &,
			QString ,QString );
		static QDomElement addElement(QDomDocument &,QDomElement &,
			QString ,QString ,QStringList ,QStringList );

		static QDomElement childElementByTagName(QDomElement &,QString);

		static bool intList(QDomElement &,QList<int> &);

		static bool stringAttribute(QDomElement &,QString,QString *);
		static bool	intAttribute(QDomElement &,QString,int *);
		static bool	doubleAttribute(QDomElement &,QString,double *);
		static bool	boolAttribute(QDomElement &,QString,bool *);

		static bool stringProperty(QDomElement &,QString,QString *,QDomElement *theElem=NULL);
		static bool	intProperty(QDomElement &,QString,int *);
		static bool	doubleProperty(QDomElement &,QString,double *);
		static bool	doubleListProperty(QDomElement &,QString,std::vector<double> &);
		static bool	intListProperty(QDomElement &,QString,std::vector<int> &);
		
		static bool boolProperty(QDomElement &,QString,bool *);
		
};

#endif


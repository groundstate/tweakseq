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

#ifndef __PROPERTY__H_
#define __PROPERTY__H_

#include <QList>
#include <QString>

class QDomDocument;
class QDomElement;

class Propertied;

class Property
{
	public:
	
		Property(Propertied *,QString,int,int s=0);
		Property(Property *);
		Property();
		Property(Propertied *,QDomElement &){;}

		virtual ~Property();
		
		virtual void saveXML(QDomDocument &,QDomElement &){;}

		QString name(){return name_;}
		void setName(QString n){name_=n;}
		
		virtual void lock(bool);
		bool isLocked(){return locked_;}
		
		virtual void block(bool);
		bool isBlocked(){return blocked_;}
		
		virtual void postLoadTidy();
		
		virtual void saveValue();
		virtual void restoreSavedValue();
		
		virtual void notifyDependers();
		
		bool changed(){return changed_;}
		void setChanged(bool c){changed_=c;}
		
		//Node *node(){return node_;}
		int type(){return type_;}
		void setSignals(int s){signals_=s;}
		
		enum Properties {DoubleV,FloatV,BoolV,IntV,FileV,StringV};

		enum Signals{
			Reload=0x01,
			Redraw=0x02,
			Recalculate=0x04
		};
	
	protected:
	
		void setType(int t){type_=t;}
		void setLocalStorage(bool l){usesLocalStorage_=l;}
		bool usesLocalStorage(){return usesLocalStorage_;}
			
	private:
		
		void init();
		
		bool locked_;
		bool blocked_;
		Propertied *owner_;
		bool changed_;
		
		QString name_;
		
		int type_;
		int signals_;
		
		bool usesLocalStorage_;
		
};

typedef QList<Property *> PropertyList;

#endif

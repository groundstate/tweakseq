//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017  Merridee A. Wouters, Michael J. Wouters
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


#ifndef __SEQUENCES_H_
#define __SEQUENCES_H_

#include <QObject>
#include <QList>
#include <QString>

class Sequence;
class SequenceGroup;

class Sequences:public QObject
{
	Q_OBJECT
	
	public:
		
		Sequences();
		~Sequences();
		
		QList<Sequence *> & sequences(){return sequences_;} 
		
		int index(Sequence *);
		int visibleIndex(Sequence *);
		
		int getIndex(QString label);
		QString getLabelAt(int);
		
		bool isEmpty();
		int size(){return sequences_.size();}
		int visibleSize();
		void clear();
		Sequence * visibleAt(int);
		int visibleToActual(int);
		
		bool isSubGroup(int,int);
		
		Sequence * add(QString,QString,QString,QString,bool );
		void  append(Sequence *);
		void  remove(QString);
		void  insert(QString,QString,int);
		void  insert(Sequence *,Sequence *,bool postInsert=true);
		void  replace(QString,QString,QString);
		void  move(int,int);
		void  replaceResidues(QString ,int pos);
		void  addInsertions(int,int,int,int);
		void  addInsertions(Sequence *,int,int);
		void  removeResidues(int,int,int,int);
		
	signals:
		
		void sequenceAdded(Sequence *);
		void cleared();
		void changed();
		
	private:
		
		QList<Sequence *> sequences_;
};

#endif
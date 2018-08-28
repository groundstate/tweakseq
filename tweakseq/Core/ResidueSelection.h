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


#ifndef __RESIDUE_SELECTION_H_
#define __RESIDUE_SELECTION_H_

#include <QObject>

class Sequence;
class SequenceGroup;

class ResidueGroup
{
	public:
		ResidueGroup(Sequence *s,int startResidueIndex,int stopResidueIndex)
		{
			sequence=s;
			start=startResidueIndex;
			stop=stopResidueIndex;
		}
		ResidueGroup(ResidueGroup *r)
		{
			sequence=r->sequence;
			start=r->start;
			stop=r->stop;
		}
		
		Sequence *sequence;
		int start,stop;
};

class ResidueSelection:public QObject
{
	Q_OBJECT
	
	public:
		
		ResidueSelection();
		~ResidueSelection();
	
		// There's only one contiguous residue selection at any time so don't need toggle
		void set(QList<ResidueGroup *> &);
		void clear();
		QString selectedResidues(int);
		
		bool isInsertionsOnly();
		bool isLocked();
		
		bool empty(){return sel_.size() == 0;}
		int  size(){return sel_.size();}
		ResidueGroup * itemAt(int);
		QList<ResidueGroup*> & residueGroups(){return sel_;}
		QList<SequenceGroup *> uniqueSequenceGroups();
		
	signals:
		
		void changed();
		
	private:
		
		QList<ResidueGroup *> sel_;
};

#endif
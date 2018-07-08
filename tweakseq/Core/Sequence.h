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


#ifndef __SEQUENCE_H_
#define __SEQUENCE_H_

#include <QList>
#include <QString>

#define EXCLUDE_CELL    0x0080 
#define HIGHLIGHT_CELL  0x0100
#define KEEP_FLAGS      0XFFFF 
#define REMOVE_FLAGS	  0X007F	

class Sequence;
class SequenceGroup;

typedef QSharedPointer< Sequence > SequencePtr;

class Sequence
{
	public:
		Sequence();
		Sequence(QString,QString,QString c=QString(),QString f=QString(),bool vis=true);
		~Sequence();
		// comment is for a longer comment
		QString label,residues,comment;
		
		QString filter(bool applyExclusions=false);
		void exclude(int,int,bool);
		QList<int> exclusions(); // returned as a flat list of [start,end] pairs
		
		void highlight(int ,int,bool );
		
		void remove(int,int);
		void insert(QString,int);
		
		bool visible;
		bool bookmarked;
		
		SequenceGroup *group;
		
		QString source; // file sequence was originally sourced from
		
};

#endif

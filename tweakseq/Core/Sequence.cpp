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

#include "Sequence.h"
#include "SequenceGroup.h"

#define LABELWIDTH 10 // FIXME
#define EXCLUDE_CELL 0x0080 // FIXME

Sequence::Sequence()
{
}

Sequence::Sequence(QString l,QString r,QString c,QString f){
	label = l;
	while (label.length()<LABELWIDTH) // FIXME this should be done elsewhere
		label.append(" ");
	residues = r;
	comment=c;
	group=NULL;
	source=f;
	//cerr << "Residues " << r.latin1() << endl;
}

Sequence::~Sequence()
{
}

QString Sequence::filter(bool applyExclusions)
{
	QString r;
	int rescnt=0;
	for (int i=0;i<residues.size();i++){
		QChar qch=residues[i];
		if ((qch.unicode() & EXCLUDE_CELL) && applyExclusions)
			continue;
		r[rescnt] = qch.unicode() & 0x7F; // FIXME
		rescnt++;
	}
	return r;
}

void Sequence::exclude(int start,int stop)
{
	if (start <0 || stop >= residues.size()) return;
			
	for (int i=start;i<=stop;i++)
		residues[i] = residues[i].unicode() | EXCLUDE_CELL;
}

// Returned as a flat list of [start,end] pairs
QList<int> Sequence::exclusions()
{
	QList<int> x;
	int xstart,xstop,i,j;
	for (i=0;i<residues.size();i++){
		QChar qch=residues[i];
		if (qch.unicode() & EXCLUDE_CELL){ // start of an excluded block
			xstart = i;xstop=i;
			for (j=i+1;j<residues.size();j++){
				qch=residues[j];
				if (!(qch.unicode() & EXCLUDE_CELL)){ // end of excluded block
					xstop=j-1;
					x.append(xstart);x.append(xstop);
					break;
				}
			}
			
			if (j==residues.size()){ // catch an exclusion that runs to the end
				xstop=j-1;
				x.append(xstart);x.append(xstop);
			}
			
			i=j;
		}
	}	
	return x;
}

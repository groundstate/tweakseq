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

#define LABELWIDTH 10 // FIXME

Sequence::Sequence()
{
}

Sequence::Sequence(QString l,QString r,QString c){
	label = l;
	while (label.length()<LABELWIDTH) // FIXME this should be done elsewhere
		label.append(" ");
	residues = r;
	comment=c;
	group=-1;
	//cerr << "Residues " << r.latin1() << endl;
}

Sequence::~Sequence()
{
}

QString Sequence::noFlags()
{
	QString r;
	for (int i=0;i<residues.size();i++){
		QChar qch=residues[i];
		r[i] = qch.unicode() & 0x7F; // FIXME
	}
	return r;
}

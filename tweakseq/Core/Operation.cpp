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


#include "Operation.h"
#include "Sequence.h"


Operation::Operation(int eMode,int startR,int stopR,int startC,int stopC,Q3StrList t){

	mode = eMode;
	startRow = startR;
	stopRow = stopR;
	startCol = startC;
	stopCol= stopC;
	editText = t;
}

Operation::Operation(int eMode,QList <Sequence *> s)
{
	// Constructor to store contents of the sequence editor
	
	mode=eMode;
	for ( int i=0;i<s.count();i++)
		seq.append(new Sequence(s.at(i)->label,s.at(i)->residues)); // CHECKME
}

Operation::~Operation()
{
	//editText.~QStrList(); // Check THIS
	//if (!seq.isEmpty()) seq.~QList(); // CHECK
}



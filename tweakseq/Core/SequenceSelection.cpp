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


#include "SequenceSelection.h"

//
// 	Public members
//

SequenceSelection::SequenceSelection()
{
}

SequenceSelection::~SequenceSelection()
{
}

void SequenceSelection::set(Sequence *s)
{
	sel_.clear();
	sel_.append(s);
	emit changed();
}

void SequenceSelection::toggle(Sequence *s)
{
	// If it's there, remove it
	if (sel_.contains(s)){
		sel_.removeOne(s);
	}
	// else add it
	else
		sel_.append(s);
	emit changed();
}

void SequenceSelection::clear()
{
	sel_.clear();
	emit changed();
}

bool SequenceSelection::contains(Sequence *s)
{
	return sel_.contains(s);
}

Sequence * SequenceSelection::itemAt(int i)
{
	if (i <0 || i >= sel_.size())
		return NULL;
	
	return sel_.at(i);
	
}

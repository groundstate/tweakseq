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

#include "Sequence.h"
#include "SequenceGroup.h"

//
// Sequences can be added in any order to a group and ARE NOT ORDERED
// Keeping them ordered seems too complicated
//

SequenceGroup::SequenceGroup()
{
	locked_=false;
	textColour_ = QColor("white");
}

SequenceGroup::~SequenceGroup()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	for (int s=0;s<seqs_.size();s++) // FIXME this will go
		seqs_.at(s)->group=NULL;
}

void SequenceGroup::addSequence(Sequence *s)
{
	if (seqs_.contains(s))
		return;
	s->group=this;
	seqs_.append(s);
}

void SequenceGroup::removeSequence(Sequence *s)
{
	if (!(seqs_.contains(s)))
		return;
	s->group = NULL; // out of the club :-)
	seqs_.removeAll(s);
	// the group may be empty now but we don't delete it here - maybe we have cleared it only temporarily
}

bool SequenceGroup::contains(Sequence *s)
{
		return seqs_.contains(s);
}

Sequence * SequenceGroup::itemAt(int i)
{
	if (i <0 || i >= seqs_.size())
		return NULL;
	
	return seqs_.at(i);
	
}

void SequenceGroup::clear()
{
	for (int s=0;s<seqs_.size();s++)
		seqs_.at(s)->group=NULL;
	seqs_.clear();
}

bool SequenceGroup::hasHiddenSequences()
{
	for (int s=0;s<seqs_.size();s++){
		if (!seqs_.at(s)->visible)
			return true;
	}
	return false;
}

void SequenceGroup::enforceVisibility()
{
	// After removing sequences, none may be visible - so if there are no visible sequences,
	// make them visible again
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	
	for (int s=0;s<seqs_.size();s++){
		if (seqs_.at(s)->visible)
			return; // nothing more to do
	}

	// None visible, so make all visible
	for (int s=0;s<seqs_.size();s++)
		seqs_.at(s)->visible=true;
	
}

		
	
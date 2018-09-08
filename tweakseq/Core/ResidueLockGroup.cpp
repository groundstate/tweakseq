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

#include "ResidueLockGroup.h"
#include "Sequence.h"

ResidueLockGroup::ResidueLockGroup()
{
	position_=0;
	update();
}

ResidueLockGroup::ResidueLockGroup(QList<Sequence *> &seqs,int pos)
{
	sequences = seqs;
	position_=pos;
	update();
}

ResidueLockGroup::~ResidueLockGroup()
{
}
		
void ResidueLockGroup::move(int delta)
{
	position_ += delta;
	if (position_ < 0) position_=0;
	if (position_ > minLength_)
		position_ = minLength_;
}

int  ResidueLockGroup::position()
{
	return position_;
}

void ResidueLockGroup::setPosition(int pos)
{
	position_=pos;
}

void ResidueLockGroup::addSequence(Sequence *seq)
{
	seq->residueLockGroup=this;
	if (sequences.size() == 0)
		minLength_=seq->residues.length(); // initialize
	sequences.append(seq);
	if (seq->residues.length() < minLength_)
		minLength_=seq->residues.length();
}

void ResidueLockGroup::removeSequence(Sequence *seq)
{
	for (int s=0;s<sequences.length();s++){
		if (sequences.at(s) == seq){
			sequences.removeAt(s);
			seq->residueLockGroup=NULL;
			break;
		}
	}
	update();
}

bool ResidueLockGroup::empty()
{
	return sequences.empty();
}

void ResidueLockGroup::update()
{
	
	if (sequences.size() > 0)
		minLength_= sequences.at(0)->residues.length();
	else
		minLength_=0;
	
	for (int s=0;s<sequences.size();s++){
		if (sequences.at(s)->residues.length() < minLength_)
			minLength_=sequences.at(s)->residues.length();
	}
}

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

#include <QtDebug>
#include "DebuggingInfo.h"

#include "Sequence.h"
#include "Sequences.h"
#include "SequenceGroup.h"

Sequences::Sequences()
{
}

Sequences::~Sequences()
{
}


bool Sequences::isEmpty()
{
	return sequences_.empty();
}

// Returns size() - number of hidden sequences
int Sequences::visibleSize()
{
		int nvis=0;
		for (int i=0;i<sequences_.size();i++){
			if ((sequences_.at(i)->visible)) 
				nvis++;
		}
		return nvis;
}

void Sequences::clear()
{
	while (!sequences_.empty()){
		Sequence *seq = sequences_.takeFirst();
		if (seq->group)
			seq->group->removeSequence(seq);
		delete seq;
	}
	emit cleared();
	emit changed();
}

Sequence * Sequences::visibleAt(int pos)
{
	int visIndex=0;
	for (int i=0;i<sequences_.size();i++){
		Sequence *seq = sequences_.at(i);
		if (visIndex == pos && seq->visible)
			return seq;
		if (seq->visible)
			visIndex++;
	}
	return NULL;
}

// Convert index of visible sequence to actual index
int Sequences::visibleToActual(int pos)
{
	int visIndex=0;
	for (int i=0;i<sequences_.size();i++){
		Sequence *seq = sequences_.at(i);
		if (visIndex == pos)
			return i;
		if (seq->visible)
			visIndex++;
	}
	return 0; // FIXME
}

Sequence * Sequences::add(QString l,QString s,QString c,QString f,bool h)
{
	Sequence * newSeq = new Sequence(l,s,c,f,h);
	sequences_.append(newSeq);
	emit sequenceAdded(newSeq);
	emit changed();
	return newSeq;
}

void  Sequences::append(Sequence *newSequence)
{
	sequences_.append(newSequence);
	emit changed();
	emit sequenceAdded(newSequence);
}


void Sequences::remove(QString)
{
	// FIXME not implemented
	emit changed();
}

void Sequences::insert(QString,QString,int)
{
	// FIXME not implemented
	emit changed();
}

void  Sequences::insert(Sequence *seq,Sequence *after,bool postInsert)
{
	int s;
	for (s=0;s<sequences_.size();s++){
		if (sequences_.at(s) == after)
			break;
	}
	if (s<sequences_.size()){
		sequences_.insert(s+1,seq); // postInsert
	}
}


void  Sequences::replace(QString oldLabel,QString newLabel,QString newResidues)
{
	int i;
	if ((i=getIndex(oldLabel))>=0){ // found it
		remove(oldLabel);
		insert(newLabel,newResidues,i);
	}
	emit changed();
}

void Sequences::move(int oldPos,int newPos)
{
	if (oldPos<0 || newPos <0 || oldPos >= sequences_.size() || newPos >= sequences_.size()){
		return;
	}
	sequences_.move(oldPos,newPos);
	emit changed();
}

void Sequences::replaceResidues(QString newResidues,int pos)
{
	sequences_.at(pos)->residues=newResidues;
	emit changed();
}

void  Sequences::addInsertions(int startSequence,int stopSequence,int startPos,int nInsertions)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << startSequence << " " << stopSequence << " " << startPos << " " << nInsertions;
	QString ins(nInsertions,'-');
	for (int s=startSequence; s<=stopSequence; s++){
		sequences_.at(s)->residues.insert(startPos,ins);
	}
	emit changed();
}

void  Sequences::addInsertions(Sequence *seq,int startPos,int nInsertions)
{
	QString ins(nInsertions,'-');
	seq->residues.insert(startPos,ins);
}

// Mainly used for removing insertions
void  Sequences::removeResidues(int startSequence,int stopSequence,int startPos,int nResidues) 
{
	for (int s=startSequence; s<=stopSequence; s++){
		sequences_.at(s)->residues.remove(startPos,nResidues);
	}
	emit changed();
}

		
int Sequences::getIndex(QString label)
{
	// Get the index of the sequence with label l
	// Returns -1 if no match
	int i=0;
	QString t=label.stripWhiteSpace();// TO DO why is this here ?
	while ((i<sequences_.count()) && (getLabelAt(i) != t)) i++;
	if (i==sequences_.count())
		return -1;
	else
		return i;
}

QString Sequences::getLabelAt(int i)
{
	if (i<0 || i > sequences_.size()-1)
		return QString(); // Return NULL if the index is out of range
	else
		// strip white space from the end of the string
		return (sequences_.at(i)->label).stripWhiteSpace(); 
}


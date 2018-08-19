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
	maxLen_=0;
}

Sequences::~Sequences()
{
}

void Sequences::forceCacheUpdate()
{
	updateCachedVariables();
}

int Sequences::index(Sequence *seq)
{
	for (int s=0;s<sequences_.size();s++){
		if (sequences_.at(s) == seq)
			return s;
	}
	return -1;
}

Sequence* Sequences::getSequence(const QString &label)
{
	QString t=label.trimmed();// TO DO why is this here ?
	int i=0;
	while ((i<sequences_.count()) && (getNameAt(i) != t)) i++;
	
	if (i==sequences_.count())
		return NULL;
	else
		return sequences_.at(i);
}

int Sequences::visibleIndex(Sequence *seq)
{
	if (!(seq->visible)) return -1;
	int visIndex=0;
	for (int i=0;i<sequences_.size();i++){
		if (seq == sequences_.at(i))
			return visIndex;
		if (sequences_.at(i)->visible)
			visIndex++;
	}
	return -1;
}

bool Sequences::isEmpty()
{
	return sequences_.empty();
}

// Returns size() - number of hidden sequences
int Sequences::numVisible()
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
	}
	updateCachedVariables();
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
		if (visIndex == pos && seq->visible)
			return i;
		if (seq->visible)
			visIndex++;
	}
	return 0; // FIXME
}

int Sequences::maxLength(bool recalculate)
{
	if (recalculate){
		updateCachedVariables();
	}
	return maxLen_;
}


// Check whether the group defined by [start,stop] belongs to a contiguous subgroup
bool Sequences::isSubGroup(int start,int stop)
{
	if (sequences_.at(start)->group == NULL  || sequences_.at(start)->group == NULL) 
		return false;
	SequenceGroup *sg = sequences_.at(start)->group;
	for (int s=start;s<=stop;s++){
		if (sequences_.at(s)->group !=sg)
			return false;
	}
	return true;
}

bool Sequences::isUniqueName(QString &name)
{
	for (int i=0;i<sequences_.size();i++){
		if (name == sequences_.at(i)->name)
			return false;
	}
	return true;
}

Sequence * Sequences::append(QString l,QString r,QString c,QString f,bool h)
{
	Sequence * newSeq = new Sequence(l,r,c,f,h);
	sequences_.append(newSeq);
	if (r.length() > maxLen_)
		maxLen_=r.length();
	emit changed();
	return newSeq;
}

void  Sequences::append(Sequence *newSequence)
{
	sequences_.append(newSequence);
	int len = newSequence->residues.length();
	if (len > maxLen_)
		maxLen_=len;
	emit changed();
}

void Sequences::append(QList<Sequence *> &seqs)
{
	sequences_.append(seqs);
	updateCachedVariables();
	emit changed();
}

void  Sequences::set(QList<Sequence *> &seqs)
{
	sequences_=seqs;
	updateCachedVariables();
	emit changed();
}

void Sequences::remove(QString)
{
	// FIXME not implemented
	emit changed();
}

void Sequences::remove(QList<Sequence *> &seqs)
{
	for (int s=0;s<seqs.size();s++)
		sequences_.removeOne(seqs.at(s));
	updateCachedVariables();
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
	int len = seq->residues.length();
	if (len > maxLen_)
		maxLen_=len;
	emit changed();
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
	updateCachedVariables();
	emit changed();
}

void  Sequences::addInsertions(int startSequence,int stopSequence,int startPos,int nInsertions)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << startSequence << " " << stopSequence << " " << startPos << " " << nInsertions;
	QString ins(nInsertions,'-');
	for (int s=startSequence; s<=stopSequence; s++){
		sequences_.at(s)->residues.insert(startPos,ins);
		int len = sequences_.at(s)->residues.length();
		if (len > maxLen_)
			maxLen_=len;
	}
	emit changed();
}

void  Sequences::addInsertions(Sequence *seq,int startPos,int nInsertions)
{
	QString ins(nInsertions,'-');
	seq->residues.insert(startPos,ins);
	int len = seq->residues.length();
	if (len > maxLen_)
		maxLen_=len;
	emit changed();
}

// Mainly used for removing insertions
void  Sequences::removeResidues(int startSequence,int stopSequence,int startPos,int nResidues) 
{
	for (int s=startSequence; s<=stopSequence; s++){
		sequences_.at(s)->residues.remove(startPos,nResidues);
	}
	updateCachedVariables(); // must recalculate
	emit changed();
}

void Sequences::removeResidues(Sequence *seq,int startPos,int nResidues)
{
	seq->residues.remove(startPos,nResidues);
	updateCachedVariables(); // must recalculate
	emit changed();
}

void  Sequences::unhideAll()
{
	for (int s=0;s<sequences_.count();s++)
		sequences_.at(s)->visible=true;
	emit changed();
}
		

int Sequences::getIndex(QString label)
{
	// Get the index of the sequence with label l
	// Returns -1 if no match
	int i=0;
	QString t=label.trimmed();// TO DO why is this here ?
	while ((i<sequences_.count()) && (getNameAt(i) != t)) i++;
	if (i==sequences_.count())
		return -1;
	else
		return i;
}

QString Sequences::getNameAt(int i)
{
	if (i<0 || i > sequences_.size()-1)
		return QString(); // Return NULL if the index is out of range
	else
		// strip white space from the end of the string
		return (sequences_.at(i)->name).trimmed(); 
}

void Sequences::updateCachedVariables()
{
	maxLen_=0;
	for (int s=0;s<sequences_.count();s++){
		int len =sequences_.at(s)->residues.length();
		if (len> maxLen_)
			maxLen_=len;
	}
}
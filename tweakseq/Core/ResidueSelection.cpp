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

#include "ResidueSelection.h"
#include "Sequence.h"
#include "SequenceGroup.h"

ResidueSelection::ResidueSelection()
{
}

ResidueSelection::~ResidueSelection()
{
	clear();
}

void ResidueSelection::set(QList<ResidueGroup *> &newSelection)
{
	clear();
	for (int s=0;s<newSelection.size();s++)
		sel_.append(newSelection.at(s)); // assuming that newSelection is not deleted !
	emit changed();
}

QString ResidueSelection::selectedResidues(int i)
{
	QString res = sel_.at(i)->sequence->filter();
	res = res.mid(sel_.at(i)->start,sel_.at(i)->stop - sel_.at(i)->start + 1);
	return res;
}

void ResidueSelection::clear()
{
	sel_.clear();
	emit changed();
}

bool ResidueSelection::isInsertionsOnly()
{
	if (0 == sel_.size()) return false;
	
	for (int s=0;s<sel_.size();s++){
		ResidueGroup *rg = sel_.at(s);
		QString residues = rg->sequence->filter(); // marks stripped
		for (int r=rg->start;r<=rg->stop;r++){
			if (residues.at(r) != '-')
				return false;
		}
	}
	return true;
}

bool ResidueSelection::isLocked()
{
	if (0 == sel_.size()) return false;
	for (int s=0;s<sel_.size();s++){
		ResidueGroup *rg = sel_.at(s);
		if (rg->sequence->residueLockGroup)
			return true;
	}
	return false;
}

ResidueGroup * ResidueSelection::itemAt(int index)
{
	return sel_.at(index);
}

QList<SequenceGroup *> ResidueSelection::uniqueSequenceGroups()
{
	QList<SequenceGroup *> sgl;
	for (int s=0;s<sel_.size();s++){
		ResidueGroup *rg = sel_.at(s);
		Sequence *seq = rg->sequence;
		if (seq->group){
			if (!sgl.contains(seq->group)){
				sgl.append(seq->group);
			}
		}
	}
	return sgl;
}


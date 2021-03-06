//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2018  Merridee A. Wouters, Michael J. Wouters
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
#include "SequenceSelection.h"
#include "GroupCmd.h"
#include "Project.h"

GroupCmd::GroupCmd(Project *project,const QList<Sequence *> &seqs,const QList<SequenceGroup*> &mergeGroups,
	const QColor &groupColour,const QString &txt):Command(project,txt)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	seqs_=seqs;
	mergeGroups_=mergeGroups;
	groupColour_=groupColour;
	newGroup_= new SequenceGroup(); // create once
}

GroupCmd::~GroupCmd()
{
}

void GroupCmd::redo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// No need to remove sequences from the group because the group will be updated
	for (int g=0;g<mergeGroups_.size();g++){
		project_->sequenceGroups.removeOne(mergeGroups_.at(g)); // but don't delete it
	}
	
	// All sorted, so initialize the group
	newGroup_->setTextColour(groupColour_);
	project_->sequenceGroups.append(newGroup_);
	for ( int s=0;s<seqs_.size();s++){
		newGroup_->addSequence(seqs_.at(s));
	}
}

void GroupCmd::undo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	newGroup_->clear();
	project_->sequenceGroups.removeOne(newGroup_);
	
	for (int g=0;g<mergeGroups_.size();g++){
		SequenceGroup *sg = mergeGroups_.at(g);
		for (int s=0;s<sg->size();s++)
			sg->itemAt(s)->group=sg; // already in the group
		project_->sequenceGroups.append(sg);
	}
	
	project_->sequenceSelection->set(seqs_);
}
		

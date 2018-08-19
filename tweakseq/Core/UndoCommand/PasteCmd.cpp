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

#include "Application.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "PasteCmd.h"
#include "Project.h"


PasteCmd::PasteCmd(Project *project,Sequence *insertAfter,const QString &txt):Command(project,txt)
{
	insertAfter_=insertAfter;
	clipboardContents_ = app->clipboard().sequences();
}

PasteCmd::~PasteCmd()
{
}

void PasteCmd::redo()
{
	oldAligned_ = project_->aligned();
	
	Sequence * lastSequence = insertAfter_;
	for (int s=0;s<clipboardContents_.size();s++){
		Sequence *seq = clipboardContents_.at(s);
		project_->sequences.insert(seq,lastSequence);
		lastSequence = seq; // so that we insert after the last insertion
	}
	// Isolated sequences which have been cut from their group will have been removed from their group but
	// still retain a pointer to the group. These are must have their group pointer set to NULL.
	// If a whole group(s) has been selected, group membership is retained and the group(s) must be added
	// back into the Project. The appended groups are saved so that the action can be undone.
	cutGroups_.clear();
	allGroups_.clear();
	qDebug() << trace.header(__PRETTY_FUNCTION__) << project_->sequenceGroups.size();
	for (int s=0;s<clipboardContents_.size();s++){
		SequenceGroup *sg = clipboardContents_.at(s)->group;
		if (NULL != sg){
			allGroups_.append(sg);
			Sequence *seq = clipboardContents_.at(s);
			if (!sg->contains(seq)){ // orphaned
				seq->group=NULL;
				qDebug() << trace.header(__PRETTY_FUNCTION__) << "orphaned sequence" << seq->name;
			}
			else{
				if (!(project_->sequenceGroups.contains(sg))){
					project_->sequenceGroups.append(sg);
					cutGroups_.append(sg);
				}
			}
		}
		else{
			allGroups_.append(NULL);
		}
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << project_->sequenceGroups.size();
	app->clipboard().clear(); // you only paste once
}

void PasteCmd::undo()
{
	project_->sequences.remove(clipboardContents_);
	for (int s=0;s<clipboardContents_.size();s++){
		clipboardContents_.at(s)->group=allGroups_.at(s); // restore groups to orphans 
	}
	
	qDebug() << trace.header(__PRETTY_FUNCTION__) << project_->sequenceGroups.size();
	for (int g=0;g<cutGroups_.size();g++)
		project_->sequenceGroups.removeOne(cutGroups_.at(g));
	qDebug() << trace.header(__PRETTY_FUNCTION__) << project_->sequenceGroups.size();
	app->clipboard().setSequences(clipboardContents_);
	project_->setAligned(oldAligned_);
}


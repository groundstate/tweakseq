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
#include "UngroupCmd.h"
#include "Project.h"

UngroupCmd::UngroupCmd(Project *project,const QString &txt):Command(project,txt)
{
	SequenceSelection *sel= project_->sequenceSelection;
	oldSelection_= sel->sequences(); // this is to keep track of sequences for undo()
	
	// Visibility is modified so save this information
	for ( int s=0;s<sel->size();s++){
		oldVisibility_.append(sel->itemAt(s)->visible);
		oldSelectionGroups_.append(sel->itemAt(s)->group);
	}
}

UngroupCmd::~UngroupCmd()
{
}

void UngroupCmd::redo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	cutGroups_.clear();
	
	// Make everything in the selection visible so that we don't lose the non-visible items after ungrouping
	// If a full group has been selected, then all its members are presumed to be in the selection
	for ( int s=0;s<oldSelection_.size();s++)
		oldSelection_.at(s)->visible=true;
	
	// Any grouped sequence that is in the selection is removed from its group
	// If this leaves only one sequence in the group, this is OK
	for ( int s=0;s<oldSelection_.size();s++){
		Sequence *seq = oldSelection_.at(s);
		if (seq->group){
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "ungrouping " << seq->label;
			SequenceGroup *sg = seq->group; // save this, cos removing it from the groups sets ptr to NULL
			seq->group->removeSequence(seq);  
			// if we have now removed all of the visible sequences in the group, make the hidden sequences
			// visible again
			sg->enforceVisibility(); // some redundancy here because we have already made fully selected sequence visible
		}
	}
	
	// Remove any empty groups
	int g=0;
	while (g<project_->sequenceGroups.size()){
		SequenceGroup *sg = project_->sequenceGroups.at(g);
		if (sg->size() == 0){
			cutGroups_.append(sg); // save it
			project_->sequenceGroups.removeOne(sg); 
			qDebug() << trace.header(__PRETTY_FUNCTION__)  << "removing empty group";
		}
		else
			g++;
	}
	
}

void UngroupCmd::undo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// Restore sequence sequence visibility and groups
	for ( int s=0;s<oldSelection_.size();s++){
		oldSelection_.at(s)->visible  = oldVisibility_.at(s);
		oldSelection_.at(s)->group = oldSelectionGroups_.at(s);
		if (NULL != oldSelection_.at(s)->group){
			oldSelectionGroups_.at(s)->addSequence(oldSelection_.at(s));
		}
	}
	// Restore cut groups
	for (int g=0;g<cutGroups_.size();g++)
		project_->sequenceGroups.append(cutGroups_.at(g));
	
	project_->sequenceSelection->set(oldSelection_); // restore the selection
}

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

#include "Application.h"
#include "CutSequencesCmd.h"
#include "Clipboard.h"
#include "Project.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SequenceSelection.h"

CutSequencesCmd::CutSequencesCmd(Project *project, const QString &txt):Command(project,txt)
{
	// This is what we need to undo()
	clipboardContents_ = app->clipboard().sequences(); // prior to cut
	sequenceSelection_.set(project_->sequenceSelection->sequences());
	seqs_=project_->sequences.sequences();
}

CutSequencesCmd::~CutSequencesCmd()
{
}

void CutSequencesCmd::redo()
{
	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	oldAligned_ = project_->aligned();
	
	QList<Sequence*> &seqs = project_->sequences.sequences();
	cutGroups_.clear();
	
	int s=0;
	QList<Sequence *> cutSeqs;
	// Duplicating the list of cut sequences this way orders the selection in
	// the correct order, recognizing that the selection may have been chosen in random order
	for (s=0;s<seqs.size();s++){
		if (project_->sequenceSelection->contains(seqs.at(s)))
			cutSeqs.append(seqs.at(s));
	}
	
	// Now we have to check whether any groups have been selected and include any
	// non-visible items in the right order
	// The convention is that if all of the visible items in a  a group have been selected
	// then the whole group is selected
	QList<SequenceGroup *> sgl =  project_->sequenceSelection->uniqueGroups();
	for( int g=0; g < sgl.size(); g++ ){
		SequenceGroup *sg = sgl.at(g);
		bool groupSelected=true;
		for (s=0; s< sg->size(); s++){
			Sequence *seq = sg->itemAt(s);
			if (seq->visible && !project_->sequenceSelection->contains(seq)){
				groupSelected=false;
				break;
			}
		}
		if (groupSelected){
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "all visible sequences in group were cut : size = " << sg->size();
			// Create the ordered list of sequences in the group
			QList<Sequence *> groupSeqs;
			for (s=0;s<seqs.size();s++){
				if (sg->contains(seqs.at(s))){
					groupSeqs.append(seqs.at(s));
				}
			}
			// Now remove the (duplicated) visible group members from the list of cut sequences
			s=0;
			while (s<cutSeqs.size()){
				Sequence *seq = cutSeqs.at(s);
				if (sg->contains(seq))
					cutSeqs.takeAt(s);
				else
					s++;
			}
			for (s=0;s<groupSeqs.size();s++) // Don't try to insert in the 'right' place
				cutSeqs.append(groupSeqs.at(s));
			cutGroups_.append(sg);
			project_->sequenceGroups.removeOne(sg);
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "cut group ";
		}
		else{
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "group partially selected - removing sequences from the group before cut";
			int s=0;
			while ( s< sg->size()){
				Sequence *seq = sg->itemAt(s);
				if (project_->sequenceSelection->contains(seq))
					sg->cutSequence(seq); // doesn't remove the group from the Sequence
				else
					s++;
			}
		}
	}
	
	// Order the cut sequences (again)
	// This is easier than trying to insert in the right place
	
	QList<Sequence *> orderedCutSeqs;
	for (s=0;s<seqs.size();s++){
		Sequence *seq = seqs.at(s);
		if (cutSeqs.contains(seq)){
			orderedCutSeqs.append(seq);
			qDebug() << trace.header(__PRETTY_FUNCTION__) << seq->name << " at " << s;
		}
	}

	s=0;
	while (s<seqs.size()){
		if (cutSeqs.contains(seqs.at(s)))
			seqs.takeAt(s);
		else
			s++;
	}
	
	cutSeqs_=orderedCutSeqs;
	
	clipboardContents_ = app->clipboard().sequences(); // save the clipboard, so it can be restored
	app->clipboard().setSequences(cutSeqs_); // this removes whatever was there
	project_->sequenceSelection->clear(); // cut, so nothing is selected now
	project_->setAligned(false);
}

void CutSequencesCmd::undo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// Restore cut groups
	for (int g=0;g<cutGroups_.size();g++)
		project_->sequenceGroups.append(cutGroups_.at(g));
	
	// Restore cut sequences to their group (not necessary if the whole group was cut, but do it anyway)
	for (int s=0;s<cutSeqs_.size();s++){
		if (NULL != cutSeqs_.at(s)->group)
			cutSeqs_.at(s)->group->addSequence(cutSeqs_.at(s)); // if already there, it's ignored
	}
	project_->sequences.set(seqs_);
	project_->sequenceSelection->set(sequenceSelection_.sequences()); // restoring this means the selection will also be shown
	app->clipboard().setSequences(clipboardContents_);
	project_->setAligned(oldAligned_);
}
		

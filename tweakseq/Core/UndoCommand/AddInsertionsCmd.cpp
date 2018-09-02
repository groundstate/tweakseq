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

#include "AddInsertionsCmd.h"
#include "Project.h"
#include "ResidueLockGroup.h"
#include "Sequence.h"
#include "Sequences.h"

AddInsertionsCmd::AddInsertionsCmd(Project *project,QList<Sequence *> &seqs,int startCol,int stopCol,bool postInsert,const QString &txt):
	Command(project,txt)
{
	
	nInsertions_=stopCol-startCol+1;
	startPos_=startCol;
	if (startCol > stopCol){
		startPos_= stopCol;
	}
	if (postInsert) startPos_++;
	seqs_=seqs;
	aligned_=project_->aligned();
	lockConstraint_=false;
}

AddInsertionsCmd::~AddInsertionsCmd()
{
}


void AddInsertionsCmd::redo()
{
	// If the sequences are all within the same residue lock group, but not all
	// of the lock group members are selected, then ...
	// First, is there a ResidueLockGroup ?
	ResidueLockGroup *rlg=NULL;
	for (int s=0;s<seqs_.size();s++){
		if (seqs_.at(s)->residueLockGroup != NULL){
			rlg = seqs_.at(s)->residueLockGroup;
			break;
		}
	}
	
	if (rlg != NULL){ // check that all sequences are in the RLG
		lockConstraint_=true;
		for (int s=0;s<seqs_.size();s++){
			if (seqs_.at(s)->residueLockGroup !=rlg){
				lockConstraint_ = false;
				break;
			}
		}
	}
	
	for (int s=0;s<seqs_.size();s++)
		project_->sequences.addInsertions(seqs_.at(s),startPos_,nInsertions_);
	
	if (lockConstraint_){
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "residue lock constraint applies at " << rlg->position();
		// now we have to add insertions just before the locked residue to the other sequences in the lock group 
		bool moved=false;
		for (int s=0;s<rlg->sequences.size();s++){
			if (!(seqs_.contains(rlg->sequences.at(s)))){
				project_->sequences.addInsertions(rlg->sequences.at(s),rlg->position()-1,1);
				moved=true;
			}
		}
		if (moved) rlg->move(1);
	}
		
	project_->setAligned(false); 
}

void AddInsertionsCmd::undo()
{
	for (int s=0;s<seqs_.size();s++){
		project_->sequences.removeResidues(seqs_.at(s),startPos_,nInsertions_);
	}
	if (aligned_) project_->setAligned(aligned_);
}


bool AddInsertionsCmd::mergeWith(const QUndoCommand *other)
{
	if (lockConstraint_)
		return false;
	
	// NB 'other' is the newest
	if (other->id() != id()) 
		return false;
	const AddInsertionsCmd *cmd = static_cast<const AddInsertionsCmd *>(other);
	
	qDebug() << trace.header(__PRETTY_FUNCTION__)  << startPos_ << " " << cmd->startPos_;
	
	// Sequences must match
	if (cmd->seqs_.size() != seqs_.size()) return false;
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "size ok";
	for (int s=0;s<seqs_.size();s++){
		if (!cmd->seqs_.contains(seqs_.at(s))) return false;
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "seqs match";
	
	
	if (cmd->startPos_>=startPos_ && cmd->startPos_ <= startPos_ + nInsertions_){
		qDebug() << "GOOD " << nInsertions_;
		nInsertions_+= cmd->nInsertions_;
		return true;
	}
	
	return false;
}
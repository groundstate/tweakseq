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
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "start=" << startCol << " " << "stop=" << stopCol;
	seqs_=seqs;
	nInsertions_=stopCol-startCol+1;
	startPos_=startCol;
	if (startCol > stopCol){
		startPos_= stopCol;
	}
	if (postInsert) startPos_++;
	
	// Determine the effects of constraints
	// First, is there a ResidueLockGroup ?
	residueLockGroup_=NULL;
	for (int s=0;s<seqs_.size();s++){
		if (seqs_.at(s)->residueLockGroup != NULL){
			residueLockGroup_ = seqs_.at(s)->residueLockGroup;
			break;
		}
	}
	
	if (residueLockGroup_ != NULL){ // check that all sequences are in the RLG
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "possible residue lock group";
		for (int s=0;s<seqs_.size();s++){
			if (seqs_.at(s)->residueLockGroup !=residueLockGroup_){
				residueLockGroup_=NULL;
				break;
			}
		}
	}
	
	qDebug() << trace.header(__PRETTY_FUNCTION__) << ((residueLockGroup_!=NULL)?"lock constraint ":"no constraint");
	
	if (residueLockGroup_){
		// Count the number of insertions that occur after the insertion point and before the locked residues
		// for each sequence in the selection
		// The maximum number of insertions that can be made is the minimum 
		int minInsertions = -1; // should be at least one
		
		for (int s=0;s<residueLockGroup_->sequences.size();s++){
			Sequence *seq = residueLockGroup_->sequences.at(s);
			int nInsertions=0;
 			if (seqs_.contains(seq)){ // consider only those selected sequences which are part of the locked group
				nInsertions=seq->numInsertions(startPos_,residueLockGroup_->position()-1);
				qDebug() << trace.header(__PRETTY_FUNCTION__) << seq->name << " nInsertions=" << nInsertions;
				if (minInsertions < 0 || nInsertions < minInsertions)
				 minInsertions = nInsertions;
			}
		}
		qDebug() << trace.header(__PRETTY_FUNCTION__)  << "minimum insertions=" << minInsertions;
		if (minInsertions < nInsertions_)
			nInsertions_=minInsertions;
	}
	
	aligned_=project_->aligned();
	
}

AddInsertionsCmd::~AddInsertionsCmd()
{
}


void AddInsertionsCmd::redo()
{
	
	// Apply any constraints
	if (residueLockGroup_){
		for (int s=0;s<seqs_.size();s++){
			Sequence *seq=seqs_.at(s);
			for (int i=0;i<nInsertions_;i++){ // insertions cannot be assumed to be in a block
				for (int r=residueLockGroup_->position()-i;r>=0;r--){ // decrement position() to account for deletions
					if(seq->isInsertion(r)){
						seq->remove(r,1);
						break;
					}
				}
			}
		}
	}
	
	for (int s=0;s<seqs_.size();s++)
		project_->sequences.addInsertions(seqs_.at(s),startPos_,nInsertions_);
	
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
	if (residueLockGroup_) return false;
	
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
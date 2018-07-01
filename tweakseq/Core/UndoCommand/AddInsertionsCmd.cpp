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

}

AddInsertionsCmd::~AddInsertionsCmd()
{
}


void AddInsertionsCmd::redo()
{
	for (int s=0;s<seqs_.size();s++){
			project_->sequences.addInsertions(seqs_.at(s),startPos_,nInsertions_);
	}
}

void AddInsertionsCmd::undo()
{
	for (int s=0;s<seqs_.size();s++){
		project_->sequences.removeResidues(seqs_.at(s),startPos_,nInsertions_);
	}
}


bool AddInsertionsCmd::mergeWith(const QUndoCommand *other)
{
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
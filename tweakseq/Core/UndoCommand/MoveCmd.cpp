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

#include "MoveCmd.h"
#include "Project.h"
#include "Sequence.h"
#include "Sequences.h"

MoveCmd::MoveCmd(Project *project,QList<Sequence *> &seqs,int delta,const QString &txt):Command(project,txt)
{
	delta_=delta;
	
	// Sort the sequences into descending row order so that the logic of move operations is simple
	Sequences & allseqs =  project_->sequences;
	int nAllSeqs=allseqs.size();
	for (int s=0;s<nAllSeqs;s++){
		Sequence *seq = allseqs.sequences().at(s);
		if (seqs.contains(seq)){
			sortseqs_.append(seq);
		}
	}
	
}

MoveCmd::~MoveCmd()
{
}


void MoveCmd::redo()
{

	if (delta_>0){
		for (int s=sortseqs_.size()-1;s>=0;s--){
			int index = project_->sequences.index(sortseqs_.at(s));
			project_->sequences.move(index, index+delta_);
		}
	}
	else{
		for (int s=0;s<sortseqs_.size();s++){
			int index = project_->sequences.index(sortseqs_.at(s));
			project_->sequences.move(index, index+delta_);
		}
	}
	
}

void MoveCmd::undo()
{
	if (delta_>0){
		for (int s=0;s<sortseqs_.size();s++){ // reverse order of redo()
			int index = project_->sequences.index(sortseqs_.at(s));
			project_->sequences.move(index, index-delta_);
		}
	}
	else{
		for (int s=sortseqs_.size()-1;s>=0;s--){
			int index = project_->sequences.index(sortseqs_.at(s));
			project_->sequences.move(index, index-delta_);
		}
	}
}

int MoveCmd::id() const
{
	return 2;
}

bool MoveCmd::mergeWith(const QUndoCommand *other)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	if (other->id() != id()) 
		return false;
	const MoveCmd *cmd = static_cast<const MoveCmd *>(other);
	// do domething
	qDebug() << trace.header(__PRETTY_FUNCTION__) << cmd->delta_;
	// Need to check that the sequences are the same
	if (sortseqs_.size() != cmd->sortseqs_.size()) return false;
	for (int s=0;s<sortseqs_.size();s++){
		if (sortseqs_.at(s) != cmd->sortseqs_.at(s)){
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "can't merge";
			return false;
		}
	}
	delta_ += cmd->delta_;
	return true;
}

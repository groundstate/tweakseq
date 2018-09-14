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

#include "Project.h"
#include "Sequence.h"
#include "Sequences.h"
#include "TrimCmd.h"

TrimCmd::TrimCmd(Project *project,const QString &txt):Command(project,txt)
{
	
}

TrimCmd::~TrimCmd()
{
}

void TrimCmd::redo()
{
	deletedCols_.clear();
	int rMax = project_->sequences.minLength(true);
	QList<Sequence *> & seqs = project_->sequences.sequences();
	// Work in reverse so that indexing is fixed
	for (int r=rMax-1;r>=0;r--){
		bool cut=true;
		for (int s=0;s<seqs.count();s++){
			Sequence *seq = seqs.at(s);
			if (r >= seq->residues.length()) // FIXME is that what we want ?
				break;
			QChar qch=seq->residues[r];
			if ((qch.unicode() & REMOVE_FLAGS) !='-'){
				cut=false;
				break;
			}
		}
		if (cut){
			deletedCols_.append(r);
			for (int s=0;s<seqs.count();s++){
				Sequence *seq = seqs.at(s);
				if (r < seq->residues.length()){ // FIXME is that what we want ?
					seq->residues.remove(r,1);
				}
			}
		}
	}
}

void TrimCmd::undo()
{
	QList<Sequence *> & seqs = project_->sequences.sequences();
	for (int r=deletedCols_.size()-1;r>=0;r--){
		for (int s=0;s<seqs.count();s++){
			if (r < seqs.at(s)->residues.length()) // FIXME is that what we want ?
				project_->sequences.addInsertions(seqs.at(s),deletedCols_.at(r),1);
		}
	}
	
}
		

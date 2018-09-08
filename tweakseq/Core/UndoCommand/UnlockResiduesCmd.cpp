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

#include "UnlockResiduesCmd.h"
#include "Project.h"
#include "ResidueLockGroup.h"
#include "Sequence.h"

UnlockResiduesCmd::UnlockResiduesCmd(Project *project,QList<ResidueGroup *> &residueSel,const QString &txt):Command(project,txt)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	sel_=residueSel;
}

UnlockResiduesCmd::~UnlockResiduesCmd()
{
}

void UnlockResiduesCmd::redo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	seqs_.clear();
	groups_.clear();
	emptyGroups_.clear();
	
	for (int r=0;r<sel_.size();r++){
		Sequence *seq = sel_.at(r)->sequence;
		if (NULL != seq->residueLockGroup){
			ResidueLockGroup *rlg = seq->residueLockGroup;
			if (sel_.at(r)->start <= rlg->position() && rlg->position() <= sel_.at(r)->stop){
				rlg->removeSequence(seq); // sets the pointer to the lock group to NULL
				seqs_.append(seq); // save the sequence and group for Undo
				groups_.append(rlg);
				if (rlg->empty()){
					project_->residueLockGroups.removeOne(rlg);
					emptyGroups_.append(rlg);
				}
			}
		}
	} // for
}

void UnlockResiduesCmd::undo()
{
	
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	
	// restore groups removed because they were empty
	for (int g=0;g<emptyGroups_.size();g++)
		project_->residueLockGroups.append(emptyGroups_.at(g));
	
	// restore sequences to their group
	for (int s=0;s<seqs_.size();s++)
		groups_.at(s)->addSequence(seqs_.at(s));
	
}
		 
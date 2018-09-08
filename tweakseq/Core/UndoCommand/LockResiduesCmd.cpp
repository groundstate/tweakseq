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

#include "LockResiduesCmd.h"
#include "Project.h"
#include "ResidueLockGroup.h"
#include "Sequence.h"

LockResiduesCmd::LockResiduesCmd(Project *project,QList<ResidueGroup *> &residueSel,const QString &txt):Command(project,txt)
{
	
	sel_=residueSel;
	//for (int r=0;r<rg.size();r++)
	//	sel_.append(rg.at(r));
	qDebug() << trace.header(__PRETTY_FUNCTION__) << sel_.size();
	rlg_ = new ResidueLockGroup(); // create once
	
}

LockResiduesCmd::~LockResiduesCmd()
{
}

void LockResiduesCmd::redo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << sel_.size();
	
	for (int r=0;r<sel_.size();r++){
		rlg_->addSequence(sel_.at(r)->sequence);
		//sel_.at(r)->sequence->residueLockGroup=rlg_;
	}
	rlg_->setPosition(sel_.at(0)->start);
	project_->residueLockGroups.append(rlg_);

}

void LockResiduesCmd::undo()
{

	for (int s=0;s<rlg_->sequences.size();s++)
		rlg_->sequences.at(s)->residueLockGroup=NULL;
	project_->residueLockGroups.removeOne(rlg_);

}
		 
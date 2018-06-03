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

#include "Project.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "UndoAlignment.h"

//
// Public members
//

		
UndoAlignment::UndoAlignment(Project *p,const QList<Sequence *> &preAlign,const QList<SequenceGroup *> &groupsPreAlign,
																					 const QList<Sequence *> &postAlign,const QList<SequenceGroup *> &groupsPostAlign,
																					 const QString &txt):UndoCommand(p,txt)
{
	
	for (int g=0;g<groupsPreAlign.size();g++){
		SequenceGroup *sg = new SequenceGroup();
		*sg = *(groupsPreAlign.at(g));
		sg->clear();
		groupsPreAlign_.append(sg);
	}
	
	for (int g=0;g<groupsPostAlign.size();g++){
		SequenceGroup *sg = new SequenceGroup();
		*sg = *(groupsPostAlign.at(g));
		sg->clear();
		groupsPostAlign_.append(sg);
	}
	
	for (int s=0;s<preAlign.size();s++){
		Sequence *seq = new Sequence();
		*seq = *(preAlign.at(s));
		seqPreAlign_.append(seq);
		if (preAlign.at(s)->group){
			int gi = groupIndex(preAlign.at(s)->group,groupsPreAlign);
			groupsPreAlign_.at(gi)->addSequence(seq);
		}
	}
	
	for (int s=0;s<postAlign.size();s++){
		Sequence *seq = new Sequence();
		*seq = *(postAlign.at(s));
		seqPostAlign_.append(seq);
		if (postAlign.at(s)->group){
			int gi = groupIndex(postAlign.at(s)->group,groupsPostAlign);
			groupsPostAlign_.at(gi)->addSequence(seq);
		}
	}
	
}

UndoAlignment::~UndoAlignment()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	while (!seqPreAlign_.isEmpty())
		delete seqPreAlign_.takeFirst();
	while (!groupsPreAlign_.isEmpty())
		delete groupsPreAlign_.takeFirst();
	while (!seqPostAlign_.isEmpty())
		delete seqPostAlign_.takeFirst();
	while (!groupsPostAlign_.isEmpty())
		delete groupsPostAlign_.takeFirst();
}

void UndoAlignment::redo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__)  << seqPreAlign_.size() << " " << groupsPreAlign_.size();
	prj_->setAlignment(seqPostAlign_,groupsPostAlign_);
	
}

void UndoAlignment::undo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << seqPreAlign_.size() << " " << groupsPreAlign_.size();
	prj_->setAlignment(seqPreAlign_,groupsPreAlign_);
}

int UndoAlignment::groupIndex(SequenceGroup *sg,const QList<SequenceGroup *> &sgl)
{
	for (int i=0;i<sgl.size();i++){
		if (sgl.at(i) == sg) return i;
	}
	return -1;
}



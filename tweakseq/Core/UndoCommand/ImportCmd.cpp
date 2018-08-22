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
#include "ImportCmd.h"
#include "Project.h"

ImportCmd::ImportCmd(Project *project,const QList<Sequence *> &seqs,const QString &txt):Command(project,txt)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	seqs_=seqs;
	aligned_=project_->aligned();
}

ImportCmd::~ImportCmd()
{
}

void ImportCmd::redo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	project_->enableUIupdates(false);
	project_->sequences.append(seqs_);
	project_->enableUIupdates(true);
	project_->setAligned(false);
}

void ImportCmd::undo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	project_->enableUIupdates(false);
	project_->sequences.remove(seqs_);
	project_->enableUIupdates(true);
	if (aligned_) project_->setAligned(true);
}
		

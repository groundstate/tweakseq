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

#include "Command.h"
#include "ExcludeResiduesCmd.h"
#include "Project.h"
#include "ResidueSelection.h"
#include "Sequence.h"


ExcludeResiduesCmd::ExcludeResiduesCmd(Project *project,
	QList<ResidueGroup *> &residues,bool add,const QString &txt):Command(project,txt)
{
	residues_=residues;
	add_=add;
}

ExcludeResiduesCmd::~ExcludeResiduesCmd()
{
}

void ExcludeResiduesCmd::redo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	for (int rg=0;rg<residues_.size();rg++){
		ResidueGroup *resGroup = residues_.at(rg);
		resGroup->sequence->exclude(resGroup->start,resGroup->stop,add_);
	}
}

void ExcludeResiduesCmd::undo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	for (int rg=0;rg<residues_.size();rg++){
		ResidueGroup *resGroup = residues_.at(rg);
		resGroup->sequence->exclude(resGroup->start,resGroup->stop,!add_);
	}
}
	

		

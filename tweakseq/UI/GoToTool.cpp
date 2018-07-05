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

#include <QBoxLayout>
#include <QComboBox>
#include <QCompleter>
#include <QLabel>
#include <QStringListModel>

#include "GoToTool.h"
#include "Project.h"
#include "Sequence.h"
#include "Sequences.h"

//
// Public members
//

GoToTool::GoToTool(QWidget *parent):QWidget(parent)
{
	
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
	layout->setContentsMargins(0,0,0,0);
	QLabel *label = new QLabel("Go to",this);
	
	layout->addWidget(label);
	cb_=new QComboBox(this);
	
	cb_->setEditable(true);
	cb_->setMinimumWidth(200);
	
	layout->addWidget(cb_);
	
	model_ = new QStringListModel(this);
	completer_ = new QCompleter(model_,this);
	completer_->setCaseSensitivity(Qt::CaseInsensitive);
	cb_->setCompleter(completer_);
	
	connect(cb_,SIGNAL(activated(const QString &)),this,SIGNAL(find(const QString &)));

}

void GoToTool::setCompleterModel(QStringList &sl)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << sl.count();
	model_->setStringList(sl);
}


// void GoToTool::updateCompleterModel()
// {
// 	QList<Sequence *> &sequences = project_->sequences.sequences();
// 	QStringList labels;
// 	for (int s=0;s<sequences.count();s++)
// 		labels.append(sequences.at(s)->label);
// 	qDebug() << trace.header(__PRETTY_FUNCTION__) << labels.count();
// 	model_->setStringList(labels);
// }

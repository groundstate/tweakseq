//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
/// Copyright (c) 2000-2017  Merridee A. Wouters, Michael J. Wouters
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

#include "AlignmentCmd.h"
#include "AlignmentCmdDlg.h"

#include <QDialogButtonBox>
#include <QTextDocument>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QPlainTextEdit>

AlignmentCmdDlg::AlignmentCmdDlg(QString cmd,QString defaultCmd,QWidget * parent):QDialog(parent)
{
	cmd_ = cmd;
	defaultCmd_=defaultCmd;
	
	setModal(true);
	setWindowTitle("Alignment command");
	
	QVBoxLayout *vb = new QVBoxLayout();
	// vb->setContentsMargins(0,0,0,11);
	setLayout(vb);
	
	
	QLabel *l = new QLabel("Command line");
	vb->addWidget(l);
	
	cmdTE_ = new QPlainTextEdit(this);
	cmdTE_->setPlainText(cmd_);
	
	vb->addWidget(cmdTE_);

	buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Reset | QDialogButtonBox::Cancel);
	vb->addWidget(buttonBox_);
	
	QPushButton *pb = buttonBox_->button(QDialogButtonBox::Reset);
	connect(pb,SIGNAL(clicked()),this,SLOT(setToOriginal()));
	
	pb = new QPushButton(tr("Defaults"));
	buttonBox_->addButton(pb,QDialogButtonBox::ActionRole);
	
	connect(pb,SIGNAL(clicked()),this,SLOT(setToDefaults()));
	
  connect(buttonBox_, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox_, SIGNAL(rejected()), this, SLOT(reject()));
}

AlignmentCmdDlg::~AlignmentCmdDlg()
{
}
		
QString AlignmentCmdDlg::command()
{
	return cmdTE_->toPlainText();
	
}

void AlignmentCmdDlg::setToDefaults()
{
	cmdTE_->setPlainText(defaultCmd_);
}

void AlignmentCmdDlg::setToOriginal()
{
	cmdTE_->setPlainText(cmd_);
}


		
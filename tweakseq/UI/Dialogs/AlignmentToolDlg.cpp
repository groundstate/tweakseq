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

#include "AlignmentTool.h"
#include "AlignmentToolDlg.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>

AlignmentToolDlg::AlignmentToolDlg(AlignmentTool *alignmentTool,QWidget * parent, Qt::WindowFlags f ):QDialog(parent,f)
{
	setWindowTitle("Alignment tool configuration");
	QVBoxLayout *vb = new QVBoxLayout();
	// vb->setContentsMargins(0,0,0,11);
	setLayout(vb);
	
	QHBoxLayout *hb = new QHBoxLayout();
	vb->addLayout(hb);
	
	QLabel *l = new QLabel("Executable");
	hb->addWidget(l);
	
	executableTE_ = new QLineEdit(this);
	hb->addWidget(executableTE_);
	executableTE_->setText(alignmentTool->executable());
	
	QPushButton *button = new QPushButton("Browse ..",this);
	hb->addWidget(button);
	connect(button,SIGNAL(clicked()),this,SLOT(browse()));
	
	buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vb->addWidget(buttonBox_);
	
  connect(buttonBox_, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox_, SIGNAL(rejected()), this, SLOT(reject()));
}

AlignmentToolDlg::~AlignmentToolDlg()
{
}
		


void AlignmentToolDlg::browse()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select executable"),"/usr");
	if (!fileName.isNull())
		executableTE_->setText(fileName);
}

		
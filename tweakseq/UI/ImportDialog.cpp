//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2018 Merridee A. Wouters, Michael J. Wouters
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

#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include "ImportDialog.h"
#include "SequenceFile.h"

ImportDialog::ImportDialog(int defaultDataType,QWidget *parent,Qt::WindowFlags f):QDialog(parent,f)
{
	QVBoxLayout *vb = new QVBoxLayout(this);

	QLabel *l = new QLabel("Select the type of data you want to import:",this);
	vb->addWidget(l,0);
	
	// no need for button group - exclusive by default
	proteinButton_= new QRadioButton("proteins",this);
	proteinButton_->setChecked(defaultDataType==SequenceFile::Proteins);
	vb->addWidget(proteinButton_);
	
	DNAButton_= new QRadioButton("DNA",this);
	DNAButton_->setChecked(defaultDataType==SequenceFile::DNA);
	vb->addWidget(DNAButton_);
	
	QHBoxLayout * hb = new QHBoxLayout();
	vb->addLayout(hb);
	
	QPushButton *okb = new QPushButton("OK",this);
	connect(okb,SIGNAL(clicked()),this,SLOT(accept()));
	hb->addWidget(okb);
	
	QPushButton *cancelb = new QPushButton("Cancel",this);
	connect(cancelb,SIGNAL(clicked()),this,SLOT(reject()));
	hb->addWidget(cancelb);
}

int ImportDialog::dataType()
{
	if (DNAButton_->isChecked())
		return SequenceFile::DNA;
	return SequenceFile::Proteins;
}
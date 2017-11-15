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


#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>

#include "SetupWizard.h"


//
//	public:
//	

SetupWizard::SetupWizard(QWidget *parent,Qt::WindowFlags flags):QWizard(parent,flags)
{
	addPage(createIntroPage());
	addPage(createAlignmentToolPage());
}

SetupWizard::~SetupWizard()
{
}

void SetupWizard::clustalOConfig(bool &checked,QString &executable)
{
	checked = clustaloCB_->isChecked();
	executable = clustaloLE_->text();
}

void SetupWizard::muscleConfig(bool &checked,QString &executable)
{
	checked = muscleCB_->isChecked();
	executable = muscleLE_->text();
}

QString SetupWizard::preferredTool()
{
	return preferredTool_->currentText();
}
//
// private slots
//
		
void  SetupWizard::browseClustalO()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select executable"));
	if (!fileName.isNull())
		clustaloLE_->setText(fileName);
	
}

void  SetupWizard::browseMUSCLE()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select executable"));
	if (!fileName.isNull())
		muscleLE_->setText(fileName);
}

//
// private
//

QWizardPage * SetupWizard::createIntroPage()
{
	QWizardPage *page = new QWizardPage;
	
	page->setTitle("Introduction");

	QLabel *label = new QLabel("This wizard does basic setup of tweakseq."
			"If you want to run it again, run tweakseq with the --configure option");
	label->setWordWrap(true);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(label);
	page->setLayout(layout);

	return page;
}

QWizardPage * SetupWizard::createAlignmentToolPage()
{
	QWizardPage *page = new QWizardPage;
	
	page->setTitle("Alignment tools");
	
	QLabel *label = new QLabel("Choose the alignment tools you want to configure");
	label->setWordWrap(true);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(label);
	
	QHBoxLayout *hb = new QHBoxLayout;
	layout->addLayout(hb);
	
	label = new QLabel("Preferred tool");
	hb->addWidget(label);
	preferredTool_ = new QComboBox(page);
	layout->addWidget(preferredTool_);
	preferredTool_->addItem("clustalo");
	preferredTool_->addItem("MUSCLE");
	hb->addWidget(preferredTool_);
	
	QGridLayout *gl = new QGridLayout();
	layout->addLayout(gl);
	
	clustaloCB_= new QCheckBox("clustalo",page);
	gl->addWidget(clustaloCB_,1,1);
	
	clustaloLE_ = new QLineEdit(page);
	gl->addWidget(clustaloLE_ ,1,2);
	
	QPushButton *pb = new QPushButton("Browse ...",page);
	gl->addWidget(pb,1,3);
	connect(pb,SIGNAL(clicked()),this,SLOT(browseClustalO()));
	
	//
	
	muscleCB_= new QCheckBox("MUSCLE",page);
	gl->addWidget(muscleCB_,2,1);
	
	muscleLE_ = new QLineEdit(page);
	gl->addWidget(muscleLE_ ,2,2);
	
	pb = new QPushButton("Browse ...",page);
	gl->addWidget(pb,2,3);
	connect(pb,SIGNAL(clicked()),this,SLOT(browseMUSCLE()));
	
	page->setLayout(layout);
	
	return page;
}


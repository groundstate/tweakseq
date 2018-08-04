//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
/// Copyright (c) 2000-2018  Merridee A. Wouters, Michael J. Wouters
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

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "Project.h"
#include "Sequence.h"
#include "SequencePropertiesDialog.h"
#include "Structure.h"

SequencePropertiesDialog::SequencePropertiesDialog(Project *project,Sequence *seq,QWidget * parent, Qt::WindowFlags f ):QDialog(parent,f)
{
	setWindowTitle("Sequence properties");
	setMinimumWidth(800);
	
	seq_=seq;
	project_=project;
	name_=seq->label;
	
	QVBoxLayout *vb = new QVBoxLayout();
	// vb->setContentsMargins(0,0,0,11);
	setLayout(vb);
	
	QGridLayout *gl = new QGridLayout();
	vb->addLayout(gl);
	
	//
	QLabel *l = new QLabel("Name");
	gl->addWidget(l,0,0);
	
	nameEditor_ = new QLineEdit(this);
	nameEditor_->setText(seq->label);
	gl->addWidget(nameEditor_,0,1);
	
	//
	l = new QLabel("Original name");
	gl->addWidget(l,1,0);
	
	QLineEdit *le = new QLineEdit(this);
	le->setText(seq->originalName);
	le->setReadOnly(true);
	gl->addWidget(le,1,1);
	
	//
	l = new QLabel("Comment");
	gl->addWidget(l,2,0);
	
	commentEditor_ = new QTextEdit(this);
	commentEditor_->setPlainText(seq->comment);
	//te->setReadOnly(true);
	gl->addWidget(commentEditor_,2,1);
	
	//
	l = new QLabel("Source");
	gl->addWidget(l,3,0);
	
	le = new QLineEdit(this);
	le->setText(seq->source);
	le->setReadOnly(true);
	gl->addWidget(le,3,1);
	
	l = new QLabel("Structure file");
	gl->addWidget(l,4,0);
	
	structureEditor_= new QLineEdit(this);
	structureEditor_->setText(seq->structureFile);
	gl->addWidget(structureEditor_,4,1);
	
	QPushButton *button = new QPushButton("Browse ..",this);
	gl->addWidget(button,4,3);
	connect(button,SIGNAL(clicked()),this,SLOT(browseStructures()));
	
	if (seq->structure){
		structureEditor_->setReadOnly(true);
		button->setEnabled(false);
		l = new QLabel("Number of chains:"+QString::number(seq->structure->chains.size()));
		gl->addWidget(l,5,0);
		l = new QLabel("Selected chain");
		gl->addWidget(l,6,0);
		selChain_ = new QComboBox(this);
		gl->addWidget(selChain_,6,1);
		for (int i=0;i<seq->structure->chains.size();i++)
			selChain_->addItem(seq->structure->chainIDs.at(i));
		selChain_->setCurrentIndex(seq->structure->selectedChain);
	}
	buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vb->addWidget(buttonBox_);
	
  connect(buttonBox_, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox_, SIGNAL(rejected()), this, SLOT(reject()));
}

SequencePropertiesDialog::~SequencePropertiesDialog()
{
}
		
QString SequencePropertiesDialog::structureFile()
{
	return structureEditor_->text().trimmed();
}

QString SequencePropertiesDialog::comment()
{
	return commentEditor_->toPlainText();
}

int  SequencePropertiesDialog::selectedChain()
{
	return selChain_->currentIndex();
}

		
void SequencePropertiesDialog::accept()
{
	QString newName = nameEditor_->text().trimmed();
	if (name_ != newName){
		if (!project_->renameSequence(seq_,newName)){
			QMessageBox::critical(this, tr("Rename failed"),"The name \"" + newName + "\" is already used in this project");
			nameEditor_->selectAll();
			return;
		}
	}
	QDialog::accept();
}

void SequencePropertiesDialog::browseStructures()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select structure file"),"./","PDB files (*.ent *.ent.gz)");
	if (!fileName.isNull())
		structureEditor_->setText(fileName);
}

		
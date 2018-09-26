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

#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QFileDialog>
#include <QFileInfo>

#include "FilePropertyInput.h"
#include "FileProperty.h"

//
// Public members
//

FilePropertyInput::FilePropertyInput(QWidget *parent,
	FileProperty * filevar,QString label,
	int inputKind,
	const char *dirName,const char *filter):
	PropertyInput(parent,label)
{
	filevar->saveValue();
	filevar_=filevar; 
	inputKind_=inputKind;
	if (filter)
		filter_=filter;
	else
		filter_="*";
	if (dirName)	
		dirName_=dirName;
	else
		dirName_="";
		
	QHBoxLayout *hb = new QHBoxLayout(this);
	hb->setContentsMargins(0,0,0,0);
	hb->setSpacing(8);
	QLabel *l = new QLabel(label,this);
	hb->addWidget(l,0);
	
	le_= new QLineEdit(filevar_->fileName(),this);
	hb->addWidget(le_,1);
	
	QPushButton *b = new QPushButton("Browse ...",this);
	hb->addWidget(b,0);
	
	connect(b,SIGNAL(clicked()),this,SLOT(browse()) );
	
}

FilePropertyInput::~FilePropertyInput()
{
}

void FilePropertyInput::addFilter(const char *)
{
}

void FilePropertyInput::setValue(const char *f)
{
	le_->setText(f);
}

const char * FilePropertyInput::value()
{
	return le_->text().toStdString().c_str();
}

//
// Public slots
//

void FilePropertyInput::set()
{
	filevar_->setFileName(le_->text().trimmed()); // removes whoopsies like an accidentally typed space(s)
}

void FilePropertyInput::restore()
{
	filevar_->restoreSavedValue();
}

//
// Private slots
//

void FilePropertyInput::browse()
{
	QString fname;
	// Use the path the file to guess a sensible starting directory.
	// The current path, as determined by the last file that was opened
	// will be used otherwise.
	QFileInfo fi(filevar_->fileName());
	
	QString fp = QString::null; // this will give the current working directory
	
	// If a directory path has been specified then use this initially
	if (dirName_.length())
		fp=dirName_;
	
	if (filevar_->fileName().length())
		fp= fi.absoluteFilePath();
	
	//if () fp=null; // 
	switch (inputKind_)
	{
		case SaveFile:
			fname=
				QFileDialog::getSaveFileName(this,"Save file",fp,filter_);
			break;
		case OpenFile:
			fname=
				QFileDialog::getOpenFileName(this,"Open file",fp,filter_);
			break;
		case Directory:
			fname=
				QFileDialog::getExistingDirectory(this,"Choose directory",QString::null);
			break;
	}
	
	
	if (!fname.isNull())
	{
		le_->setText(fname);
		// if we set filevar here then eg a redraw will cause object model
		// to be loaded because gl() will load the model
	}
}	

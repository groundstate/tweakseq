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


#include <QDialogButtonBox>
#include <QLayout>
#include <QScrollArea>

#include "SeqPreview.h"
#include "SeqPreviewDlg.h"

SeqPreviewDlg::SeqPreviewDlg( QWidget * parent, Qt::WindowFlags f):QDialog(parent,f)
{
	setWindowTitle("Alignment preview");
	QVBoxLayout *vb = new QVBoxLayout();
	vb->setContentsMargins(0,0,0,11);
	setLayout(vb);
	scrollArea_ = new QScrollArea(this);
	scrollArea_->setBackgroundRole(QPalette::Dark);
	vb->addWidget(scrollArea_);
	
	preview_ = new SeqPreview();
	scrollArea_->setWidget(preview_);
	
	buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vb->addWidget(buttonBox_);
	
  connect(buttonBox_, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox_, SIGNAL(rejected()), this, SLOT(reject()));
		 
}

SeqPreviewDlg::~SeqPreviewDlg()
{
}


void SeqPreviewDlg::setSequences(QStringList &labels,QStringList &residues)
{
	preview_->setSequences(labels,residues);
}

void SeqPreviewDlg::setPreviewFont(const QFont &f)
{
	preview_->setPreviewFont(f);
}

void SeqPreviewDlg::setWidth(int w)
{
	QRect r = geometry();
	r.setWidth(w);
	setGeometry(r);
}
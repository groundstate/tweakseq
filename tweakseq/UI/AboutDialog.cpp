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


#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QVBoxLayout>

#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent,Qt::WindowFlags f):QDialog(parent,f)
{
	setMinimumWidth(480);

	QVBoxLayout *vb = new QVBoxLayout(this);

	QLabel *l = new QLabel("tweakseq",this);
	vb->addWidget(l,0);

	QTabWidget  *tw = new QTabWidget(this);
	vb->addWidget(tw,1);
	
	// about
	QWidget *w = new QWidget();
	QVBoxLayout *wvb = new QVBoxLayout(w);
	tw->addTab(w,"&About");
	QTextBrowser *tb = new QTextBrowser(w);
	wvb->addWidget(tb);
	tb->setReadOnly(true);
	tb->setOpenExternalLinks(true);
	tb->setSource(QUrl("qrc:/text/aboutdialog/about.html"));

	// authors
	w = new QWidget();
	wvb = new QVBoxLayout(w);
	tw->addTab(w,"&Authors");
	tb = new QTextBrowser(w);
	wvb->addWidget(tb);
	tb->setReadOnly(true);
	tb->setOpenExternalLinks(true);
	tb->setSource(QUrl("qrc:/text/aboutdialog/authors.html"));

	// thanks to
	w = new QWidget();
	tw->addTab(w,"&Thanks to");

	// copyrights
	w = new QWidget();
	wvb = new QVBoxLayout(w);
	tw->addTab(w,"&Copyrights");
	tb = new QTextBrowser(w);
	wvb->addWidget(tb);
	tb->setReadOnly(true);
	tb->setOpenExternalLinks(true);
	tb->setSource(QUrl("qrc:/text/aboutdialog/copyrights.html"));

	// license
	w = new QWidget();
	wvb = new QVBoxLayout(w);
	tw->addTab(w,"&License agreement");
	tb = new QTextBrowser(w);
	wvb->addWidget(tb);
	tb->setReadOnly(true);
	tb->setLineWrapMode(QTextEdit::NoWrap);
	QFile tf(":/text/aboutdialog/license");
	tf.open(QIODevice::ReadOnly);
	QByteArray ba = tf.readAll();
	tf.close();
	tb->setText(QString(ba));
	//tb->setSource(QUrl("qrc:/text/aboutdialog/GPL_V2"));

	QPushButton *b = new QPushButton("Close",this);
	vb->addWidget(b,0);
	connect(b,SIGNAL(clicked()),this,SLOT(accept()));

}

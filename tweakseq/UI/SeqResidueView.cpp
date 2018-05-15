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

#include <QtDebug>
#include "DebuggingInfo.h"

#include <QFont>
#include <QPainter>
#include <QPaintEvent>

#include "SeqResidueView.h"

//
// Public members
//

SeqResidueView::SeqResidueView(Project *project,QWidget *parent): QWidget(parent)
{
	setContentsMargins(0,0,0,0);
	
	init();
	project_=project;
}

void SeqResidueView::setProject(Project *project)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	init();
	project_=project;
}

void SeqResidueView::setReadOnly(bool readOnly)
{
	readOnly_=readOnly;
}

void SeqResidueView::setViewFont(const QFont &f)
{
	setFont(f);
	repaint();
}

//
// Public slots
//

void SeqResidueView::sequencesCleared()
{

}

//
// Protected members
//

void SeqResidueView::paintEvent(QPaintEvent *pev)
{
	QPainter p(this);
	p.fillRect(pev->rect(),QColor(0,0,0));
}


//
// Private members
//

void SeqResidueView::init()
{
	project_=NULL;
	readOnly_=true;
}
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

#include "Project.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SeqInfoView.h"

#define FLAGS_WIDTH 4
#define LABEL_WIDTH 16

//
// Public members
//

SeqInfoView::SeqInfoView(Project *project,QWidget *parent): QWidget(parent)
{
	setContentsMargins(0,0,0,0);
	
	init();
	project_=project;
	
	setMinimumSize(100,400);
}

void SeqInfoView::setProject(Project *project)
{
	init();
	project_=project;
}

void SeqInfoView::setNumRows(int num)
{
	numRows_=num;
}

void SeqInfoView::setRowPadding(double p)
{
	rowPadding_=p;
}

void SeqInfoView::setViewFont(const QFont &f)
{
	setFont(f); // presumption is that the requested font is available
	QFontMetrics fm(f);
	int h = fm.width('W'); // a wide character
	int w = h;
	rowHeight_= h*rowPadding_;
	flagsWidth_=w*FLAGS_WIDTH;
	labelWidth_=w*LABEL_WIDTH;
	
	setFixedWidth(flagsWidth_ + labelWidth_);
	repaint();
}

void SeqInfoView::updateViewport()
{
	qDebug() << numRows_;
	setFixedHeight(rowHeight_*numRows_);
}

//
// Public slots
//

void SeqInfoView::sequencesCleared()
{
	numRows_=0;
}

//
// Protected members
//

void SeqInfoView::paintEvent(QPaintEvent *pev)
{
	QPainter p(this);

	p.fillRect(pev->rect(),QColor(0,0,0));
	
	for (int r=0;r<numRows_;r++){
		paintRow(&p,r);
	}
}

//
// Private members
//

void SeqInfoView::init()
{
	project_=NULL;
	numRows_=0;
	rowPadding_=1.3;
	flagsWidth_=0;
	labelWidth_=0;
}

void SeqInfoView::paintRow(QPainter *p,int row)
{
	QColor txtColor;
	
	Sequence *currSeq = project_->sequences.visibleAt(row);
	
	if (currSeq->group != NULL){
		if (currSeq->group->locked()){
			txtColor.setRgb(255,0,0);
			p->setPen(txtColor);
			p->drawText( 0, rowHeight_*row, flagsWidth_, rowHeight_, Qt::AlignLeft, "L");
		}
		txtColor=currSeq->group->textColour(); // group colour for sequence label
	}
	else{
		txtColor.setRgb(255,255,255); //default colour 
	}
	
	p->setPen(txtColor);
	p->drawText( flagsWidth_, rowHeight_*row, labelWidth_,rowHeight_,Qt::AlignLeft, currSeq->label);
	
}

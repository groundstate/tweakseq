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
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>

#include "Project.h"
#include "ResidueSelection.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SeqInfoView.h"
#include "SequenceSelection.h"
#include "Utility.h"

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
	setMouseTracking(true);
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
	columnWidth_=w*columnPadding_;
	
	flagsWidth_=columnWidth_*FLAGS_WIDTH;
	labelWidth_=w*LABEL_WIDTH;
	
	setFixedWidth(flagsWidth_ + labelWidth_);
	setFixedHeight(numRows_*rowHeight_);
	repaint();
}

void SeqInfoView::setReadOnly(bool readOnly)
{
	readOnly_=readOnly;
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

void SeqInfoView::mousePressEvent( QMouseEvent *ev )
{
	
	if (readOnly_) return;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (seq.count() == 0) return;
	
	QPoint clickedPos;
	int clickedRow,clickedCol;
	int col,row,startRow,stopRow,startCol,stopCol;
	
	clickedPos = ev->pos();		
	clickedRow=rowAt( clickedPos.y() + contentsRect().y());
	clickedCol=columnAt(clickedPos.x() + contentsRect().x());
	
	// If we clicked outside the editing area ... well ... do nothing
	if ((clickedRow < 0) || (clickedRow > seq.count() -1) || clickedCol < 0){
		switch (ev->button()){
			case Qt::LeftButton:
				break;
			default:break;
		} // end switch (e->button
		return;
	}
	qDebug() << clickedRow << " " << clickedCol;
	// For the moment we do nothing with the flags area
	if (clickedCol < FLAGS_WIDTH) return;
	
	// otherwise ...
	
	Sequence *selSeq =  project_->sequences.visibleAt(clickedRow);
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "selected " << selSeq->label;
	
	// Remove any residue selection
	selAnchorRow_=selAnchorCol_=selDragRow_=selDragCol_=-1;
	project_->residueSelection->clear(); // FIXME better done with a signal ?
		
	selectingSequences_=true;
	
	switch (ev->button()){
		case Qt::LeftButton:
		{
			switch (ev->modifiers()){
				case Qt::NoModifier:
					project_->sequenceSelection->clear();
					seqSelectionAnchor_=seqSelectionDrag_=clickedRow;
					break;
				case Qt::ShiftModifier:
					if (project_->sequenceSelection->contains(selSeq))
						project_->sequenceSelection->toggle(selSeq);
					else{
						project_->sequenceSelection->set(selSeq);
					}
					break;
				case Qt::ControlModifier:
					project_->sequenceSelection->toggle(selSeq);
					break;
				default:
					break;
				}
			leftDown_=true; // not set in mouse move event
			break;
			
		}
		case Qt::RightButton:
		{
				// If a context menu is being requested for a non-selected item
				// make it the new selection
				if (!project_->sequenceSelection->contains(selSeq)){
					project_->sequenceSelection->set(selSeq);
				}
		}
		default:
			break;
	}
	
	this->repaint();
	return;
		
}

void SeqInfoView::mouseReleaseEvent( QMouseEvent *ev )
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	
	if (readOnly_) return;
	
	switch (ev->button()){
		case Qt::LeftButton:
		{
			leftDown_=false; // have finished selection
			if (selectingSequences_){
				selectingSequences_=false;
				int startRow = seqSelectionAnchor_,stopRow=seqSelectionDrag_;
				if (stopRow < startRow) swap_int(&startRow,&stopRow);
				int visStart = startRow,visStop=stopRow;
				startRow = project_->sequences.visibleToActual(startRow);
				stopRow  = project_->sequences.visibleToActual(stopRow);
				
				// Find all groups which are in the raw selection
				QList<SequenceGroup *> groups;
				for (int r=visStart;r<=visStop;r++){
					int rr = project_->sequences.visibleToActual(r);
					SequenceGroup *sg = project_->sequences.sequences().at(rr)->group;
					if (sg){
						if (!(groups.contains(sg)))
							groups.append(sg);
					}
				}
				
				// If all visible members of a group are in the selection, select the whole group
				bool cleared = false;
				for (int g=0;g<groups.size();g++){
					SequenceGroup *sg = groups.at(g);
					int s;
					for (s=0;s<sg->size();s++){
						Sequence *seq = sg->itemAt(s);
						if (seq->visible){
							int index = project_->sequences.visibleIndex(seq);
							qDebug() << trace.header(__PRETTY_FUNCTION__) << "checking group:" << seq->label << " visible at " << index;
							if (index < visStart || index > visStop) break; // not in selection, so bail out
						}
					}
					// At least sequence must have been visible and tested so we can't have got here
					// without one test at least
					if (s==sg->size()){
						qDebug() << trace.header(__PRETTY_FUNCTION__) << "group selected";
						if (!cleared){
							project_->sequenceSelection->clear();
							cleared=true;
						}
						for (s=0;s<sg->size();s++)
							project_->sequenceSelection->add(sg->itemAt(s));
					}
				}
				// Hidden members are part of groups and will have been added to the selection
				// in the previous step.
				// Select only the visible members in this step (duplicates will be skipped by add() )
				if (!cleared){
					project_->sequenceSelection->clear();
					cleared=true;
				}
				for (int r=startRow;r<=stopRow;r++){
					if (project_->sequences.sequences().at(r)->visible)
						project_->sequenceSelection->add(project_->sequences.sequences().at(r));
				}
			}
			break;
		}
		default:break;
	}
}

void SeqInfoView::mouseMoveEvent(QMouseEvent *ev)
{
	QPoint clickedPos;
	int col,row,currRow,currCol;
	int startRow,stopRow,startCol,stopCol;
	int clickedRow;
	
	if (numRows_== 0) return; // so we don't have to guard against null pointers
	
	clickedPos = ev->pos();              		
	clickedRow=rowAt( clickedPos.y() + contentsRect().y());
	
	// clamp to bounds
	if (clickedRow < 0){
		if (clickedPos.y() >=0) // clamp to bottom
			clickedRow = numRows_ -1;
		else // clamp to top
			clickedRow = 0;
	}
	
	if (clickedRow >= numRows_)
		clickedRow = numRows_ -1;
	
	// show the label of the sequence we are moving over
	Sequence *currSeq = project_->sequences.visibleAt(clickedRow);
	if (currSeq->label != lastInfo_){
		lastInfo_ = currSeq->label;
		emit info(lastInfo_);
	}
	
	if (readOnly_) return;
	
	if (leftDown_){ // only scroll if we are selecting (and we can't select if the widget is read-only)
		ensureRowVisible(clickedRow);
	}	
	
	if (selectingSequences_ && leftDown_){
		if (seqSelectionDrag_ != clickedRow){
			currRow = clickedRow;
			startRow =  find_smallest_int(currRow,seqSelectionAnchor_,seqSelectionDrag_);
			stopRow  =  find_largest_int(currRow,seqSelectionAnchor_,seqSelectionDrag_);
			seqSelectionDrag_=currRow;
			// don't need to translate visible to actual here
			//for (int row=startRow;row<=stopRow;row++)
			//	for (int col=FLAGSWIDTH; col < LABELWIDTH+FLAGSWIDTH; col ++)
			//		updateCell(row,col);
			//qDebug() << trace.header(__PRETTY_FUNCTION__) << seqSelectionAnchor_ << " " << seqSelectionDrag_ ;
			//for (int row=startRow;row<=stopRow;row++)
			//	updateRow(row);
			repaint();
		}
	}
	
	
}

void SeqInfoView::mouseDoubleClickEvent(QMouseEvent *ev)
{
	// Double clicking on a group member selects the whole group
	
	if (readOnly_) return;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (seq.count() == 0) return;
	
	QPoint clickedPos;
	int clickedRow,clickedCol;
	
	clickedPos = ev->pos();		
	clickedRow=rowAt( clickedPos.y() + contentsRect().y());
	clickedCol=columnAt( clickedPos.x() + contentsRect().x());

	// If we clicked outside the editing area ... well ... do nothing
	if ((clickedRow < 0) || (clickedRow > project_->sequences.visibleSize() -1) || (clickedCol < 0 )){
		switch (ev->button()){
			case Qt::LeftButton:
				break;
			default:break;
		} // end switch (e->button
		return;
	}

	if (clickedCol >= FLAGS_WIDTH){
		Sequence *selseq = project_->sequences.visibleAt(clickedRow);
		if (selseq->group){
			project_->sequenceSelection->clear();
			project_->addGroupToSelection(selseq->group);
		}
	}
	this->repaint();
}

// Need to know about wheel events so that we can update the global scrollbars
void SeqInfoView::wheelEvent(QWheelEvent *ev)
{
	ev->ignore(); // the event will be handled by the parent QScrollArea
	emit wheelScrolled();
}
		
void SeqInfoView::keyPressEvent( QKeyEvent* )
{
}

		
//
// Private members
//

void SeqInfoView::init()
{
	project_=NULL;
	
	readOnly_=false;
	
	numRows_=0;
	
	rowPadding_=1.3;
	rowHeight_=16;
	
	columnPadding_=1.3;
	columnWidth_=16;
	
	flagsWidth_=0;
	labelWidth_=0;
	
	selectingSequences_=false;
	selAnchorRow_=selAnchorCol_=selDragRow_=selDragCol_=-1; 
	seqSelectionAnchor_=seqSelectionDrag_=-1;
	leftDown_=false;
	
	lastInfo_="";
}

void SeqInfoView::paintRow(QPainter *p,int row)
{
	QColor txtColor;
	
	Sequence *currSeq = project_->sequences.visibleAt(row);
	
	if (project_->sequenceSelection->contains(currSeq)) // highlight if selectde
		p->fillRect(flagsWidth_, rowHeight_*row, labelWidth_,rowHeight_,QColor(128,128,128)); // add one to fill properly
	
	// The selection isn't filled until the mouse is released
	// so a possible selection event has to be handled on the fly
	if (selectingSequences_ && leftDown_){
		int startRow = seqSelectionAnchor_;
		int stopRow  = seqSelectionDrag_;
		if (startRow > stopRow) swap_int(&startRow,&stopRow);
		if (row >= startRow && row <=stopRow)
			p->fillRect(flagsWidth_, rowHeight_*row, labelWidth_,rowHeight_,QColor(128,128,128));
	}
		
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
	
	txtColor.setRgb(255,255,255);
	p->setPen(txtColor);
	p->drawText( columnWidth_*1, rowHeight_*row,columnWidth_*3,rowHeight_, Qt::AlignLeft, QString::number(row));
	
}

int SeqInfoView::rowAt(int ypos)
{
	return (int) (ypos/rowHeight_);
}

int SeqInfoView::columnAt(int xpos)
{
	// The return result only makes sense for the FLAGS area
	return (int) (xpos/columnWidth_);
}

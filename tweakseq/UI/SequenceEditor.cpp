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

#include <cmath>

#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTime>

#include "Project.h"
#include "ResidueSelection.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SequenceEditor.h"
#include "SequenceSelection.h"
#include "Utility.h"

#define FLAGS_WIDTH 4
#define LABEL_WIDTH 16

#define N_GROUP_COLOURS 10

// Colours chose for maximum  contrast for Kenneth Kelly's sequence
static int groupColours[N_GROUP_COLOURS][3]
{
	{241,191,21}, // 82,  yellow
	{247,118,11}, // 48,  orange
	{153,198,249}, // 180, light blue
	{200,177,139}, // 90,  buff
	{35,234,165}, // 139, green
	{244,131,205}, // 247, purplish pink
	{245,144,128}, // 26,  yellowish pink
	{255,190,80}, // 67,  orange yellow (should be 66
	{235,221,33}, // 97   greenish yellow
	{167,220,38} // 115  yellow green
};

//
// Public members
//

SequenceEditor::SequenceEditor(Project *project,QWidget *parent): QWidget(parent)
{
	setContentsMargins(0,0,0,0);
	setFocusPolicy(Qt::StrongFocus);
	
	init();
	project_=project;
	
	setMouseTracking(true);
	
	setMinimumSize(400,600);
	
	connectToProject();
}

void SequenceEditor::setProject(Project *project)
{
	disconnectFromProject();
	
	init();
	project_=project;
	
	connectToProject();
	
}

void SequenceEditor::setEditorFont(const QFont &f)
{
	setFont(f); // presumption is that the requested font is available
	QFontMetrics fm(f);
	int h = fm.width('W'); // a wide character
	int w = h;
	rowHeight_= (int) h*rowPadding_;
	colWidth_= (int) w*columnPadding_;
	
	flagsWidth_=colWidth_*FLAGS_WIDTH;
	labelWidth_=w*LABEL_WIDTH;
	
	updateViewExtents();
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	
	repaint();
}

void SequenceEditor::setReadOnly(bool readOnly)
{
	readOnly_=readOnly;
}



QColor SequenceEditor::getSequenceGroupColour()
{
	currGroupColour_++;
	if (currGroupColour_ == N_GROUP_COLOURS+1)
		currGroupColour_=1;
	return QColor(groupColours[currGroupColour_-1][0],groupColours[currGroupColour_-1][1],groupColours[currGroupColour_-1][2]);
}

void SequenceEditor::updateViewport()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	updateViewExtents();
	repaint();
}

void SequenceEditor::cutSelectedResidues()
{
}

void SequenceEditor::cutSelectedSequences()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	project_->cutSelectedSequences();
	updateViewport();
}

void SequenceEditor::excludeSelection()
{
}

void SequenceEditor::removeExcludeSelection()
{
}

void SequenceEditor::visibleRows(int *start,int *stop)
{
	*start = firstVisibleRow_;
	*stop  = lastVisibleRow_;
	qDebug() << *start << " " << *stop;
}

void SequenceEditor::selectSequence(const QString &label)
{
	
	Sequence *seq = project_->sequences.getSequence(label);
	if (seq){
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "found " <<seq->label;
		project_->sequenceSelection->set(seq);
		// now make sure it is visible
		if (!seq->visible){
			seq->visible = true;
			updateViewExtents();
		}
		int index = project_->sequences.visibleIndex(seq);
		if (index < firstVisibleRow_){
			setFirstVisibleRow(index);
		}
		else if (index > lastVisibleRow_){
		  setFirstVisibleRow(index-(lastVisibleRow_-firstVisibleRow_));
		}
		emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
		repaint();
	}
}

//
// Public slots
//

void SequenceEditor::sequenceAdded(Sequence *s)
{
	
	if (s->visible){
		numRows_=numRows_+1;
		numCols_=project_->sequences.maxLength();
	}
	
	if (!loadingSequences_){ // suppress viewport updates until loading is finished
		emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	}
}

void SequenceEditor::sequencesCleared()
{
	numRows_=0;
	numCols_=0;
}

void SequenceEditor::postLoadTidy()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	// Determine the last group colour used so that currGroupColour_ can be set correctly
	int maxCol =0;
	for (int s=0;s<project_->sequences.size();s++){ // checked OK
		Sequence *seq = project_->sequences.sequences().at(s);
		if (seq->group != NULL){
			QColor gcol = seq->group->textColour();
			
			for (int c=0;c<N_GROUP_COLOURS;c++){
				if (gcol.red() == groupColours[c][0] && gcol.green() == groupColours[c][1] && gcol.blue() == groupColours[c][2]){
					if (c>maxCol){
						maxCol=c;
						break;
					}
				}
			}
		}
	}
	currGroupColour_=maxCol+1;
}

void SequenceEditor::loadingSequences(bool loading)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << loading;
	loadingSequences_= loading;
	if (!loadingSequences_){
		updateViewport();
		emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	}
}

// This is connected to the vertical scrollbar
void SequenceEditor::setFirstVisibleRow(int val)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << val << endl;
	if (val <0) val=0; // the slider can return -1
	firstVisibleRow_=val;
	updateViewExtents();
	repaint();
}

// This is connected to the horizontal scrollbar
void SequenceEditor::setFirstVisibleColumn(int val)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << val << endl;
	if (val <0) val=0; // the slider can return -1
	firstVisibleCol_=val;
	updateViewExtents();
	repaint();
}

		
//
// Protected members
//

void SequenceEditor::resizeEvent(QResizeEvent *)
{
	updateViewExtents();
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_); // FIXME
}

void SequenceEditor::paintEvent(QPaintEvent *pev)
{
	QPainter p(this);

	QTime t;
	t.start();
	p.fillRect(pev->rect(),QColor(0,0,0));
	
	for (int r=firstVisibleRow_;r<=lastVisibleRow_;r++){
		paintRow(&p,r);
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << t.elapsed() << "ms";
}

void SequenceEditor::mousePressEvent( QMouseEvent *ev )
{
	
	totalWheelRotation_ = 0;
	cleanupTimer();
	
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
	if ((clickedRow < 0) || (clickedRow > lastVisibleRow_) || clickedCol > lastVisibleCol_){
		switch (ev->button()){
			case Qt::LeftButton:
				break;
			default:break;
		} // end switch (e->button
		return;
	}
	qDebug() << clickedRow << " " << clickedCol;
	
	// For the moment we do nothing with the flags area
	if (clickedPos.x() < flagsWidth_) return;
	
	// otherwise ...
	
	if (clickedPos.x() < flagsWidth_+labelWidth_){
		Sequence *selSeq =  project_->sequences.visibleAt(clickedRow);
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "selected " << selSeq->label;

		// Remove any residue selection
		selAnchorRow_=selAnchorCol_=selDragRow_=selDragCol_=-1;
		project_->residueSelection->clear(); // FIXME better done with a signal ?
		selectingResidues_=false;	
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
						selectingSequences_=false;
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

		repaint();
		return;
	}
	
	// The user has clicked inside the sequence area so start selecting residues
	project_->sequenceSelection->clear(); // clear the current selection
	
	switch (ev->button()){
		case Qt::LeftButton:
			leftDown_=true;
			if (selectingResidues_){
				// If we have already selected a block of cells
				// then we need to update these viz remove the highlight mark
				selectingResidues_=false; // this will block a redraw of the highlight
				if (selAnchorRow_ > selDragRow_){ 
					startRow=selDragRow_;
					stopRow=selAnchorRow_;
				}
				else{
					startRow=selAnchorRow_;
					stopRow=selDragRow_;
				}
				if (selAnchorCol_ > selDragCol_){ 
					startCol=selDragCol_;
					stopCol=selAnchorCol_;
				}
				else{
					startCol=selAnchorCol_;
					stopCol=selDragCol_;
				}
				//for (row=startRow;row<=stopRow;row++)
					//for (col=startCol;col<=stopCol;col++)
						//updateCell(row,col);
			}
			// A new selection has been made
			selectingResidues_ = true;
			
			selAnchorRow_ = clickedRow;   // map to row; set current cell
			selAnchorCol_ = clickedCol;   // map to col; set current cell
			selDragRow_ = selAnchorRow_;
			selDragCol_ = selAnchorCol_;
			//updateCell(selAnchorRow,selAnchorCol);
			break;
		case Qt::MidButton:
			break;
		case Qt::RightButton:
			break;
		default:break;
	}
	repaint();
		
}

void SequenceEditor::mouseReleaseEvent( QMouseEvent *ev )
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	
	// clean up any events we might be tracking
	totalWheelRotation_ = 0;
	cleanupTimer();
	
	if (readOnly_) return;
	
	switch (ev->button()){
		case Qt::LeftButton:
		{
			leftDown_=false; // have finished selection
			if (selectingResidues_){
				//selectingResidues_=false; // somewhat illogically, don't drop the mode because painting depends on it still being true
				qDebug() << trace.header(__PRETTY_FUNCTION__) << selAnchorRow_ << " " << selAnchorCol_ << " " << selDragRow_ << " " << selDragCol_;
				int startRow=selAnchorRow_,stopRow=selDragRow_;
				int startCol=selAnchorCol_,stopCol=selDragCol_;
				if (stopRow < startRow) swap_int(&startRow,&stopRow);
				if (stopCol < startCol) swap_int(&startCol,&stopCol);
				qDebug() << trace.header(__PRETTY_FUNCTION__) << startRow << " " << startCol << " " << stopRow << " " << stopCol;
				QList<ResidueGroup *> resSel;
				// startRow and stopRow are for the visible sequences but we need to loop over the hidden ones too
				startRow = project_->sequences.visibleToActual(startRow);
				stopRow  = project_->sequences.visibleToActual(stopRow);
				for (int r=startRow;r<=stopRow;r++)
					resSel.append(new ResidueGroup(project_->sequences.sequences().at(r),startCol,stopCol));
				project_->residueSelection->set(resSel);
				// DO NOT delete the selection
			}
			else if (selectingSequences_){
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

void SequenceEditor::mouseMoveEvent(QMouseEvent *ev)
{

	QPoint pos;
	int col,row,currRow,currCol;
	int startRow,stopRow;
	int startCol,stopCol;
	int clickedRow,clickedCol;

	totalWheelRotation_ = 0;
	
	if (numRows_== 0) return; // so we don't have to guard against null pointers
	
	pos = ev->pos();              		
	clickedRow=rowAt( pos.y() + contentsRect().y());
	clickedCol=columnAt(pos.x() + contentsRect().x());
	
	// clamp to bounds
	if (clickedRow < 0){
		if (pos.y() >=0) // clamp to bottom
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
	
	if (selectingSequences_&& leftDown_){ // only scroll if we are selecting (and we can't select if the widget is read-only)
		// If we go out of the bounds of the window, then the view is scrolled at a rate proportional to the distance
		// we have moved out of the window
		if (pos.y() > height() || pos.y() < 0){
			if (!scrollRowTimer_.isActive()){
				if (pos.y() > height())
					scrollRowIncrement_=1;
				else
					scrollRowIncrement_=-1;
				scrollRowTimer_.start(); 
			}
			else{
				int newTimeout=baseTimeout_;
				if (pos.y() > height())
					newTimeout = baseTimeout_/rint(( pos.y() - height())/5); // scrolling is accelerated proportional to displacement
				else if (pos.y() < 0)
					newTimeout = baseTimeout_/rint( -pos.y()/5);
				if (newTimeout < 100) newTimeout=100;
				if (newTimeout != currentTimeout_){
					currentTimeout_=newTimeout;
					scrollRowTimer_.start(currentTimeout_);
				}
			}
		}
		else{
			if (scrollRowTimer_.isActive()) cleanupTimer();
			if (seqSelectionDrag_ != clickedRow){
				seqSelectionDrag_=clickedRow;
				repaint();
			}
		}
	}	
	
	if (selectingResidues_ && leftDown_){ 
		
		// Must be moving the highlight box around in the sequence field
		//if (clickedCol < LABELWIDTH+FLAGSWIDTH && clickedCol >= FLAGSWIDTH)
		//	clickedCol = LABELWIDTH+FLAGSWIDTH;
		if (clickedCol < 0)
			clickedCol = firstVisibleCol_;
		//	if (clickedPos.x() >= 0)
		//		clickedCol = numCols() -1;
		//	else
		//		clickedCol = LABELWIDTH+FLAGSWIDTH;
		//}
		
		currRow = clickedRow;    // map to row; set current cell
		currCol = clickedCol;    // map to col; set current cell
		// Determine the bounds of the rectangle enclosing both
		// the old highlight box and the new highlight box (so we redraw cells in the old box)
		// Determine the vertical bounds
		startRow = find_smallest_int(currRow,selAnchorRow_,selDragRow_);
		stopRow  = find_largest_int(currRow,selAnchorRow_,selDragRow_);
		// Determine the horizontal bounds
		startCol= find_smallest_int(currCol,selAnchorCol_,selDragCol_);
		stopCol = find_largest_int(currCol,selAnchorCol_,selDragCol_);
		selDragRow_=currRow;
		selDragCol_=currCol;
		
		qDebug() << trace.header(__PRETTY_FUNCTION__) << startRow << " " << stopRow << " " <<
			startCol << " " << stopCol;
			
		repaint();
		// Now update the cells
		//for (row=startRow;row<=stopRow;row++)
		//	for (col=startCol;col<=stopCol;col++)
		//		updateCell(row,col);
	}
	
}

void SequenceEditor::mouseDoubleClickEvent(QMouseEvent *ev)
{
	// Double clicking on a group member selects the whole group
	
	if (readOnly_) return;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (seq.count() == 0) return;
	
	QPoint clickedPos = ev->pos();		
	int clickedRow=rowAt( clickedPos.y() + contentsRect().y());

	if (clickedPos.x() >= flagsWidth_ && clickedPos.x() <= flagsWidth_ + labelWidth_){
		Sequence *selseq = project_->sequences.visibleAt(clickedRow);
		if (selseq->group){
			project_->sequenceSelection->clear();
			project_->addGroupToSelection(selseq->group);
			repaint();
		}
	}
	
}

void SequenceEditor::wheelEvent(QWheelEvent *ev)
{

	int nvis = lastVisibleRow_ - firstVisibleRow_ + 1;
	totalWheelRotation_ += ev->angleDelta().y();
	int nToScroll = totalWheelRotation_/120;
	totalWheelRotation_ -= 120*nToScroll;
	firstVisibleRow_ -= nToScroll;
	if (firstVisibleRow_< 0)
		firstVisibleRow_= 0;
	if (firstVisibleRow_ + nvis -1 >= numRows_)
		firstVisibleRow_ = numRows_- nvis;
	lastVisibleRow_ = firstVisibleRow_ + nvis -1;
	
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	
	repaint();
}
		
void SequenceEditor::keyPressEvent( QKeyEvent* )
{
}

//
// Private members
//

void SequenceEditor::scrollRow()
{
	// Scrolls the view by one row
	qDebug() << trace.header(__PRETTY_FUNCTION__) << scrollRowIncrement_;
	int nvis = lastVisibleRow_ - firstVisibleRow_ + 1;
	
	firstVisibleRow_ += scrollRowIncrement_;
	if (firstVisibleRow_< 0)
		firstVisibleRow_= 0;
	if (firstVisibleRow_ + nvis -1 >= numRows_)
		firstVisibleRow_ = numRows_- nvis;
	lastVisibleRow_ = firstVisibleRow_ + nvis -1;
	
	if (leftDown_ && selectingSequences_){ // only use scrollRow in this context but just in case I forget
		if (scrollRowIncrement_ > 0)
			seqSelectionDrag_ = lastVisibleRow_;
		else
			seqSelectionDrag_ = firstVisibleRow_;
	}
	
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	
	repaint();
}

		
//
// Private members
//

void SequenceEditor::init()
{
	project_=NULL;
	
	readOnly_=false;
	
	numRows_=0;
	numCols_=0;
	
	firstVisibleRow_ = 0;
	lastVisibleRow_=1;
	firstVisibleCol_=0;
	lastVisibleCol_=1;
	
	rowPadding_=1.3;
	rowHeight_=16;
	
	columnPadding_=1.3;
	colWidth_=16;
	
	flagsWidth_=0;
	labelWidth_=0;
	
	loadingSequences_=false;
	selectingSequences_=false;
	selectingResidues_=false;
	selAnchorRow_=selAnchorCol_=selDragRow_=selDragCol_=-1; 
	seqSelectionAnchor_=seqSelectionDrag_=-1;
	leftDown_=false;
	
	lastInfo_="";
	
	currGroupColour_=0;
	
	totalWheelRotation_=0;
	
	baseTimeout_=500;
	currentTimeout_=baseTimeout_;
	scrollRowTimer_.setInterval(baseTimeout_);
	connect(&scrollRowTimer_, SIGNAL(timeout()), this, SLOT(scrollRow()) );
	
}

void SequenceEditor::updateViewExtents()
{
	// round up to catch fractional bits ?
	int displayableRows=ceil((double)height()/((double) rowHeight_));
	lastVisibleRow_ = firstVisibleRow_ + displayableRows-1; // zero indexed, so subtract 1
	
	numRows_ = project_->sequences.numVisible();
	if (lastVisibleRow_ >= numRows_){
		lastVisibleRow_ = numRows_-1; // could be < 0 but next test fixes that
		//firstVisibleRow_ = lastVisibleRow_ - displayableRows + 1;
	}
	if (lastVisibleRow_ < firstVisibleRow_)
		lastVisibleRow_=firstVisibleRow_;
	
	numCols_= project_->sequences.maxLength();
	int displayableCols=ceil((double)(width() - (flagsWidth_+labelWidth_))/((double) colWidth_));
	lastVisibleCol_ = firstVisibleCol_ + displayableCols -1;
	
	if (lastVisibleCol_ >= numCols_)
		lastVisibleCol_ = numCols_-1; 
	if (lastVisibleCol_ < firstVisibleCol_)
		lastVisibleCol_=firstVisibleCol_;
	
	qDebug() << trace.header(__PRETTY_FUNCTION__) << firstVisibleRow_ << " " << lastVisibleRow_ << " " << displayableCols << " " << firstVisibleCol_ << " " << lastVisibleCol_;
}



QChar SequenceEditor::cellContent(int row, int col, int maskFlags )
{
	
	QChar pChar;
	QString s;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	Sequence *currSeq = project_->sequences.visibleAt(row);
	
	// Note Since paintCell() calls this function before any sequences
	// are added have to return valid values when the editor is empty
	
	if (!seq.isEmpty()){
		s=currSeq->residues;
		pChar= s[col];
		if (pChar.unicode() != 0){
			if (col <  s.length())
				return QChar(pChar.unicode() & maskFlags);
			else
				return QChar(0);
		}
		else
			return QChar(0);
	}
	else 
		return QChar(0);

}

void SequenceEditor::paintCell( QPainter* p, int row, int col )
{
	QChar c,cwflags;
	QColor txtColor;
	int cellSelected=false;
	
	c=cellContent(row,col,REMOVE_FLAGS);
	if (c.unicode()==0) return;
	
	Sequence *currSeq = project_->sequences.visibleAt(row);

	int yrow = rowHeight_*(row-firstVisibleRow_);
	int xcol = flagsWidth_ + labelWidth_ + (col-firstVisibleCol_)*colWidth_;
	p->translate(xcol,yrow);
	
	int w = colWidth_;
	int h = rowHeight_;
	
	cwflags=cellContent(row,col,KEEP_FLAGS);
	
	// If the cell is highlighted then do it
	
	if (selectingResidues_ && currSeq->visible){
	txtColor.setRgb(255,255,255);
		if (selAnchorRow_ <= selDragRow_){ // dragging top to bottom
			if (selAnchorCol_ <= selDragCol_){ // left to right
				if (row >= selAnchorRow_ && row <= selDragRow_ && 
					col >= selAnchorCol_ && col<=selDragCol_){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=true;
					}
			}
			else{
				if (row >= selAnchorRow_ && row <= selDragRow_ && 
					col <= selAnchorCol_ && col>=selDragCol_){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=true;
					}
			}
		}
		else{ // dragging bottom to top
			if (selAnchorCol_ <= selDragCol_){ // left to right
				if (row <= selAnchorRow_ && row >= selDragRow_ && 
					col >= selAnchorCol_ && col<=selDragCol_){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=true;
					}
			}
			else{
				if (row <= selAnchorRow_ && row >= selDragRow_ && 
					col <= selAnchorCol_ && col>=selDragCol_){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=true;
					}
			}
		}
	}
	
	//  Draw cell content 
	switch (c.toLatin1()){
		case 'D': case 'E': case 'S': case 'T':// red 
			txtColor.setRgb(255,0,0);
			break; 
		case 'R': case 'K': case 'H': // sky blue
			txtColor.setRgb(135,206,235);
			break;
		case 'Q': case 'N': // purple
			txtColor.setRgb(255,0,255);
			break; 
		case 'M': case 'C': // yellow
			txtColor.setRgb(255,255,0);
			break; 
		case 'A': case 'I': case 'L' : case 'V' : case 'G': case '-': 
		case '!'://white
			if (cellSelected)
				txtColor.setRgb(0,0,0);
			else
				txtColor.setRgb(255,255,255);
			break; 
		case 'Y': case 'F': case 'W': // orange
			txtColor.setRgb(254,172,0);
			break; 
		case 'P': // green
			txtColor.setRgb(0,255,0);
			break;
	};

		// FIXME not so useful if the group is not contiguous
	if (currSeq->group != NULL){
		int start = rowFirstVisibleSequenceInGroup(currSeq->group);
		int stop  = rowLastVisibleSequenceInGroup(currSeq->group);
		if (row==start){
			p->setPen(currSeq->group->textColour());
			p->drawLine(0,2,w-1,2);
		}
		if (row==stop){
			p->setPen(currSeq->group->textColour());
			p->drawLine(0,h-2,w-1,h-2);
		}
	}
	
	if ((!cellSelected) && (cwflags.unicode() & EXCLUDE_CELL) && currSeq->visible ){
		p->setPen(QColor(240,240,16));
		p->drawLine(2,2,w-2,h-2); // X marks the spot ...
		p->drawLine(w-2,2,2,h-2);
	}
	
	if (currSeq->visible){
		p->setPen(txtColor);
		p->drawText( 0, 0, w, h, Qt::AlignCenter, c);
	}

	p->translate(-xcol,-yrow);
	
}

void SequenceEditor::paintRow(QPainter *p,int row)
{
	QColor txtColor;
	
	Sequence *currSeq = project_->sequences.visibleAt(row);
	if (currSeq == NULL) return; // this happens with an empty project
	
	int yrow = rowHeight_*(row-firstVisibleRow_);
	
	if (project_->sequenceSelection->contains(currSeq)) // highlight if selected
		p->fillRect(flagsWidth_, yrow, labelWidth_,rowHeight_,QColor(128,128,128)); // add one to fill properly
	
	// The selection isn't filled until the mouse is released
	// so a possible selection event has to be handled on the fly
	if (selectingSequences_ && leftDown_){
		int startRow = seqSelectionAnchor_;
		int stopRow  = seqSelectionDrag_;
		if (startRow > stopRow) swap_int(&startRow,&stopRow);
		if (row >= startRow && row <=stopRow)
			p->fillRect(flagsWidth_, yrow, labelWidth_,rowHeight_,QColor(128,128,128));
	}
		
	if (currSeq->group != NULL){
		if (currSeq->group->locked()){
			txtColor.setRgb(255,0,0);
			p->setPen(txtColor);
			p->drawText( 0, yrow, flagsWidth_, rowHeight_, Qt::AlignLeft, "L");
		}
		txtColor=currSeq->group->textColour(); // group colour for sequence label
	}
	else{
		txtColor.setRgb(255,255,255); //default colour 
	}
	
	p->setPen(txtColor);
	p->drawText( flagsWidth_, yrow, labelWidth_,rowHeight_,Qt::AlignLeft, currSeq->label);
	
	txtColor.setRgb(255,255,255);
	p->setPen(txtColor);
	p->drawText( colWidth_*1, yrow,colWidth_*3,rowHeight_, Qt::AlignLeft, QString::number(row));
	
	for (int col=firstVisibleCol_;col<=lastVisibleCol_;col++)
		paintCell(p,row,col); // FIXME pass currSeq - maybe optional parameter & test for NULL?
	
}

int SequenceEditor::rowAt(int ypos)
{
	return (int) (ypos/rowHeight_) + firstVisibleRow_;
}

int SequenceEditor::columnAt(int xpos)
{
	// The return result only makes sense for the FLAGS area
	return (int) ((xpos-(flagsWidth_ + labelWidth_))/colWidth_) + firstVisibleCol_;
}

int SequenceEditor::rowFirstVisibleSequenceInGroup(SequenceGroup *sg)
{
	int visIndex=0;
	for (int s=0;s<project_->sequences.size();s++){ // checked OK
		Sequence *seq = project_->sequences.sequences().at(s);
		if (sg==seq->group && seq->visible)
			return visIndex;
		if (seq->visible)
			visIndex++;
	}
	return -1;
}

int SequenceEditor::rowLastVisibleSequenceInGroup(SequenceGroup *sg)
{
	int visIndex=project_->sequences.numVisible()-1;
	for (int s=project_->sequences.size()-1;s>=0;s--){ // checked OK
		Sequence *seq = project_->sequences.sequences().at(s);
		if (sg==seq->group && seq->visible)
			return visIndex;
		if (seq->visible)
			visIndex--;
	}
	return -1;
}

void SequenceEditor::connectToProject()
{
	connect(&(project_->sequences),SIGNAL(sequenceAdded(Sequence *)),this,SLOT(sequenceAdded(Sequence *)));
	connect(&(project_->sequences),SIGNAL(cleared()),this,SLOT(sequencesCleared()));
	connect(project_,SIGNAL(loadingSequences(bool)),this,SLOT(loadingSequences(bool)));
}

void SequenceEditor::disconnectFromProject()
{
	disconnect(&(project_->sequences),SIGNAL(sequenceAdded(Sequence *)),this,SLOT(sequenceAdded(Sequence *)));
	disconnect(&(project_->sequences),SIGNAL(cleared()),this,SLOT(sequencesCleared()));
	disconnect(project_,SIGNAL(loadingSequences(bool)),this,SLOT(loadingSequences(bool)));
}

void SequenceEditor::cleanupTimer()
{
	scrollRowTimer_.stop();
	scrollRowTimer_.setInterval(baseTimeout_);
	currentTimeout_ = baseTimeout_;
}
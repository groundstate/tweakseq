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

// A general purpose sequence editor

// 09052007 MJW BUG FIXED in getSequence() : was removing the last residue from
//					the returned string
// 10052007 MJW BUG FIXED removal of exclusions didn't work ...
// 01062007 MJW BUG FIXED clicking in empty area of the edit caused a segfault
//

// TO DO 
// Improve highlighting so that cells are not unnecessarily repainted
// and flicker is avoided

// Put cursor at position of operation for undo
// Preferences : colours, font size, cell size

// TO DO Sequence locking

// BUG undoLastAlignment() not working ?

#include <QtDebug>
#include "DebuggingInfo.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>


#include <QPainter>
#include <QColor>
#include <QString>
//#include <qdragobject.h>
#include <QMessageBox>
#include <QMouseEvent>

#include "MessageWin.h"
#include "Project.h"
#include "ResidueSelection.h"
#include "SequenceEditor.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SequenceSelection.h"


using namespace std;

#define FLAGSWIDTH 4
#define LABELWIDTH 16
#define NULLCHAR '\0'

#define HPADDING 1.3
#define VPADDING 1.3

#define N_GROUP_COLOURS 10
//
// Main widget for the sequence editor
//

static void swap_int(int *a,int *b);
static int find_smallest_int(int a,int b,int c);
static int find_largest_int(int a,int b,int c);

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

// -----------------------------------------------------------------------------
//
// SequenceEditor - the sequence editing widget
//
// -----------------------------------------------------------------------------

//
// SequenceEditor - public members
//

SequenceEditor::SequenceEditor(Project *project,QWidget *parent)
	:QWidget(parent)
{
	qDebug() << trace.header() << "Creating SequenceEditor";
	init();
	project_=project;
	
	setFocusPolicy( Qt::StrongFocus );              // we accept keyboard focus
	setBackgroundRole( QPalette::Base ); 
	setMouseTracking(true);
	//QPalette pal = palette();
	//pal.setColor(QPalette::Background, Qt::black);
	//setAutoFillBackground(true);
	//setPalette(pal);
	QColor bcolour;
	bcolour.setRgb(0,0,0);
	//setPaletteBackgroundColor(bcolour); // FIXME         
	setNumCols(0);                      
	setNumRows(0);
	
	// Cell width and height based on current font
	
  int w;
	int h;
	// FIXME may have misunderstood here: 
	QFontInfo fi(font());
	QFont f = font();
	if (!fi.exactMatch()){
		f= QFont(fi.family(),fi.pointSize(),fi.weight());
	}
	QFontMetrics fm(f);
	w = fm.width('W');
	h = fm.width('W');
	setCellWidth((int) (w*HPADDING));                        
	setCellHeight((int) (h*VPADDING)); 
	
	
	//setTableFlags( Tbl_vScrollBar |             
  //             Tbl_hScrollBar |            
  //             Tbl_clipCellPainting |      
   //            Tbl_smoothScrolling);       
	resize( 400, 200 );                         
	
	nAlignments=0;
	connectSignals();
	
	
}

SequenceEditor::~SequenceEditor()
{
}

void SequenceEditor::setProject(Project *newProject)
{
	// init() resets project_ to NULL so do anything related to the old project before calling init()
	disconnectSignals();
	
	
	init(); // make sure we are in a clean state
	project_=newProject;
	connectSignals();
	
	draggedSeq=NULL;
	// Now we need to resize the widget
	int maxlen=0;
	for (int s=0;s<project_->sequences.size();s++){ // checked OK
		int seqlen = project_->sequences.sequences().at(s)->residues.length();
		if (seqlen > maxlen)
			maxlen = seqlen;
	}
	setNumRows(project_->sequences.visibleSize()); // checked OK
	setNumCols( FLAGSWIDTH + LABELWIDTH + maxlen);
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
		if (col < FLAGSWIDTH){
			// FIXME
			return QChar(0);
		}
		else if (col >= FLAGSWIDTH && col < LABELWIDTH + FLAGSWIDTH ){ // in label field
			s = currSeq->label;
			pChar = s[col-FLAGSWIDTH];
			// FIXME looks like random copypasta
			if ((pChar.unicode() != 0) &&(col-FLAGSWIDTH < s.length()))
				return QChar(pChar.unicode() & maskFlags);
			else
				return QChar(0); // don't want to un/mask NULLs
		}
		else{ // in sequence field
			s=currSeq->residues;
			pChar= s[col-(LABELWIDTH+FLAGSWIDTH)];
			if (pChar.unicode() != 0){
				if (col < LABELWIDTH + FLAGSWIDTH + s.length())
					return QChar(pChar.unicode() & maskFlags);
				else
					return QChar(0);
			}
			else
				return QChar(0);
		}
	}
	else 
		return QChar(0);

}

void SequenceEditor::cutSelectedResidues()
{
	project_->cutSelectedResidues(); 
	if (project_->residueSelection->empty()){ // FIXME residues only
		selectingResidues_=false;
	}
	updateViewport();
}

void SequenceEditor::cutSelectedSequences()
{
	project_->cutSelectedSequences();
	updateViewport();
}

void SequenceEditor::excludeSelection()
{
	qDebug() << trace.header() << "SequenceEditor::excludeSelection()";
	
	int startRow=selAnchorRow,stopRow=selDragRow,
			startCol=selAnchorCol,stopCol=selDragCol,row,col;
			
	if (selectingResidues_){ // flag selected residues as being excluded from the
		selectingResidues_=false; // alignment
		// Order start and stop so they can be used in loops
		if (startRow > stopRow) swap_int(&startRow,&stopRow);
		if (startCol > stopCol) swap_int(&startCol,&stopCol);
		
		// Create a new undo record
		//project_->logOperation( new Operation(Operation::Mark,startRow,stopRow,
		//	startCol,stopCol,1));
			
		// Mark the residues
		for (row=startRow;row<=stopRow;row++)
			// Update past the deletion point only
			for (col=startCol;col<=stopCol;col++){
				setCellMark(row,col,true);
				updateCell(row,col);
			}
		update();	
	}
}

void SequenceEditor::removeExcludeSelection()
{
	// Selected cells that are marked are unmarked
	qDebug() << trace.header() << "SequenceEditor::removeExcludeSelection()";
	
	int startRow=selAnchorRow,stopRow=selDragRow,
			startCol=selAnchorCol,stopCol=selDragCol,row,col;
			
	if (selectingResidues_){ // flag selected residues as being excluded from the
		selectingResidues_=false; // alignment
		// Order start and stop so they can be used in loops
		if (startRow > stopRow) swap_int(&startRow,&stopRow);
		if (startCol > stopCol) swap_int(&startCol,&stopCol);
		
		// Create a new undo record
		//undoStack.push( new TEditRec(Edit_Mark,startRow,stopRow,
		//	startCol,stopCol,1));
		
		// Mark the residues
		for (row=startRow;row<=stopRow;row++)
			// Update past the deletion point only
			for (col=startCol;col<=stopCol;col++){
			  setCellMark(row,col,false);
				updateCell(row,col);
			}// of for (col=)
		update();
		
	}	// of if (selectingResidues_)
	
}

void SequenceEditor::setCellMark(int row,int col,int on)
{
	// TO DO - make portable the OR
	qDebug() << trace.header() << "SequenceEditor::setCellMark() row=" << row << " col=" << col << " on=" << on;
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (on)
		seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)] = 
			seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)].unicode() | EXCLUDE_CELL;
	else
		seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)] = 
			seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)].unicode() & (~EXCLUDE_CELL);
}

QColor SequenceEditor::getSequenceGroupColour(){
	currGroupColour_++;
	if (currGroupColour_ == N_GROUP_COLOURS+1)
		currGroupColour_=1;
	return QColor(groupColours[currGroupColour_-1][0],groupColours[currGroupColour_-1][1],groupColours[currGroupColour_-1][2]);
}

void SequenceEditor::updateViewport(){
	checkLength();
	this->repaint();
}
	


// QGridView public functions

int SequenceEditor::contentsX()
{
	return 0;
}

int SequenceEditor::contentsY()
{
	return 0;
}

	
int SequenceEditor::columnAt ( int x ) const
{
	return 0;
}

void SequenceEditor::ensureCellVisible ( int row, int column )
{
}

QSize SequenceEditor::gridSize () const
{
	return QSize();
}

int SequenceEditor::numCols () const
{
	return 0;
}

int SequenceEditor::numRows () const
{
	return 0;
}

void SequenceEditor::repaintCell ( int row, int column, bool erase )
{
}

int SequenceEditor::rowAt ( int y ) const
{
	return 0;
}

void SequenceEditor::setCellHeight ( int )
{
}

void SequenceEditor::setCellWidth ( int )
{
}

void SequenceEditor::setNumCols ( int )
{
}

void SequenceEditor::setNumRows ( int )
{
}

void SequenceEditor::updateCell ( int row, int column )
{
}


	

//
// Public slots
//

// Do any cleanup needed after loading a project
void SequenceEditor::postLoadTidy(){
	
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

void SequenceEditor::setEditorFont(const QFont &f)
{
	int w;
	int h;
	QFontInfo fi(f);
	QFont ftmp = f;
	if (!fi.exactMatch()){
		ftmp= QFont(fi.family(),fi.pointSize(),fi.weight());
	}
	QFontMetrics fm(ftmp);
	w = fm.width('W');
	h = fm.width('W'); // square looketh better ?
	setCellWidth((int) (w*HPADDING));                        
	setCellHeight((int) (h*VPADDING)); 
	setFont(ftmp);
	this->repaint();
}

//
// SequenceEditor - protected members
//

QRect SequenceEditor::cellGeometry ( int row, int column )
{
	return QRect();
}

void SequenceEditor::paintCell( QPainter* p, int row, int col )
{
	QChar c,cwflags;
	QColor txtColor;
	int cellSelected=false;
	
	Sequence *currSeq = project_->sequences.visibleAt(row);
	
	QRect r=cellGeometry(row,col);
	int w = r.width();
	int h = r.height();
	
	if (col==0 && currSeq->visible){
		if (currSeq->group != NULL){
			if (currSeq->group->locked()){
				txtColor.setRgb(255,0,0);
				p->setPen(txtColor);
				p->drawText( 0, 0, w, h, Qt::AlignCenter, "L");
				return;
			}
		}
	}
	
	c=cellContent(row,col,REMOVE_FLAGS);
	if (c.unicode()==0) return;
	cwflags=cellContent(row,col,KEEP_FLAGS);
	
	// If the cell is highlighted then do it
	
	if (selectingResidues_ && currSeq->visible){
	txtColor.setRgb(255,255,255);
		if (selAnchorRow <= selDragRow){ // dragging top to bottom
			if (selAnchorCol <= selDragCol){ // left to right
				if (row >= selAnchorRow && row <= selDragRow && 
					col >= selAnchorCol && col<=selDragCol){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=true;
					}
			}
			else{
				if (row >= selAnchorRow && row <= selDragRow && 
					col <= selAnchorCol && col>=selDragCol){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=true;
					}
			}
		}
		else{ // dragging bottom to top
			if (selAnchorCol <= selDragCol){ // left to right
				if (row <= selAnchorRow && row >= selDragRow && 
					col >= selAnchorCol && col<=selDragCol){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=true;
					}
			}
			else{
				if (row <= selAnchorRow && row >= selDragRow && 
					col <= selAnchorCol && col>=selDragCol){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=true;
					}
			}
		}
	}
	
	
	//  Draw cell content (text
	
	if (col >= FLAGSWIDTH && col < LABELWIDTH+FLAGSWIDTH && currSeq->visible){ 
		// Set colour of text
		if (currSeq->group != NULL)
			txtColor=currSeq->group->textColour();
		else
			txtColor.setRgb(255,255,255);
		if (project_->sequenceSelection->contains(currSeq))
			p->fillRect(0,0,w,h,QColor(128,128,128));
		// The selection doesn't get filled until the mouse is released
		// so whatever we are currently selecting has
		if (selectingSequences_ && leftDown_){
			int startRow = seqSelectionAnchor_;
			int stopRow  = seqSelectionDrag_;
			if (startRow > stopRow) swap_int(&startRow,&stopRow);
			if (row >= startRow && row <=stopRow)
				p->fillRect(0,0,w,h,QColor(128,128,128));
		}
	}
	else{
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
			int start = indexFirstVisibleSequenceInGroup(currSeq->group);
			int stop  = indexLastVisibleSequenceInGroup(currSeq->group);
			if (row==start){
				p->setPen(currSeq->group->textColour());
				p->drawLine(0,2,w-1,2);
			}
			if (row==stop){
				p->setPen(currSeq->group->textColour());
				p->drawLine(0,h-2,w-1,h-2);
			}
		}
	}
	
	if ((!cellSelected) && (cwflags.unicode() & EXCLUDE_CELL) && currSeq->visible ){
		//txtColor.setRgb(128,128,128);
		//p->fillRect(0,0,w,h,txtColor);
		p->setPen(QColor(240,240,16));
		p->drawLine(2,2,w-2,h-2); // X marks the spot ...
		p->drawLine(w-2,2,2,h-2);
	}
	
	if (currSeq->visible){
		p->setPen(txtColor);
		p->drawText( 0, 0, w, h, Qt::AlignCenter, c);
	}

}

void SequenceEditor::mousePressEvent( QMouseEvent* e )
{
	// Handles mouse press events for the SequenceEditor widget.
  // The current cell marker is set to the cell the mouse is clicked in.
	
	if (readOnly_) return;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (seq.count() == 0) return;
	
	QPoint clickedPos;
	int clickedRow,clickedCol;
	int col,row,startRow,stopRow,startCol,stopCol;
	
	clickedPos = e->pos();		
	clickedRow=rowAt( clickedPos.y() + contentsY());
	clickedCol=columnAt( clickedPos.x() +contentsX());
	
	// If we clicked outside the editing area ... well ... do nothing
	if ((clickedRow < 0) || (clickedRow > seq.count() -1) || (clickedCol <0 )){
		switch (e->button()){
			case Qt::LeftButton:
				break;
			default:break;
		} // end switch (e->button
		return;
	}
	
	// Clicked inside the "sequence label" area
	if ((clickedCol < LABELWIDTH+FLAGSWIDTH) && (clickedCol >= FLAGSWIDTH))
	{
		Sequence *selSeq =  project_->sequences.visibleAt(clickedRow);
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "selected " << selSeq->label;
		// Remove any residue selection
		selAnchorRow=selAnchorCol=selDragRow=selDragCol=-1; 
		selectingResidues_=false;
		project_->residueSelection->clear();
			
		switch (e->button()){
			case Qt::LeftButton:
			{
				switch (e->modifiers()){
					case Qt::NoModifier:
						project_->sequenceSelection->clear();
						seqSelectionAnchor_=seqSelectionDrag_=clickedRow;
						selectingSequences_=true;
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
	
	// Clicked inside the sequence area so we start selecting residues
	project_->sequenceSelection->clear(); // clear any other selections
	
	switch (e->button()){
		case Qt::LeftButton:
			leftDown_=true;
			if (selectingResidues_){
				// If we have already selected a block of cells
				// then we need to update these viz remove the highlight mark
				selectingResidues_=false; // this will block a redraw of the highlight
				if (selAnchorRow > selDragRow){ 
					startRow=selDragRow;
					stopRow=selAnchorRow;
				}
				else{
					startRow=selAnchorRow;
					stopRow=selDragRow;
				}
				if (selAnchorCol > selDragCol){ 
					startCol=selDragCol;
					stopCol=selAnchorCol;
				}
				else{
					startCol=selAnchorCol;
					stopCol=selDragCol;
				}
				for (row=startRow;row<=stopRow;row++)
					for (col=startCol;col<=stopCol;col++)
						updateCell(row,col);
			}
			// A new selection has been made
			selectingResidues_ = true;
			
			selAnchorRow = clickedRow;   // map to row; set current cell
			selAnchorCol = clickedCol;   // map to col; set current cell
			selDragRow = selAnchorRow;
			selDragCol = selAnchorCol;
			updateCell(selAnchorRow,selAnchorCol);
			break;
		case Qt::MidButton:
			break;
		case Qt::RightButton:
			break;
		default:break;
	}
	this->repaint();
}

void SequenceEditor::mouseReleaseEvent( QMouseEvent* e ){
	
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	
	if (readOnly_) return;
	
	QString l1,l2,s1,s2;
	
	switch (e->button()){
		case Qt::LeftButton:
		{
			leftDown_=false; // have finished selection
			if (selectingResidues_){
				qDebug() << trace.header(__PRETTY_FUNCTION__) << selAnchorRow << " " << selAnchorCol << " " << selDragRow << " " << selDragCol;
				int startRow=selAnchorRow,stopRow=selDragRow,startCol=selAnchorCol-(FLAGSWIDTH+LABELWIDTH),stopCol=selDragCol-(FLAGSWIDTH+LABELWIDTH);
				if (stopRow < startRow) swap_int(&startRow,&stopRow);
				if (stopCol < startCol) swap_int(&startCol,&stopCol);
				qDebug() << trace.header(__PRETTY_FUNCTION__) << startRow << " " << startCol << " " << stopRow << " " << stopCol;
				QList<ResidueGroup *> resSel;
				// startRow and stopRow are for the visible sequences but we need to loop over the hidden ones too
				startRow = project_->sequences.visibleToActual(startRow);
				stopRow  = project_->sequences.visibleToActual(stopRow);
				for (int r=startRow;r<=stopRow;r++){
					resSel.append(new ResidueGroup(project_->sequences.sequences().at(r),startCol,stopCol));
				}
				project_->residueSelection->set(resSel);
				// DO NOT delete the selection
			}
			else if (selectingSequences_){
				selectingSequences_=false;
				int startRow = seqSelectionAnchor_,stopRow=seqSelectionDrag_;
				if (stopRow < startRow) swap_int(&startRow,&stopRow);
				startRow = project_->sequences.visibleToActual(startRow);
				stopRow  = project_->sequences.visibleToActual(stopRow);
				for (int r=startRow;r<=stopRow;r++){
					project_->sequenceSelection->toggle(project_->sequences.sequences().at(r));
				}
			}
			break;
		}
		default:break;
	}
}


void SequenceEditor::contentsMouseMoveEvent(QMouseEvent *ev)
{
	
	QPoint clickedPos;
	int col,row,currRow,currCol;
	int startRow,stopRow,startCol,stopCol;
	int clickedRow,clickedCol;
	
	if (numRows() == 0) return; // so we don't have to guard against null pointers
	
	clickedPos = ev->pos();              		
	clickedRow=rowAt( clickedPos.y());
	clickedCol=columnAt( clickedPos.x());
	
	// clamp to bounds
	if (clickedRow < 0){
		if (clickedPos.y() >=0) // clamp to bottom
			clickedRow = numRows() -1;
		else // clamp to top
			clickedRow = 0;
	}
	
	if (clickedRow >= numRows())
		clickedRow = numRows() -1;
	
	// show the label of the sequence we are moving over
	Sequence *currSeq = project_->sequences.visibleAt(clickedRow);
	if (currSeq->label != lastInfo){
		lastInfo = currSeq->label;
		emit info(lastInfo);
	}
	
	if (readOnly_) return;
	
	if (leftDown_) // only scroll if we are selecting
		ensureCellVisible(clickedRow,clickedCol); // FIXME not ideal - scrolls too fast
		
	if (selectingSequences_ && leftDown_){
		if (seqSelectionDrag_ != clickedRow){
			currRow = clickedRow;
			startRow =  find_smallest_int(currRow,seqSelectionAnchor_,seqSelectionDrag_);
			stopRow  =  find_largest_int(currRow,seqSelectionAnchor_,seqSelectionDrag_);
			seqSelectionDrag_=currRow;
			// don't need to translate visible to actual here
			for (int row=startRow;row<=stopRow;row++)
				for (int col=FLAGSWIDTH; col < LABELWIDTH+FLAGSWIDTH; col ++)
					updateCell(row,col);
			qDebug() << trace.header(__PRETTY_FUNCTION__) << seqSelectionAnchor_ << " " << seqSelectionDrag_ ;
		}
		return;
	}
	
	if (leftDown_){ // FIXME in event?
		
		// Must be moving the highlight box around in the sequence field
		if (clickedCol < LABELWIDTH+FLAGSWIDTH && clickedCol >= FLAGSWIDTH)
			clickedCol = LABELWIDTH+FLAGSWIDTH;
		else if (clickedCol < 0){
			if (clickedPos.x() >= 0)
				clickedCol = numCols() -1;
			else
				clickedCol = LABELWIDTH+FLAGSWIDTH;
		}
		
		currRow = clickedRow;    // map to row; set current cell
		currCol = clickedCol;    // map to col; set current cell
		// Determine the bounds of the rectangle enclosing both
		// the old highlight box and the new highlight box (so we redraw cells in the old box)
		// Determine the vertical bounds
		startRow = find_smallest_int(currRow,selAnchorRow,selDragRow);
		stopRow  = find_largest_int(currRow,selAnchorRow,selDragRow);
		// Determine the horizontal bounds
		startCol= find_smallest_int(currCol,selAnchorCol,selDragCol);
		stopCol = find_largest_int(currCol,selAnchorCol,selDragCol);
		selDragRow=currRow;
		selDragCol=currCol;
		
		// Now update the cells
		for (row=startRow;row<=stopRow;row++)
			for (col=startCol;col<=stopCol;col++)
				updateCell(row,col);
	}
}

void SequenceEditor::mouseDoubleClickEvent(QMouseEvent *e){
	// Double clicking on a group member selects the whole group
	
	if (readOnly_) return;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (seq.count() == 0) return;
	
	QPoint clickedPos;
	int clickedRow,clickedCol;
	
	clickedPos = e->pos();		
	clickedRow=rowAt( clickedPos.y() + contentsY());
	clickedCol=columnAt( clickedPos.x() +contentsX());
	
	// If we clicked outside the editing area ... well ... do nothing
	if ((clickedRow < 0) || (clickedRow > project_->sequences.visibleSize() -1) || (clickedCol <0 )){
		switch (e->button()){
			case Qt::LeftButton:
				break;
			default:break;
		} // end switch (e->button
		return;
	}
	
	if ((clickedCol < LABELWIDTH+FLAGSWIDTH) && (clickedCol >= FLAGSWIDTH)){
		Sequence *selseq = project_->sequences.visibleAt(clickedRow);
		if (selseq->group){
			project_->sequenceSelection->clear();
			project_->addGroupToSelection(selseq->group);
		}
	}
	this->repaint();
}


void SequenceEditor::keyPressEvent( QKeyEvent* e )
{
	if (readOnly_) return;
	
	QString l;
	int startRow=selAnchorRow,stopRow=selDragRow,
			startCol=selAnchorCol,stopCol=selDragCol,row,col;	

	switch (e->key()){
		case Qt::Key_0:case Qt::Key_1:case Qt::Key_2:case Qt::Key_3:case Qt::Key_4:
		case Qt::Key_5:case Qt::Key_6:case Qt::Key_7:case Qt::Key_8:case Qt::Key_9:
			if (selectingResidues_){
				if (startCol != stopCol)
					return;
				else{
					//build up a string
					numStr.append(e->text());
				}
			}
			break;
		case Qt::Key_Space: // add insertions
			if (selectingResidues_){
				
				// Check that selection is only one column wide - if not
				// do nothing because the user probably hit the spacebar by mistake
				if (startCol != stopCol)
					return;
				else{
					if (startRow > stopRow) swap_int(&startRow,&stopRow);
			
					// The residue selection is contiguous but we could have selected part of a group at the beginning or end of the
					// selection. The members of the group are not necessarily contiguous however.
					// So create a list of all rows in which insertions will be made
					// First, find all of the unique groups in the selection
					
					QList<SequenceGroup *> sgl = project_->residueSelection->uniqueSequenceGroups();
					qDebug() << trace.header(__PRETTY_FUNCTION__) << "unique groups " << sgl.count(); 
					
					QList<Sequence *> insSeqs;
					
					// Add all of the sequences in each group to the list of sequences receiving insertions
					int firstVisibleLockedSequence = startRow;
					int lastVisibleLockedSequence= stopRow;
					
					for (int g=0;g<sgl.count();g++){
						SequenceGroup *sg = sgl.at(g);
						if (sg->locked()){
							for (int s=0;s<sg->size();s++){
								Sequence *seq = sg->itemAt(s);
								insSeqs.append(seq); // this adds non-visible sequences too
								if (seq->visible){
									row = indexVisibleSequence(seq);
									if (row >= 0){
										if (row < firstVisibleLockedSequence) firstVisibleLockedSequence = row;
										if (row > lastVisibleLockedSequence)  lastVisibleLockedSequence= row;
									}
								}
							}
						}
						qDebug() << trace.header(__PRETTY_FUNCTION__) << insSeqs.size() << " seqs to insert in"; 
					}
					
					// Now add in the rest of the selection to the list of sequences receiving insertions
					for (row=startRow;row<=stopRow;++row){
						Sequence *seq = project_->sequences.visibleAt(row);
						if (!insSeqs.contains(seq))
							insSeqs.append(seq);
					}
					qDebug() << trace.header(__PRETTY_FUNCTION__) << insSeqs.size() << " total seqs to insert in"; 
				
					// Is there a numeric argument to the insertion
					if (!(numStr.isEmpty())){
						stopCol=startCol+numStr.toInt()-1;
						numStr=""; // reset for reuse ...
					}
					else
						stopCol=startCol;
					
					bool postInsert = !(e->modifiers() & Qt::ShiftModifier);
					
					// Add 1 to startCol,stopCol because  post insertion is
					// used and undo deletes [startCol,stopCol]
					//project_->logOperation( new Operation(Operation::Insertion,startRow,stopRow,
					//		startCol+1,stopCol+1,Q3StrList()));
				
					// Insertions across multiple rows are allowed
					// Increase the size of the displayed area 
					setNumCols(numCols()+stopCol-startCol+1);
					
					for (int s=0;s<insSeqs.size();s++){
						if (postInsert)
							project_->sequences.addInsertions(insSeqs.at(s),startCol-LABELWIDTH-FLAGSWIDTH+1,stopCol-startCol+1);
						else
							project_->sequences.addInsertions(insSeqs.at(s),startCol-LABELWIDTH-FLAGSWIDTH,stopCol-startCol+1);	
					}
					
					checkLength();
					if (firstVisibleLockedSequence  < startRow) startRow = firstVisibleLockedSequence;
					if (lastVisibleLockedSequence   > stopRow)   stopRow = lastVisibleLockedSequence;
					for (row=startRow;row<=stopRow;++row){
						for (col=startCol;col< numCols();col++)
							updateCell(row,col);
					}	
				//emit alignmentChanged();
				} // end of if ... else
			} // end of if
			break;
	} // end of switch()		
	 
}

void SequenceEditor::focusInEvent( QFocusEvent* )
{
	// Handles focus reception events for the SequenceEditor widget.
	// Repaint only the current cell; to avoid flickering
	// updateCell( curRow, curCol );               // draw current cell
}    


void SequenceEditor::focusOutEvent( QFocusEvent* )
{
	// Handles focus loss events for the SequenceEditor widget.
	// Repaint only the current cell; to avoid flickering
	// updateCell( curRow, curCol );               // draw current cell
}    


//
//
//
void SequenceEditor::sequenceAdded(Sequence *s)
{

	int rowNum,rowLength;
	int sequenceLength = s->residues.length();
	// Stop setNumRows() from forcing a repaint
	// FIXME setAutoUpdate(FALSE);
	
	rowNum = numRows();
	if (s->visible) setNumRows(rowNum+1);
	rowLength = FLAGSWIDTH + LABELWIDTH + sequenceLength;
	
	// numCols() is initialized to zero so the following will
	// set numCols when the editor is empty too
	if (numCols()<rowLength) setNumCols(rowLength);
	
	// Now we update the new cells
	// repainting only this line
	
	if (loadingSequences_ || !s->visible) return; // defer repaints until loading has finished and no repaint if not visible
	
	for (unsigned int i=0;i<FLAGSWIDTH;i++)
		updateCell(rowNum,i);
	
	for (unsigned int i=0;i<LABELWIDTH;i++)
		updateCell(rowNum,i+FLAGSWIDTH);
		
	for (int i=0;i<sequenceLength;i++)
		updateCell(rowNum,i+LABELWIDTH+FLAGSWIDTH);

	this->repaint();
}

void SequenceEditor::sequencesCleared()
{
	setNumRows(0);
	setNumCols(0);
}

void SequenceEditor::loadingSequences(bool loading)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << loading;
	loadingSequences_= loading;
	if (!loadingSequences_)
		updateViewport();
}

//
// SequenceEditor - private members
//

void SequenceEditor::init()
{
	project_=NULL;
	readOnly_=false;
	
	loadingSequences_=false;
	
	selectingResidues_ = false;
	draggingSequence = false;
	draggedSeq=NULL;
	selAnchorRow=selAnchorCol=selDragRow=selDragCol=-1; 
	
	selectingSequences_ = false;
	seqSelectionAnchor_=seqSelectionDrag_=-1;
	
	leftDown_=false;
	lastInfo="";
	currGroupColour_=0;
}

void SequenceEditor::connectSignals()
{
	connect(&(project_->sequences),SIGNAL(sequenceAdded(Sequence *)),this,SLOT(sequenceAdded(Sequence *)));
	connect(&(project_->sequences),SIGNAL(cleared()),this,SLOT(sequencesCleared()));
	connect(project_,SIGNAL(loadingSequences(bool)),this,SLOT(loadingSequences(bool)));
}

void SequenceEditor::disconnectSignals()
{
	disconnect(&(project_->sequences),SIGNAL(sequenceAdded(Sequence *)),this,SLOT(sequenceAdded(Sequence *)));
	disconnect(&(project_->sequences),SIGNAL(cleared()),this,SLOT(sequencesCleared()));
	disconnect(project_,SIGNAL(loadingSequences(bool)),this,SLOT(loadingSequences(bool)));
}


void SequenceEditor::checkLength(){
	// The size of the region displayed needs to be checked after some
	// operations e.g. insert, delete so that we are not showing large 
	// blank areas in the display and so that the end of the slider takes
	// us to the end of the sequence
	int i,maxLength;
	
	// Number of rows is taken care of by deleteSequence(),
	// addSequence() and clearSequence()
	// All we need to do is find the longest row ..
	QList<Sequence *> &seq = project_->sequences.sequences();
	if (project_->sequences.visibleSize()==0){
		maxLength=0;
		setNumRows(0);
	}
 	else{
		setNumRows(project_->sequences.visibleSize());
		maxLength=seq.at(0)->residues.length();
		for (i=0;i<seq.count();i++)
			if (seq.at(i)->residues.length() > maxLength)
				maxLength = seq.at(i)->residues.length();
		maxLength+=LABELWIDTH+FLAGSWIDTH; 
	}
	setNumCols(maxLength);
	
  
}


int SequenceEditor::indexFirstVisibleSequenceInGroup(SequenceGroup *sg)
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

int SequenceEditor::indexLastVisibleSequenceInGroup(SequenceGroup *sg)
{
	int visIndex=project_->sequences.visibleSize()-1;
	for (int s=project_->sequences.size()-1;s>=0;s--){ // checked OK
		Sequence *seq = project_->sequences.sequences().at(s);
		if (sg==seq->group && seq->visible)
			return visIndex;
		if (seq->visible)
			visIndex--;
	}
	return -1;
}

int SequenceEditor::indexVisibleSequence(Sequence *seq)
{
	int visIndex=0;
	for (int s=0;s<project_->sequences.size();s++){ 
		Sequence *tmpseq = project_->sequences.sequences().at(s);
		if (tmpseq==seq)
			return visIndex;
		if (tmpseq->visible)
			visIndex++;
	}
	return -1;
}

//
//
//

void swap_int(int *a,int *b){
	int tmp;
	tmp = *b;
	*b=*a;
	*a=tmp;
}

int find_smallest_int(int a,int b,int c){
	int r;
	r=a;
	if (b<r) r=b;
	if (c<r) r=c;
	return r;
}

int find_largest_int(int a,int b,int c){
	int r;
	r=a;
	if (b>r) r=b;
	if (c>r) r=c;
	return r;
}

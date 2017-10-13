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

#include <Q3GridView> // FIXME
#include <Q3DragObject>
#include <Q3TextDrag>
#include <Q3ImageDrag>

#include <QPainter>
#include <QColor>
#include <QString>
//#include <qdragobject.h>
#include <QMessageBox>
#include <QMouseEvent>

#include "MessageWin.h"
#include "Operation.h"
#include "Project.h"
#include "SeqEdit.h"
#include "Sequence.h"
#include "SequenceSelection.h"


using namespace std;

#define FLAGSWIDTH 4
#define LABELWIDTH 16
#define NULLCHAR '\0'

#define HPADDING 0.9
#define VPADDING 0.9

//
// Main widget for the sequence editor
//

static void swap_int(int *a,int *b);
static int find_smallest_int(int a,int b,int c);
static int find_largest_int(int a,int b,int c);



// -----------------------------------------------------------------------------
//
// SeqEdit - the sequence editing widget
//
// -----------------------------------------------------------------------------

//
// SeqEdit - public members
//

SeqEdit::SeqEdit(Project *project,QWidget *parent)
	:Q3GridView(parent)
{
	
	QColor bcolour;

	project_=project;
	
	isSelected = FALSE;
	draggingSequence = FALSE;
	selAnchorRow=selAnchorCol=selDragRow=selDragCol=-1; 
	leftDown=FALSE;
	                     
	setFocusPolicy( Qt::StrongFocus );              // we accept keyboard focus
	setBackgroundRole( QPalette::Base ); 
	//QPalette pal = palette();
	//pal.setColor(QPalette::Background, Qt::black);
	//setAutoFillBackground(true);
	//setPalette(pal);
	bcolour.setRgb(0,0,0);
	setPaletteBackgroundColor(bcolour); // FIXME         
	setNumCols(0);                      
	setNumRows(0);
	
	// Cell width and height based on current font
	
  QFont currFont("Helvetica",14,50,FALSE);
	currFont.setBold(true);
	QFontMetrics fm (currFont);
		                      
	setCellWidth((int) (fm.maxWidth()*HPADDING));                        
	setCellHeight((int) (fm.lineSpacing()*VPADDING)); 
	                        
	//setTableFlags( Tbl_vScrollBar |             
  //             Tbl_hScrollBar |            
  //             Tbl_clipCellPainting |      
   //            Tbl_smoothScrolling);       
	resize( 400, 200 );                         
	
	//seq.setAutoDelete(TRUE);	// FIXME need to manage memory cleanup
	//undoStack.setAutoDelete(TRUE); // FIXME need to manage memory cleanup
	
	setAcceptDrops(TRUE);
	
	nAlignments=0;
	lockModeOn = FALSE;
	
	connect(project_,SIGNAL(sequenceAdded(Sequence *)),this,SLOT(sequenceAdded(Sequence *)));
	
}

SeqEdit::~SeqEdit()
{
	//seq.~QList();
	//undoStack.~QStack();
}

QChar SeqEdit::cellContent(int row, int col, int maskFlags )
{
	
	QChar pChar;
	QString s;
	
	QList<Sequence *> &seq = project_->sequences;
	
	// Note Since paintCell() calls this function before any sequences
	// are added have to return valid values when the editor is empty
	
	if (!seq.isEmpty()){
		if (col < FLAGSWIDTH){
			// FIXME
			return QChar(0);
		}
		else if (col >= FLAGSWIDTH && col < LABELWIDTH + FLAGSWIDTH ){ // in label field
			s = seq.at(row)->label;
			pChar = s[col-FLAGSWIDTH];
			// FIXME looks like random copypasta
			if ((pChar.unicode() != 0) &&(col-FLAGSWIDTH < s.length()))
				return QChar(pChar.unicode() & maskFlags);
			else
				return QChar(0); // don't want to un/mask NULLs
		}
		else{ // in sequence field
			s=seq.at(row)->residues;
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

void SeqEdit::cutSelection()
{
	qDebug() << trace.header() << "SeqEdit::cutSelection()";
	char *buf;
	Q3StrList l;
	
	int startRow=selAnchorRow,stopRow=selDragRow,
			startCol=selAnchorCol,stopCol=selDragCol,row,col;
	
	if (isSelected){ 
	
		// Order start and stop so they can be used in loops
		if (startRow > stopRow) swap_int(&startRow,&stopRow);
		if (startCol > stopCol) swap_int(&startCol,&stopCol);
		
		// Check whether the selected region contains residues
		// If it does, deletion is disallowed
		for (row=startRow;row<=stopRow;row++)
			for (col=startCol;col<=stopCol;col++)
				if (cellContent(row,col,REMOVE_FLAGS) != '-'){
					QMessageBox::information(this,"DeltaAlpha",
						"Bummer. You are not allowed to delete\n"
						"a block containing residues; only gaps can be deleted.");
					return;
				}
			
		isSelected=FALSE; 		
		// Create a deletion record
		buf = new char[stopCol-startCol+2];
		for (row=startRow;row<=stopRow;row++){
			for (col=startCol;col<=stopCol;col++)
				buf[col-startCol] = cellContent(row,col,KEEP_FLAGS).toAscii();
			buf[stopCol-startCol+1]=NULLCHAR;
			l.append(buf);
		}
		project_->logOperation( new Operation(Operation::Deletion,startRow,stopRow,
			startCol,stopCol,l));
		l.~Q3StrList();
		delete[] buf;
		
		for (row=startRow;row<=stopRow;row++){
			deleteCells(row,startCol,stopCol);
			checkLength();
			// Update past the deletion point only
			for (col=startCol;col<=stopCol;col++)
				updateCell(row,col);
		}
		update();
		//emit alignmentChanged();
	}	
}

void SeqEdit::excludeSelection()
{
	qDebug() << trace.header() << "SeqEdit::excludeSelection()";
	
	int startRow=selAnchorRow,stopRow=selDragRow,
			startCol=selAnchorCol,stopCol=selDragCol,row,col;
			
	if (isSelected){ // flag selected residues as being excluded from the
		isSelected=FALSE; // alignment
		// Order start and stop so they can be used in loops
		if (startRow > stopRow) swap_int(&startRow,&stopRow);
		if (startCol > stopCol) swap_int(&startCol,&stopCol);
		
		// Create a new undo record
		project_->logOperation( new Operation(Operation::Mark,startRow,stopRow,
			startCol,stopCol,1));
			
		// Mark the residues
		for (row=startRow;row<=stopRow;row++)
			// Update past the deletion point only
			for (col=startCol;col<=stopCol;col++){
				setCellMark(row,col,TRUE);
				updateCell(row,col);
			}
		update();	
	}
}

void SeqEdit::removeExcludeSelection()
{
	// Selected cells that are marked are unmarked
	qDebug() << trace.header() << "SeqEdit::removeExcludeSelection()";
	
	int startRow=selAnchorRow,stopRow=selDragRow,
			startCol=selAnchorCol,stopCol=selDragCol,row,col;
			
	if (isSelected){ // flag selected residues as being excluded from the
		isSelected=FALSE; // alignment
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
			  setCellMark(row,col,FALSE);
				updateCell(row,col);
			}// of for (col=)
		update();
		
	}	// of if (isSelected)
	
}

	

void SeqEdit::setCellMark(int row,int col,int on)
{
	// TO DO - make portable the OR
	qDebug() << trace.header() << "SeqEdit::setCellMark() row=" << row << " col=" << col << " on=" << on;
	QList<Sequence *> &seq = project_->sequences;
	
	if (on)
		seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)] = 
			seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)].unicode() | EXCLUDE_CELL;
	else
		seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)] = 
			seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)].unicode() & (~EXCLUDE_CELL);
}

void SeqEdit::lockSelection()
{
	// TO DO - lots
	qDebug() << trace.header() << "SeqEdit::lockSelection";
	
	int startRow=selAnchorRow,stopRow=selDragRow,
			startCol=selAnchorCol,stopCol=selDragCol,row,col;
			
	if (isSelected){ 
		isSelected=FALSE;
		// Order start and stop so they can be used in loops
		if (startRow > stopRow) swap_int(&startRow,&stopRow);
		if (startCol > stopCol) swap_int(&startCol,&stopCol);
		
		// TO DO Create a new undo record
		//undoStack.push( new TEditRec(Edit_Mark,startRow,stopRow,
		//	startCol,stopCol,1));
		
		// Mark the residues
		for (row=startRow;row<=stopRow;row++)
			// Update past the deletion point only
			for (col=startCol;col<=stopCol;col++){
				lockCell(row,col);
				updateCell(row,col);
			}
	}
	update();
}


void SeqEdit::unlockSelection()
{
	//TO DO
	qWarning() << warning.header() << "SeqEdit::unlockSelection() NOT IMPLEMENTED !!!";
}




//
// SeqEdit - public slots
//

void SeqEdit::lockMode(bool s)
{
	lockModeOn=s;
	// TO DO a special cursor ?
}


//
// SeqEdit - protected members
//

void SeqEdit::paintCell( QPainter* p, int row, int col )
{
	QChar c,cwflags;
	QColor txtColor;
	int cellSelected=FALSE;
	
	QList<Sequence *> &seq=project_->sequences;
	
	c=cellContent(row,col,REMOVE_FLAGS);
	if (c.unicode()==0) return;
	cwflags=cellContent(row,col,KEEP_FLAGS);
	
	// Handles cell painting for the SeqEdit widget.
	//int w = cellWidth( col );                   // width of cell in pixels
	//int h = cellHeight( row );                  // height of cell in pixels
	QRect r=cellGeometry(row,col);
	int w = r.width();
	int h = r.height();
	
	// If the cell is highlighted then do it
	
	if (isSelected){
	txtColor.setRgb(255,255,255);
		if (selAnchorRow <= selDragRow){ // dragging top to bottom
			if (selAnchorCol <= selDragCol){ // left to right
				if (row >= selAnchorRow && row <= selDragRow && 
					col >= selAnchorCol && col<=selDragCol){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=TRUE;
					}
			}
			else{
				if (row >= selAnchorRow && row <= selDragRow && 
					col <= selAnchorCol && col>=selDragCol){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=TRUE;
					}
			}
		}
		else{ // dragging bottom to top
			if (selAnchorCol <= selDragCol){ // left to right
				if (row <= selAnchorRow && row >= selDragRow && 
					col >= selAnchorCol && col<=selDragCol){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=TRUE;
					}
			}
			else{
				if (row <= selAnchorRow && row >= selDragRow && 
					col <= selAnchorCol && col>=selDragCol){
						p->fillRect(0,0,w,h,txtColor);
						cellSelected=TRUE;
					}
			}
		}
	}
	
	
	//  Draw cell content (text)
	
	Sequence *currSeq = seq.at(row);
	if (project_->sequenceSelection->contains(currSeq)){
		p->fillRect(0,0,w,h,QColor(128,128,128));
	}
	if (col >= FLAGSWIDTH && col < LABELWIDTH+FLAGSWIDTH){ 
		// Set colour of text
		if (currSeq->group > 0)
			txtColor.setRgb(255,255,0);
		else
			txtColor.setRgb(255,255,255);
	}
	else{

		switch (c.toAscii()){
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
	}
	
	p->setPen(txtColor);
	p->drawText( 0, 0, w, h, Qt::AlignCenter, c);
	
	if ((!cellSelected) && (cwflags.unicode() & EXCLUDE_CELL) ){
		//txtColor.setRgb(128,128,128);
		//p->fillRect(0,0,w,h,txtColor);
		txtColor.setRgb(255,0,0);
		p->setPen(txtColor);
		p->drawLine(2,2,w-2,h-2); // X marks the spot ...
		p->drawLine(w-2,2,2,h-2);
	}
	
	// Draw lock marks
	
	if (!cellSelected && (cwflags.unicode() & LOCK_CELL)){
		txtColor.setRgb(0,255,0); // TO DO colour setting
		p->setPen(txtColor);
		p->drawRect(2,2,w-2,h-2);
	}
}

void SeqEdit::mousePressEvent( QMouseEvent* e )
{
	// Handles mouse press events for the SeqEdit widget.
  // The current cell marker is set to the cell the mouse is clicked in.
	
	QList<Sequence *> &seq = project_->sequences;
	
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
	
	if ((clickedCol < LABELWIDTH+FLAGSWIDTH) && (clickedCol >= FLAGSWIDTH))
	{
		Sequence *selSeq = seq.at(clickedRow);
		qDebug() << trace.header() << "Selected " << selSeq->label;
		// The usual logic:
		// If no key pressed then clear the selection
		
		switch (e->modifiers())
		{
			case Qt::NoModifier:
				project_->sequenceSelection->set(selSeq);
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
		this->viewport()->repaint();
		return;
	}
	
	// If we are in normal mode and we have clicked inside the sequence
	// label field then start a drag
	
	if ((clickedCol < LABELWIDTH+FLAGSWIDTH) && (clickedCol >= FLAGSWIDTH) && !lockModeOn){
		// Get  the label
		draggedRowNum=clickedRow;
		QString l = (seq.at(clickedRow)->label).stripWhiteSpace();
		QRect br = fontMetrics().boundingRect(l,-1);
		//QPixmap pm(br.width(),br.height(),-1,QPixmap::DefaultOptim); // CHECK ME
		QPixmap pm(br.width(),br.height());
		QPainter p;
		p.begin(&pm);
			p.fillRect(0,0,br.width(),br.height(),QBrush(QColor(Qt::black)));
			p.setPen(QColor(Qt::yellow));
			p.setFont(currFont);
			p.drawText(0,0,br.width(),br.height(),
				Qt::AlignCenter,l);
		p.end();
		Q3DragObject *d = new Q3TextDrag(l,this);
		d->setPixmap(pm,QPoint(-5,-7));
		d->dragMove();
		draggingSequence = FALSE;
		leftDown=TRUE;
		return;
	}
	 
	// Must have clicked inside the sequence area so we start
	// a selection	
	switch (e->button()){
		case Qt::LeftButton:
			leftDown=TRUE;
			if (isSelected){
				// If we have already selected a block of cells
				// then we need to update these viz remove the highlight mark
				isSelected=FALSE; // this will block a redraw of the highlight
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
			isSelected = TRUE;
			
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
	
}

void SeqEdit::mouseReleaseEvent( QMouseEvent* e ){
	
	QString l1,l2,s1,s2;
	
	switch (e->button()){
		case Qt::LeftButton:
			leftDown=FALSE; // have finished selection
			break;
		case Qt::MidButton:
			break;
		case Qt::RightButton:
			break;
		default:break;
	}
}

void SeqEdit::mouseMoveEvent( QMouseEvent* e ){
	QPoint clickedPos;
	int col,row,currRow,currCol;
	int startRow,stopRow,startCol,stopCol;
	int clickedRow,clickedCol;
	
	if (leftDown){
		clickedPos = e->pos();              		
		clickedRow=rowAt( clickedPos.y() +contentsY());
		clickedCol=columnAt( clickedPos.x() + contentsX());
		
		// Must be moving the highlight box around in the sequence field
		if (clickedCol < LABELWIDTH+FLAGSWIDTH && clickedCol >= FLAGSWIDTH)
			clickedCol = LABELWIDTH+FLAGSWIDTH;
		else if (clickedCol < 0){
			if (clickedPos.x() >= 0)
				clickedCol = numCols() -1;
			else
				clickedCol = LABELWIDTH+FLAGSWIDTH;
		}
		
		if (clickedRow < 0){
			if (clickedPos.y() >=0) // clamp to bottom
				clickedRow = numRows() -1;
			else // clamp to top
				clickedRow = 0;
		}
		
		currRow = clickedRow;    // map to row; set current cell
		currCol = clickedCol;    // map to col; set current cell
		// Determine the bounds of the rectangle enclosing both
		// the old highlight box and the new highlight box
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

void SeqEdit::mouseDoubleClickEvent(QMouseEvent *e){
	// Double clicking on a group member selects the whole group
	QList<Sequence *> &seq = project_->sequences;
	
	if (seq.count() == 0) return;
	
	QPoint clickedPos;
	int clickedRow,clickedCol;
	
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
	
	if ((clickedCol < LABELWIDTH+FLAGSWIDTH) && (clickedCol >= FLAGSWIDTH)){
		Sequence *selseq = project_->sequences.at(clickedRow);
		if (selseq->group){
			project_->sequenceSelection->clear();
			project_->addGroupToSelection(selseq->group);
		}
	}
}


void SeqEdit::keyPressEvent( QKeyEvent* e )
{
	// Handles key press events for the SeqEdit widget.
	
	QString l;
	int startRow=selAnchorRow,stopRow=selDragRow,
			startCol=selAnchorCol,stopCol=selDragCol,row,col;	
	
	if (lockModeOn){
		// TO DO anything special ??
	}
	else{			
  	switch (e->key()){
			case Qt::Key_0:case Qt::Key_1:case Qt::Key_2:case Qt::Key_3:case Qt::Key_4:
			case Qt::Key_5:case Qt::Key_6:case Qt::Key_7:case Qt::Key_8:case Qt::Key_9:
				if (isSelected){
					if (startCol != stopCol)
						return;
					else{
						//build up a string
						numStr.append(e->text());
					}
				}
				break;
			case Qt::Key_Space:
				if (isSelected){
				
					// Check that selection is only one column wide - if not
					// do nothing because the user probably hit the spacebar by mistake
					if (startCol != stopCol)
						return;
					else{
						if (startRow > stopRow) swap_int(&startRow,&stopRow);
			
						// Is there a numeric argument to the insertion
						if (!(numStr.isEmpty())){
							stopCol=startCol+numStr.toInt()-1;
							numStr=""; // reset for reuse ...
						}
					else
							stopCol=startCol;
						// Add 1 to startCol,stopCol because  post insertion is
						// used and undo deletes [startCol,stopCol]
						project_->logOperation( new Operation(Operation::Insertion,startRow,stopRow,
								startCol+1,stopCol+1,Q3StrList()));
					
						// Insertions across multiple rows are allowed
						// Increase the size of the displayed area 
						setNumCols(numCols()+stopCol-startCol+1);
						l.fill(QChar('-'),stopCol-startCol+1);
						for (row=startRow;row<=stopRow;row++){
							insertCells(l,row,startCol);
							checkLength();
							for (col=startCol;col< numCols();col++)
								updateCell(row,col);
						}	// end of for loop
						//emit alignmentChanged();
					} // end of if ... else
				} // end of if
				break;
			} // end of switch()		
	 }
}

void SeqEdit::focusInEvent( QFocusEvent* )
{
	// Handles focus reception events for the SeqEdit widget.
	// Repaint only the current cell; to avoid flickering
	// updateCell( curRow, curCol );               // draw current cell
}    


void SeqEdit::focusOutEvent( QFocusEvent* )
{
	// Handles focus loss events for the SeqEdit widget.
	// Repaint only the current cell; to avoid flickering
	// updateCell( curRow, curCol );               // draw current cell
}    


//
//
//
void SeqEdit::sequenceAdded(Sequence *s)
{
	int rowNum,rowLength;
	int sequenceLength = s->residues.length();
	// Stop setNumRows() from forcing a repaint
	// FIXME setAutoUpdate(FALSE);
	
	rowNum = numRows();
	setNumRows(rowNum+1);
	rowLength = FLAGSWIDTH + LABELWIDTH + sequenceLength;
	
	// numCols() is initialized to zero so the following will
	// set numCols when the editor is empty too
	if (numCols()<rowLength) setNumCols(rowLength);
	
	// Now we update the new cells
	// repainting only this line
	
	for (unsigned int i=0;i<FLAGSWIDTH;i++)
		updateCell(rowNum,i);
	
	for (unsigned int i=0;i<LABELWIDTH;i++)
		updateCell(rowNum,i+FLAGSWIDTH);
		
	for (int i=0;i<sequenceLength;i++)
		updateCell(rowNum,i+LABELWIDTH+FLAGSWIDTH);
	
	this->viewport()->repaint();
}


//
// SeqEdit - private members
//

void SeqEdit::insertCell(char c,int row,int col){
	// TO DO - not actually using this function ...
	// Can only insert into a sequence so ..
	QList<Sequence *> &seq = project_->sequences;
	seq.at(row)->residues.insert(col-LABELWIDTH-FLAGSWIDTH+1,c);
	update();	
}

void SeqEdit::insertCells(QString s,int row,int col){
	QString t;
	QList<Sequence *> &seq = project_->sequences;
	(seq.at(row)->residues).insert(col-LABELWIDTH-FLAGSWIDTH+1,s); // post insertion
	update();
}

void SeqEdit::deleteCells(int row,int start,int stop){
	QList<Sequence *> &seq = project_->sequences;
	(seq.at(row)->residues).remove(start-(LABELWIDTH+FLAGSWIDTH),stop-start+1);
	update();
}

void SeqEdit::lockCell(int row,int col){
	QList<Sequence *> &seq = project_->sequences;
	if (col>=LABELWIDTH+FLAGSWIDTH)
		seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)] = 
			seq.at(row)->residues[col-(LABELWIDTH+FLAGSWIDTH)].unicode() ^ LOCK_CELL;
}


void SeqEdit::checkLength(){
	// The size of the region displayed needs to be checked after some
	// operations e.g. insert, delete so that we are not showing large 
	// blank areas in the display and so that the end of the slider takes
	// us to the end of the sequence
	int i,maxLength;
	
	// Number of rows is taken care of by deleteSequence(),
	// addSequence() and clearSequence()
	// All we need to do is find the longest row ..
	QList<Sequence *> &seq = project_->sequences;
	if (numRows()==0)
		maxLength=0;
 	else{
		maxLength=seq.at(0)->residues.length();
		for (i=0;i<seq.count();i++)
			if (seq.at(i)->residues.length() > maxLength)
				maxLength = seq.at(i)->residues.length();
		maxLength+=LABELWIDTH+FLAGSWIDTH; 
	}
	setNumCols(maxLength);
	
  
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

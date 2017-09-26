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

#include "SeqEdit.h"
#include "Sequence.h"
#include "MessageWin.h"

using namespace std;

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
// TEditRec - private class  for the undo stack
//
// -----------------------------------------------------------------------------

TEditRec::TEditRec(int eMode,int startR,int stopR,
	int startC,int stopC,Q3StrList t){
	
	
	editMode = eMode;
	startRow = startR;
	stopRow = stopR;
	startCol = startC;
	stopCol= stopC;
	editText = t;
	// seq.setAutoDelete(TRUE); // FIXME need to manage memory cleanup
}

TEditRec::TEditRec(int eMode,QList <Sequence *> s)
{
	// Constructor to store contents of the sequence editor
	
	editMode=eMode;
	for ( int i=0;i<s.count();i++)
		seq.append(new Sequence(s.at(i)->label,s.at(i)->residues));
	
}

TEditRec::~TEditRec()
{
	//editText.~QStrList(); // Check THIS
	//if (!seq.isEmpty()) seq.~QList(); // CHECK
}

// -----------------------------------------------------------------------------
//
// SeqEdit - the sequence editing widget
//
// -----------------------------------------------------------------------------

//
// SeqEdit - public members
//

SeqEdit::SeqEdit(QWidget *parent)
	:Q3GridView(parent)
{
	
	QColor bcolour;

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
	
}

SeqEdit::~SeqEdit()
{
	//seq.~QList();
	//undoStack.~QStack();
}


QString SeqEdit::getSequence(int i,int maskFlags)
{
	
	// First sequence has id 0
	// REMOVE_FLAGS in this context means return only those
	// residues which are not to be excluded from the alignment
	
	// It is intended that this function be mainly used to construct
	// a string suitable for use by an external alignment program
	
	QString r;
	int j,k=0;
	
	qDebug() << trace.header() << "SeqEdit::getSequence()" << i << " " << maskFlags;
	// Return NULL if the index is out of range
	if ( i > seq.count()-1)
		return NULL;
	else{
		r=seq.at(i)->residues;
		switch (maskFlags)
		{
			case KEEP_FLAGS:
				break;
			case REMOVE_FLAGS:
			  // Excluded residues are removed from the returned sequence
				for (j=0;j<r.length();j++)
				  if (!(r[j].unicode() & EXCLUDE_CELL))
					{
						r[k]=r[j];
						k++;
					}
				// Chop off any remaining residues
				// 09052007 BUG FIXED - was removing the last residue
				r.truncate(k);
				//if (k!=0)
				//	r.truncate(k-1);
				//else
				//	r.truncate(0);
					
				break;
		}
		return r;
	}
	
}

QString SeqEdit::getSequence(QString l)
{
	// Returns the (masked) sequence with label l
	// Mainly for use by other programs
	// Returns NULL if nothing matching is found
	qDebug() << trace.header() << "SeqEdit::getSequence() " << l;
	
	int i;
	if ((i=getSeqIndex(l)) == -1)
		return NULL;
	else
		return seq.at(i)->residues;
}

QString SeqEdit::getLabel(int i)
{
	qDebug() << trace.header() << "SeqEdit::getLabel() " << i;
	// Return NULL if the index is out of range
	if (i<0 || i > numRows()-1)
		return NULL;
	else
		// strip white space from the end of the string
		return (seq.at(i)->label).stripWhiteSpace(); 
}

int SeqEdit::numSequences()
{
	return seq.count();
}

QChar SeqEdit::cellContent(int row, int col, int maskFlags )
{
	
	QChar pChar;
	QString s;
	
	// Note Since paintCell() calls this function before any sequences
	// are added have to return valid values when the editor is empty
	
	if (!seq.isEmpty()){
		if (col < LABELWIDTH ){ // in label field
			s = seq.at(row)->label;
			pChar = s[col];
			if ((pChar.unicode() != 0) &&(col < s.length()))
				return QChar(pChar.unicode() & maskFlags);
			else
				return QChar(0); // don't want to un/mask NULLs
		}
		else{ // in sequence field
			s=seq.at(row)->residues;
			pChar= s[col-LABELWIDTH];
			if (pChar.unicode() != 0){
				if (col < LABELWIDTH + s.length())
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

void SeqEdit::addSequence( QString l,QString s,QString c )
{
	
	int rowNum,rowLength;

	qDebug() << trace.header() << "SeqEdit::addSequence()\n" << l << " " << s;
	seq.append(new Sequence(l,s,c));
	
	// Stop setNumRows() from forcing a repaint
	// FIXME setAutoUpdate(FALSE);
	
	rowNum = numRows();
	setNumRows(rowNum+1);
	rowLength = LABELWIDTH + strlen(s);
	
	qDebug() << trace.header() << "SeqEdit::addSequence() row length=" << rowLength << " numCols=" << numCols();
	// numCols() is initialized to zero so the following will
	// set numCols when the editor is empty too
	if (numCols()<rowLength) setNumCols(rowLength);
	
	//FIXME setAutoUpdate(TRUE);
	
	// Now we update the new cells
	// repainting only this line
	
	for (unsigned int i=0;i<LABELWIDTH;i++)
		updateCell(rowNum,i);
		
	for (unsigned int i=0;i<strlen(s);i++)
		updateCell(rowNum,i+LABELWIDTH);
		
	update();
	
}

void SeqEdit::clearSequences()
{
	// TO DO - more stuff ? Difference between clearing and a new project ?

	qDebug() << trace.header() << "SeqEdit::clearSequences()";
	
	seq.clear();
	update();
	emit alignmentChanged();
}

int SeqEdit::deleteSequence(QString l){
	// TODO
	// Deletes the sequence with identifier id
	// Returns position of the deleted sequence, -1 if id was not found
	qWarning() << warning.header() << "SeqEdit::deleteSequence() NOT IMPLEMENTED!";
	int i;
	if ((i=getSeqIndex(l))>=0){ // found it
	}
	return i;
}

void SeqEdit::insertSequence(QString ,QString ,int )
{
	qWarning() << warning.header() << "SeqEdit::insertSequence() NOT IMPLEMENTED!";
}

int SeqEdit::replaceSequence(QString l,QString newLabel,QString newRes){
	// Replace sequencewith identifier id
	// Returns position of the replace sequence, -1 if id was not found
	qDebug() << trace.header() << "SeqEdit::replaceSequence()";
	int i;
	if ((i=getSeqIndex(l))>=0){ // found it
		deleteSequence(l);
		insertSequence(newLabel,newRes,i);
	}
	update();
	return  i;
	
}

void SeqEdit::moveSequence(int i,int j)
{
	// Moves the sequence at position i to position j
	qDebug() << trace.header() << "SeqEdit::moveSequence() from " << i << " " << j;
	//Sequence *pSeq=seq.take(i);
	//seq.insert(j,pSeq);
	if (i<0 || j <0 || i >= seq.size() || j >= seq.size()){
		return;
	}
	seq.move(i,j);
	update();
	emit alignmentChanged();
}

void SeqEdit::changeResidues(QString r,int pos)
{
	qDebug() << trace.header() << "SeqEdit::changeResidues()  " << r << " " << pos;
	seq.at(pos)->residues=r;
	update();
	emit alignmentChanged();
}


void SeqEdit::newAlignment(QList <Sequence *> s)
{
	// Called after making an alignment
	qDebug() << trace.header() << "SeqEdit::newAlignment()";
	undoStack.push(new TEditRec(Edit_Alignment,seq));
	seq.clear();
	for (int i=0;i<s.count();i++)
	{
		seq.append(new Sequence(s.at(i)->label,s.at(i)->residues));
		int rowLength = LABELWIDTH + s.at(i)->residues.length();
		if (numCols()<rowLength) setNumCols(rowLength);
	}	
	update();
	nAlignments++;
	emit alignmentChanged();
}

void SeqEdit::undoEdit()
{

	// Undo last editing command
	qDebug() << trace.header() << "SeqEdit::undoEdit()";
	int startRow,stopRow,startCol,stopCol,row,firstRow;
	int col;
	TEditRec *er;
	
	if (!undoStack.isEmpty()){
	
		er = undoStack.top(); // FIXME ported but not tested
		startRow=er->startRow;
		stopRow =er->stopRow;
		startCol=er->startCol;
		stopCol=er->stopCol;
		
		switch (er->editMode){
		
   		case Edit_Insertion:
     		for (row=startRow;row<=stopRow;row++){
      		deleteCells(row,startCol,stopCol);
					checkLength();
      		// Update past the deletion point only
      		for (col=startCol;col<numCols();col++)
        		updateCell(row,col);
    	 	}
    		
     		break;
				
   		case Edit_Deletion:
    		
    		for (row=startRow;row<=stopRow;row++){
     			insertCells((er->editText).at(row-startRow),
         		row,startCol-1); // subtract 1 because we get a post-insertion
					checkLength();
       		for (col=startCol;col< numCols();col++)
       			updateCell(row,col);
    		}
    		
     		break;
				
			case Edit_Mark:
    		for (row=startRow;row<=stopRow;row++)
      		for (col=startCol;col<=stopCol;col++){
        		setCellMark(row,col,FALSE);
        		updateCell(row,col);
      		}
				break;
			case Edit_Move:
				moveSequence(stopRow,startRow);
				firstRow=stopRow;
				if (startRow<stopRow) firstRow=startRow;
				for (row=firstRow;row<seq.count();row++)
					for (col=0;col<seq.at(row)->residues.length()+LABELWIDTH;col++)
						updateCell(row,col); 
    		break;
			case Edit_Alignment:
				setAlignment(er->seq);
				nAlignments--; // changeAlignment increments so decrement by 2
				break;
		} // end of case
 		TEditRec * er = undoStack.pop(); delete er; // FIXME ported but not tested
 		update();
		emit alignmentChanged();
	} // end of if
	else{
		// Make a rude sound
		printf("\a");
	}
}

void SeqEdit::redoEdit()
{
	// TO DO
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
		undoStack.push( new TEditRec(Edit_Deletion,startRow,stopRow,
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
		emit alignmentChanged();
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
		undoStack.push( new TEditRec(Edit_Mark,startRow,stopRow,
			startCol,stopCol,1));
			
		// Mark the residues
		for (row=startRow;row<=stopRow;row++)
			// Update past the deletion point only
			for (col=startCol;col<=stopCol;col++){
				setCellMark(row,col,TRUE);
				updateCell(row,col);
			}
		update();	
		emit alignmentChanged();
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
		emit alignmentChanged();
	}	// of if (isSelected)
	
}

	

void SeqEdit::setCellMark(int row,int col,int on)
{
	// TO DO - make portable the OR
	qDebug() << trace.header() << "SeqEdit::setCellMark() row=" << row << " col=" << col << " on=" << on;
	
	if (on)
		seq.at(row)->residues[col-LABELWIDTH] = 
			seq.at(row)->residues[col-LABELWIDTH].unicode() | EXCLUDE_CELL;
	else
		seq.at(row)->residues[col-LABELWIDTH] = 
			seq.at(row)->residues[col-LABELWIDTH].unicode() & (~EXCLUDE_CELL);
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


void SeqEdit::undoLastAlignment()
{
	qDebug() << trace.header() << "SeqEdit::undoLastAlignment";
	TEditRec *er;
	
	if (nAlignments > 0){
		// Find the most recent alignment on the undo stack
		// removing all other edit records
		// TO DO the redo stack
		er=undoStack.top();
		while (er->editMode != Edit_Alignment){ // FIXME ported but not checked yet
			undoStack.pop();
			delete er;
			er = undoStack.top();
		}
		setAlignment(er->seq);
		nAlignments--;
		update();
		emit alignmentChanged();
	}
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
	
	if (col < LABELWIDTH){ 
		// Set colour of text
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
	
	// If we are in normal mode and we have clicked inside the sequence
	// label field then start a drag
	
	if ((clickedCol < LABELWIDTH) && (clickedCol >= 0) && !lockModeOn){
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
		if (clickedCol < LABELWIDTH && clickedCol >= 0)
			clickedCol = LABELWIDTH;
		else if (clickedCol < 0){
			if (clickedPos.x() >= 0)
				clickedCol = numCols() -1;
			else
				clickedCol = LABELWIDTH;
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
						undoStack.push( new TEditRec(Edit_Insertion,startRow,stopRow,
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
						emit alignmentChanged();
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

void SeqEdit::dragEnterEvent(QDragEnterEvent *event){
   event->accept(
        Q3TextDrag::canDecode(event) ||
        Q3ImageDrag::canDecode(event));
	
}

void SeqEdit::dropEvent(QDropEvent *e)
{
	// TO DO
	qDebug() << trace.header() << "SeqEdit::dropEvent()";
	int droppedRow=rowAt(e->pos().y()+contentsY());
	
	// Dropped sequences are pre-inserted
	// This means that there is no problem with dropping a sequence
	// at the beginning of the list.
	// To drop into the last place, have to do something yucky and use
	// the value -1 return by findRow to indicate that the dragged row must
	// be made the last row
	
	if (droppedRow != draggedRowNum){
		if (droppedRow <0){ // move dragged row to the end
			Sequence *pSeq=seq.takeAt(draggedRowNum); // FIXME checked darggedRowNum ??
			seq.append(pSeq);
			undoStack.push( new TEditRec(Edit_Move,draggedRowNum,numSequences()-1,
				0,0,Q3StrList()));
		}
		else{
			moveSequence(draggedRowNum,droppedRow);
			undoStack.push( new TEditRec(Edit_Move,draggedRowNum,droppedRow,
				0,0,Q3StrList()));
		}
	
		update();
		emit alignmentChanged();
	}
	
}

//
// SeqEdit - private members
//

void SeqEdit::insertCell(char c,int row,int col){
	// TO DO - not actually using this function ...
	// Can only insert into a sequence so ..
	
	seq.at(row)->residues.insert(col-LABELWIDTH+1,c);
	update();	
}

void SeqEdit::insertCells(QString s,int row,int col){
	QString t;
	
	(seq.at(row)->residues).insert(col-LABELWIDTH+1,s); // post insertion
	update();
}

void SeqEdit::deleteCells(int row,int start,int stop){
	

	(seq.at(row)->residues).remove(start-LABELWIDTH,stop-start+1);
	update();
}

void SeqEdit::lockCell(int row,int col){
	if (col>=LABELWIDTH)
		seq.at(row)->residues[col-LABELWIDTH] = 
			seq.at(row)->residues[col-LABELWIDTH].unicode() ^ LOCK_CELL;
	
}

void SeqEdit::setAlignment(QList <Sequence *> s){

	seq.clear();
	for (int i=0;i<s.count();i++)
		seq.append(new Sequence(s.at(i)->label,s.at(i)->residues));	
	update();
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
	if (numRows()==0)
		maxLength=0;
 	else{
		maxLength=seq.at(0)->residues.length();
		for (i=0;i<seq.count();i++)
			if (seq.at(i)->residues.length() > maxLength)
				maxLength = seq.at(i)->residues.length();
		maxLength+=LABELWIDTH; 
	}
	setNumCols(maxLength);
	
  
}

int SeqEdit::getSeqIndex(QString l){
	// Get the index of the sequence with label l
	// Returns -1 if no match
	int i=0;
	QString t=l.stripWhiteSpace();// TO DO why is this here ?
	while ((i<seq.count()) && (getLabel(i) != t)) i++;
	if (i==seq.count())
		return -1;
	else
		return i;
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

//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
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

#ifndef __SEQ_EDIT_H_
#define __SEQ_EDIT_H_

#include <Q3GridView> // FIXME
#include <QList>
#include <QStack>
#include <Q3StrList> // FIXME

class QString;
class QFont;
class QPixmap;  
class QDragEnterEvent;
class QDropEvent;

#define Edit_Deletion	0
#define Edit_Insertion	1
#define Edit_Mark 2
#define Edit_Move 3
#define Edit_Alignment 4
#define Edit_Lock 5

#define KEEP_FLAGS 0XFFFF // TO DO change all this to allow higher order bits
#define REMOVE_FLAGS	0X007F	

#define EXCLUDE_CELL 0x0080
#define LOCK_CELL	0x0100

enum alignmentFormats {FASTA,CLUSTALW};

class Project;
class Sequence;

// Keeps information about an edit operation
// so that it can be undone
class TEditRec
{
public:
	TEditRec(int,int,int,int,int,Q3StrList);
	TEditRec(int,QList <Sequence *>);
	~TEditRec();
	int editMode;
	int startRow,stopRow,startCol,stopCol;
	Q3StrList editText;
	QList <Sequence *> seq;
};

class SeqEdit:public Q3GridView
{

Q_OBJECT

public:
	SeqEdit(Project *,QWidget *parent);
	~SeqEdit();
	
	QString getSequence(int,int);
	QString getSequence(QString);
	QString getLabel(int);
	int numSequences();

	QChar cellContent(int,int,int );
	
	void clearSequences();
	void addSequence(QString ,QString,QString );
	int  deleteSequence(QString);
	void insertSequence(QString,QString,int);
	int  replaceSequence(QString,QString,QString);
	void moveSequence(int,int);
	void changeResidues(QString ,int pos);
	void newAlignment(QList <Sequence *>);
	
	void undoEdit();
	void redoEdit();
	void cutSelection();
	
	void excludeSelection();
	void removeExcludeSelection();
	void lockSelection();
	void unlockSelection();
	
	void undoLastAlignment();

public slots:

	void lockMode(bool);
			
protected:

	void paintCell( QPainter*, int row, int col );
	void mousePressEvent( QMouseEvent* );
	void mouseReleaseEvent( QMouseEvent* );
	void mouseMoveEvent( QMouseEvent* );
	void keyPressEvent( QKeyEvent* );
	void focusInEvent( QFocusEvent* );
	void focusOutEvent( QFocusEvent* );
	virtual void dragEnterEvent(QDragEnterEvent *);
	virtual void dropEvent(QDropEvent *);
	
signals:

	void alignmentChanged();
	
private:
	
	void insertCell(char c,int row,int col);
	void insertCells(QString,int row,int col);
	void deleteCells(int r,int start,int stop);
	void setCellMark(int row,int col,int on);
	void lockCell(int row,int col);
	void setAlignment(QList <Sequence *>);
	void checkLength();
	int  getSeqIndex(QString);
	
	Project *project_;
	
	QList <Sequence *> seq;
	int isSelected;
	int selAnchorRow,selAnchorCol,selDragRow,selDragCol;
	int insertionPoint,draggingSequence,draggedRowNum;
	Sequence *draggedSeq;
	int leftDown,lockModeOn;
	QFont currFont;	
	QStack <TEditRec *> undoStack;
	int nAlignments;
	QString numStr;
	
	int cellWidth_,cellHeight_;
	
};



#endif

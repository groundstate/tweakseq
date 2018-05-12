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

#ifndef __SEQUENCE_EDITOR_H_
#define __SEQUENCE_EDITOR_H_

#include <QWidget> 
#include <QList>
#include <QStack>

class QString;
class QFont;
class QPixmap;  
class QDragEnterEvent;
class QDropEvent;

class Project;
class Sequence;
class SequenceGroup;

class SequenceEditor:public QWidget
{

Q_OBJECT

public:
	
	SequenceEditor(Project *,QWidget *parent);
	~SequenceEditor();
	void setProject(Project *);
	
	QChar cellContent(int,int,int );
	
	void undoEdit();
	void redoEdit();
	
	void cutSelectedResidues();
	void cutSelectedSequences();
	void excludeSelection();
	void removeExcludeSelection();

	QColor getSequenceGroupColour();
	void updateViewport();
	
	const QFont &editorFont(){return font();}
	
	void setReadOnly(bool readOnly){readOnly_=readOnly;}
	bool isReadOnly(){return readOnly_;}
	
	int contentsX();
	int contentsY();
	
	QRect 	cellGeometry ( int row, int column );
	int 	columnAt ( int x ) const;
	void 	ensureCellVisible ( int row, int column );
	QSize 	gridSize () const;
	int 	numCols () const;
	int 	numRows () const;
	void 	repaintCell ( int row, int column, bool erase = true );
	int 	rowAt ( int y ) const;
	virtual void 	setCellHeight ( int );
	virtual void 	setCellWidth ( int );
	virtual void 	setNumCols ( int );
	virtual void 	setNumRows ( int );
	void 	updateCell ( int row, int column );

public slots:
	
	void postLoadTidy();
	void setEditorFont(const QFont &);
	void loadingSequences(bool);
	
signals:
	
	void info(const QString &);
	
protected:

	void paintCell( QPainter*, int row, int col );
	void mousePressEvent( QMouseEvent* );
	void mouseReleaseEvent( QMouseEvent* );
	void contentsMouseMoveEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	
	void keyPressEvent( QKeyEvent* );
	void focusInEvent( QFocusEvent* );
	void focusOutEvent( QFocusEvent* );

private slots:
	
	void sequenceAdded(Sequence *);
	void sequencesCleared();
	
private:
	
	void init();
	void connectSignals();
	void disconnectSignals();
	
	void setCellMark(int row,int col,int on);

	void checkLength();
	
	int indexFirstVisibleSequenceInGroup(SequenceGroup *);
	int indexLastVisibleSequenceInGroup(SequenceGroup *);
	int indexVisibleSequence(Sequence *);
	
	Project *project_;
	bool readOnly_;
	bool loadingSequences_;
	
	bool selectingResidues_;
	int selAnchorRow,selAnchorCol,selDragRow,selDragCol;
	int insertionPoint,draggingSequence,draggedRowNum;
	Sequence *draggedSeq;
	
	bool selectingSequences_;
	int  seqSelectionAnchor_,seqSelectionDrag_;
	
	bool leftDown_;
	int nAlignments;
	QString numStr;
	
	int cellWidth_,cellHeight_;
	
	int currGroupColour_;
	
	QString lastInfo;
	
};



#endif

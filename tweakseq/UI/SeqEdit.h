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

class QString;
class QFont;
class QPixmap;  
class QDragEnterEvent;
class QDropEvent;

class Project;
class Sequence;
class SequenceGroup;

class SeqEdit:public Q3GridView
{

Q_OBJECT

public:
	
	SeqEdit(Project *,QWidget *parent);
	~SeqEdit();
	void setProject(Project *);
	
	QChar cellContent(int,int,int );
	
	void undoEdit();
	void redoEdit();
	
	void cutSelection();
	void excludeSelection();
	void removeExcludeSelection();

	QColor getSequenceGroupColour();
	void updateViewport();
	
	const QFont &editorFont(){return font();}
	
	void setReadOnly(bool readOnly){readOnly_=readOnly;}
	bool isReadOnly(){return readOnly_;}
	
public slots:
	
	void postLoadTidy();
	void setEditorFont(const QFont &);
	
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
	
	void insertCell(char c,int row,int col);
	void insertCells(QString,int row,int col);
	void setCellMark(int row,int col,int on);

	void checkLength();
	
	int indexFirstinGroup(SequenceGroup *);
	int indexLastinGroup(SequenceGroup *);
	
	Project *project_;
	bool readOnly_;
	
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

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

#ifndef __SEQUENCE_EDITOR_H_
#define __SEQUENCE_EDITOR_H_


#include <QSharedPointer>
#include <QTimer>
#include <QWidget>

#include "Sequence.h"

class QDomDocument;
class QDomElement;
class QFont;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QWheelEvent;

class Project;
class Sequence;
class SequenceGroup;

class SequenceEditor: public QWidget
{
	Q_OBJECT
	
	public:
		
		SequenceEditor(Project*,QWidget *);
		void setProject(Project *);
		
		void writeSettings(QDomDocument &,QDomElement &);
		void readSettings(QDomDocument &);
		
		void setReadOnly(bool);
		bool isReadOnly(){return readOnly_;}
		
		const QFont &editorFont(){return font();}
	
		void setResidueView(int);
		int  residueView(){return residueView_;}
		
		void setColourMap(int);
		int  colourMap(){return colourMap_;}
		
		QColor getNextGroupColour();
		
		void cutSelectedResidues();
		void cutSelectedSequences();
		void excludeSelection();
		void removeExcludeSelection();
	
		bool isBookmarked(Sequence *);
		
		void updateViewport();
		void visibleRows(int *,int *);
	
		enum ResidueView {StandardView, InvertedView,  SolidView};
		enum ColourMap   {PhysicoChemicalMap,RasMolMap,TaylorMap};
		
	signals:
	
		void info(const QString &);
		void viewExtentsChanged(int,int,int,int,int,int);
	
	public slots:
	
		void sequencesCleared();

		void postLoadTidy();
		void enableUpdates(bool);
		void setEditorFont(const QFont &); // this is a slot so that QFontDialog can be used for interactive preview
	
		void setFirstVisibleRow(int);
		void setFirstVisibleColumn(int);
		
		void createBookmark();
		void createBookmark(Sequence *);
		void removeBookmark();
		void removeBookmark(Sequence *);
		void moveToNextBookmark();
		void moveToPreviousBookmark();
		
		void selectSequence(const QString &);
		
	protected:
	
		void resizeEvent(QResizeEvent * event);
		
		void paintEvent(QPaintEvent *);
			
		void mousePressEvent( QMouseEvent* );
		void mouseReleaseEvent( QMouseEvent* );
		void mouseMoveEvent(QMouseEvent *);
		void mouseDoubleClickEvent(QMouseEvent *);
		
		void wheelEvent(QWheelEvent *);
	
		void keyPressEvent( QKeyEvent* );
	
	private slots:
		
		void scrollRow();
		void scrollCol();
		
	private:
	
		void init();

		void buildBookmarks();
		void sortBookmarks();
		
		void updateViewExtents();
		QChar cellContent(int, int, int,Sequence *currSeq );
		void setCellFlag(int row,int col,bool exclude);
		
		void paintCell( QPainter* p, int, int,Sequence *currSeq );
		void paintRow(QPainter *p,int);
		void paintHeader(QPainter *p);
		
		int rowAt(int);
		int columnAt(int);
		
		int rowFirstVisibleSequenceInGroup(SequenceGroup *);
		int rowLastVisibleSequenceInGroup(SequenceGroup *);
		int rowVisibleSequence(Sequence *seq);
		
		void connectToProject();
		void disconnectFromProject();
		void cleanupTimer();
		
		// data
		Project *project_;
		QList<Sequence *> bookmarks_; // bookmarks move with a sequence FIXME shared pointer
		int currBookmark_;
		
		// properties
		bool readOnly_;
		int  residueView_;
		int  colourMap_;
		
		// geometry
		int numRows_,numCols_;
		double rowPadding_,columnPadding_;
		int rowHeight_,colWidth_,charWidth_;
		int flagsWidth_,labelWidth_;
		int headerHeight_;
		
		int firstVisibleRow_,lastVisibleRow_;
		int firstVisibleCol_,lastVisibleCol_;
		
		int currGroupColour_;
		
		QString lastInfo_;
		
		bool enableUpdates_;
		
		// mouse state
		bool selectingSequences_,selectingResidues_;
		int  selAnchorRow_,selAnchorCol_,selDragRow_,selDragCol_;
		int  seqSelectionAnchor_,seqSelectionDrag_;
		bool leftDown_;
		QTimer scrollRowTimer_,scrollColTimer_;
		int scrollRowIncrement_,scrollColIncrement_; // +/-1
		int baseTimeout_, currentTimeout_;
		
		int totalWheelRotation_;
		
		QString numStr_; 
};
#endif

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
class SearchResult;
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
		
		void setSequenceDataType(int);
		int sequenceDataType(){return sequenceDataType_;}
		
		QColor getNextGroupColour();
		
		bool isBookmarked(Sequence *);
		
		void setSearchResults(QList<SearchResult *> &);
		
		
		void updateViewport();
		void visibleRows(int *,int *);
	
		enum ResidueView {StandardView, InvertedView,  SolidView};
		enum AminoColourMap   {PhysicoChemicalMap,RasMolMap,TaylorMap,MonoMap};
		enum DNAColourMap {StandardDNAMap,MonoDNAMap};
		
	signals:
	
		void info(const QString &);
		void viewExtentsChanged(int,int,int,int,int,int);
		void statusMessage(const QString &, int);
		void edited();
		
	public slots:
		
		void undo();
		void redo();
		void cutSelection();
		void pasteClipboard();
		void moveSelection(int);
		
		void groupSequences();
		void ungroupSequences();
		void ungroupAllSequences();
		void lockSelectedGroups();
		void unlockSelectedGroups();
		void hideNonSelectedGroupMembers();
		void unhideAllGroupMembers();
		void unhideAll();
	
		void excludeSelectedResidues();
		void removeExclusions();
		
		void sequencesCleared();

		void postLoadTidy();
		void enableUpdates(bool);
		void setEditorFont(const QFont &); // this is a slot so that QFontDialog can be used for interactive preview
	
		void setFirstVisibleRow(int);
		void setFirstVisibleColumn(int);
		
		void buildBookmarks();
		void createBookmark();
		void createBookmark(Sequence *);
		void removeBookmark();
		void removeBookmark(Sequence *);
		void moveToNextBookmark();
		void moveToPreviousBookmark();
		
		void goToSearchResult(int);
		void clearSearchResults();
		
		void selectSequence(const QString &); // connected eg to the Find tool
		
	protected:
	
		void resizeEvent(QResizeEvent * event);
		
		void paintEvent(QPaintEvent *);
			
		void mousePressEvent( QMouseEvent* );
		void mouseReleaseEvent( QMouseEvent* );
		void mouseMoveEvent(QMouseEvent *);
		void mouseDoubleClickEvent(QMouseEvent *);
		
		void focusInEvent(QFocusEvent *);
		
		void wheelEvent(QWheelEvent *);
	
		void keyPressEvent( QKeyEvent* );
	
	private slots:
		
		void scrollRow();
		void scrollCol();
		
	private:
	
		enum FocusRegion {ResidueView, SequenceView};
		
		void init();
		
		void updateViewExtents();
		QChar cellContent(int, int, int,Sequence *currSeq );

		void getResidueColour(int ch,QColor &,bool);
		void paintCell( QPainter* p, int, int,Sequence *currSeq );
		void paintRow(QPainter *p,int);
		void paintHeader(QPainter *p);
		void paintConsensusSequence(QPainter *p);
		
		int rowAt(int);
		int columnAt(int);
		
		int rowFirstVisibleSequenceInGroup(SequenceGroup *);
		int rowLastVisibleSequenceInGroup(SequenceGroup *);
		
		int rowVisibleSequence(Sequence *seq);
		int rowFirstVisibleSequence(QList<Sequence *> &);
		int rowLastVisibleSequence(QList<Sequence *> &);
		
		void makeVisible(Sequence *,int startCol=-1,int stopCol=-1);
		
		void connectToProject();
		void disconnectFromProject();
		void cleanupTimer();
		
		QRect dirtyRowsRect(int,int);
		// data
		Project *project_;
		QList<Sequence *> bookmarks_; // bookmarks move with a sequence FIXME shared pointer
		int currBookmark_;
		
		// properties
		bool readOnly_;
		int  sequenceDataType_;
		int  residueView_;
		int  colourMap_;
		int  defaultProteinColourMap_;
		int  defaultDNAColourMap_;
		
		// Search
		QList<SearchResult *> searchResults_;
		SearchResult * currSearchResult_;
		
		// geometry
		int numRows_,numCols_;
		double rowPadding_,columnPadding_;
		int rowHeight_,colWidth_,charWidth_;
		int flagsWidth_,flagsColWidth_,labelWidth_;
		int bookmarkPos_,indexPos_,lockPos_,expanderPos_;
		int headerHeight_,footerHeight_;
		
		int firstVisibleRow_,lastVisibleRow_;
		int firstVisibleCol_,lastVisibleCol_;
		
		int currGroupColour_;
		
		QString lastInfo_;
		
		bool enableUpdates_;
		
		// mouse state
		bool selectingSequences_,selectingResidues_,draggingSequences_;
		int  selAnchorRow_,selAnchorCol_,selDragRow_,selDragCol_;
		int  seqSelectionAnchor_,seqSelectionDrag_;
		bool leftDown_;
		QTimer scrollRowTimer_,scrollColTimer_;
		int scrollRowIncrement_,scrollColIncrement_; // +/-1
		int baseTimeout_, currentTimeout_;
		
		int totalWheelRotation_;
		
		// keyboard state
		int currFocus_;
		
		QString numStr_;
		
		// 
		bool repaintDirtyRows_;
		int  firstDirtyRow_,lastDirtyRow_;
		
};
#endif

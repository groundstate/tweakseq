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

#include <QWidget>

class QFont;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QWheelEvent;

class Project;


class SequenceEditor: public QWidget
{
	Q_OBJECT
	
	public:
		
		SequenceEditor(Project*,QWidget *);
		void setProject(Project *);
		
		void setNumRows(int);
		
		const QFont &editorFont(){return font();}
		
		QColor getSequenceGroupColour();
		
		void setReadOnly(bool);
		bool isReadOnly(){return readOnly_;}
		
		void cutSelectedResidues();
		void cutSelectedSequences();
		void excludeSelection();
		void removeExcludeSelection();
		
		void setRowPadding(double);
		
		void updateViewport();
		void visibleRows(int *,int *);
		
	signals:
		
		void wheelScrolled();
		void info(const QString &);
		void ensureRowVisible(int);
		
	public slots:
		
		void sequencesCleared();

		void postLoadTidy();
		void loadingSequences(bool);
		void setEditorFont(const QFont &); // this is a slot so that QFontDialog can be used for interactive preview
		
	protected:
	
		void paintEvent(QPaintEvent *);
			
		void mousePressEvent( QMouseEvent* );
		void mouseReleaseEvent( QMouseEvent* );
		void mouseMoveEvent(QMouseEvent *);
		void mouseDoubleClickEvent(QMouseEvent *);
		
		void wheelEvent(QWheelEvent *);
		
		void keyPressEvent( QKeyEvent* );
	
	private:
	
		void init();
		
		void connectSignals();
		void disconnectSignals();
		
		void paintRow(QPainter *p,int row);
		
		int rowAt(int);
		int columnAt(int);
		
		Project *project_;
		
		bool readOnly_;
		
		int numRows_,numCols_;
		double rowPadding_,columnPadding_;
		int rowHeight_,columnWidth_;
		int flagsWidth_,labelWidth_;
		
		int currGroupColour_;
		
		QString lastInfo_;
		
		bool loadingSequences_;
		bool selectingSequences_;
		int  selAnchorRow_,selAnchorCol_,selDragRow_,selDragCol_;
		int  seqSelectionAnchor_,seqSelectionDrag_;
		bool leftDown_;
};
#endif

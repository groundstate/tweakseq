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

#ifndef __SEQ_EDITOR_H_
#define __SEQ_EDITOR_H_

#include <QWidget>

class QResizeEvent;
class QScrollArea;
class QScrollBar;
class QSplitter;

class Project;
class Sequence;
class SeqInfoView;
class SeqResidueView;

class SeqEditor:public QWidget
{
	Q_OBJECT
	
	public:
	
		SeqEditor(Project *,QWidget *parent);
		~SeqEditor();
		void setProject(Project *);
	
		void setReadOnly(bool readOnly);
		bool isReadOnly(){return readOnly_;}
		
		const QFont &editorFont(){return font();}
		
		QColor getSequenceGroupColour();
		
		void cutSelectedResidues();
		void cutSelectedSequences();
		void excludeSelection();
		void removeExcludeSelection();

		void updateViewport();
	
	public slots:
	
		void postLoadTidy();
		void loadingSequences(bool);
		void setEditorFont(const QFont &); // this is a slot so that QFontDialog can be used for interactive preview
	
	signals:
	
		void info(const QString &);
		
	protected:
	
		virtual void resizeEvent(QResizeEvent *);
		
	private slots:
		
		void horizSliderMoved(int);
		void vertSliderMoved(int);
		
		void wheelScrolled();
		void ensureRowVisible(int);
		
		void sequenceAdded(Sequence *);
		void sequencesCleared();
	
		void postInfo(const QString &);
		
	private:
	
		void init();
		void connectSignals();
		void disconnectSignals();
		void updateScrollBars();
		
		QSplitter *splitter_;
		QScrollBar *hscroller_,*vscroller_;
		QScrollArea *seqInfoScrollArea_;
		SeqInfoView *seqInfoView_;
		SeqResidueView *seqResidueView_;

		Project *project_;
		bool readOnly_;
		bool loadingSequences_;
		
		int numRows_,numCols_;
		int rowHeight_,columnWidth_;
		
		int currGroupColour_;
		
};

#endif

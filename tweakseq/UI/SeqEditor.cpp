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

#include <QtDebug>
#include "DebuggingInfo.h"

#include <cmath>

#include <QBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>

#include "Project.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SeqEditor.h"
#include "SeqInfoView.h"
#include "SeqResidueView.h"

#define ROW_PADDING 1.3
#define COLUMN_PADDING 1.3

#define N_GROUP_COLOURS 10

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


//
// Public members
//

SeqEditor::SeqEditor(Project *project,QWidget *parent):QWidget(parent)
{
	qDebug() << trace.header() << "Creating SeqEditor";
	init();
	project_=project;
	
	QBoxLayout *hl = new QBoxLayout(QBoxLayout::LeftToRight,this);
	//hl->setContentsMargins(0,0,0,0);
	
	QBoxLayout *l = new QBoxLayout(QBoxLayout::TopToBottom);
	//l->setContentsMargins(0,0,0,0);
	hl->addLayout(l);
	
	splitter_ = new QSplitter(Qt::Horizontal,this);
	splitter_->setContentsMargins(0,0,0,0);
	
	seqInfoScrollArea_ = new QScrollArea(splitter_);
	seqInfoScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	seqInfoScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	seqInfoScrollArea_->setStyleSheet("background-color:black;");
	
	seqInfoView_ = new SeqInfoView(project_,this);
	seqInfoView_->setRowPadding(ROW_PADDING);
	seqInfoScrollArea_->setWidget(seqInfoView_);
	
	seqResidueView_= new SeqResidueView(project_,splitter_);
	//seqResidueView_->setRowPadding(ROW_PADDING);
	
	l->addWidget(splitter_);
	
	hscroller_ = new QScrollBar(Qt::Horizontal,this);
	hscroller_->setSingleStep(1);
	l->addWidget(hscroller_);
	connect(hscroller_,SIGNAL(valueChanged(int)),this,SLOT(horizSliderMoved(int)));
	
	vscroller_ = new QScrollBar(Qt::Vertical,this);
	vscroller_->setSingleStep(1); // units of the slider are 'rows'
	
	hl->addWidget(vscroller_);
	connect(vscroller_,SIGNAL(valueChanged(int)),this,SLOT(vertSliderMoved(int)));
	
	resize(400,600);
	
	connect(seqInfoView_,SIGNAL(wheelScrolled()),this,SLOT(wheelScrolled()));
	connect(seqInfoView_,SIGNAL(ensureRowVisible(int)),this,SLOT(ensureRowVisible(int)));
	connect(seqInfoView_,SIGNAL(info(const QString &)),this,SLOT(postInfo(const QString &)));
	connectSignals();
	
	updateScrollBars();
	
}

SeqEditor::~SeqEditor()
{
}

void SeqEditor::setProject(Project * project)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	disconnectSignals();
	init(); // make sure we are in a clean state
	project_=project;
	seqInfoView_->setProject(project_);
	seqResidueView_->setProject(project_);
	
	connectSignals();
	
	// Set scrollers to initial position
	hscroller_->setValue(0);
	vscroller_->setValue(0);
	
}

void SeqEditor::setReadOnly(bool readOnly)
{
	readOnly_=readOnly;
	seqInfoView_->setReadOnly(readOnly_);
	seqResidueView_->setReadOnly(readOnly_);
}


QColor SeqEditor::getSequenceGroupColour()
{
	currGroupColour_++;
	if (currGroupColour_ == N_GROUP_COLOURS+1)
		currGroupColour_=1;
	return QColor(groupColours[currGroupColour_-1][0],groupColours[currGroupColour_-1][1],groupColours[currGroupColour_-1][2]);
}

void SeqEditor::updateViewport()
{
	numRows_=project_->sequences.visibleSize();
	seqInfoView_->updateViewport();
	seqResidueView_->updateViewport();
	updateScrollBars();
	
}


void SeqEditor::cutSelectedResidues()
{
}

void SeqEditor::cutSelectedSequences()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	project_->cutSelectedSequences();
	updateViewport();
}

void SeqEditor::excludeSelection()
{
}

void SeqEditor::removeExcludeSelection()
{
}

		


//
// Public slots
//

void SeqEditor::postLoadTidy()
{
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

void SeqEditor::loadingSequences(bool loading)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << loading;
	loadingSequences_= loading;
	if (!loadingSequences_){
		updateViewport();
		updateScrollBars();
	}
}
	
void SeqEditor::setEditorFont(const QFont &f)
{
	QFontInfo fi(f);
	QFont ftmp = f;
	if (!fi.exactMatch()){
		ftmp= QFont(fi.family(),fi.pointSize(),fi.weight());
	}
	QFontMetrics fm(ftmp);
	int w = fm.width('W');
	int h = fm.width('W');
	rowHeight_ = (int)(h*ROW_PADDING);
	columnWidth_ = (int)(w*COLUMN_PADDING);
	setFont(ftmp); 

	seqInfoView_->setViewFont(f); // generates a resizEvent(), updating the scrollbars
	seqResidueView_->setViewFont(f);
}

void SeqEditor::resizeEvent(QResizeEvent *)
{
	updateScrollBars();
}


//
// Private slots
//

void  SeqEditor::horizSliderMoved(int)
{
}

void  SeqEditor::vertSliderMoved(int value)
{
	QScrollBar *sb = seqInfoScrollArea_->verticalScrollBar();
	sb->setValue(value);
}

void SeqEditor::wheelScrolled()
{
	updateScrollBars();
}

void SeqEditor::ensureRowVisible(int row)
{
	QScrollBar *sb = seqInfoScrollArea_->verticalScrollBar();
	qDebug() << seqInfoView_->visibleRegion().boundingRect();
	int startRow,stopRow;
	seqInfoView_->visibleRows(&startRow,&stopRow);
	if (row<startRow){
		int newval = rowHeight_*rint((sb->value()-rowHeight_)/((double) rowHeight_));
		sb->setValue(newval);
		updateScrollBars();
	}
	else if (row > stopRow){
		int newval = rowHeight_*rint((sb->value()+rowHeight_)/((double) rowHeight_));
		sb->setValue(newval);
		updateScrollBars();
	}
}

void SeqEditor::sequenceAdded(Sequence *s)
{

	int sequenceLength = s->residues.length();
	
	if (s->visible){
		numRows_=numRows_+1;
		seqInfoView_->setNumRows(numRows_);
	}
	
	if (!loadingSequences_){
		updateScrollBars();
	}
}

void SeqEditor::sequencesCleared()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	numRows_=0;
	seqInfoView_->sequencesCleared();
	seqResidueView_->sequencesCleared();
	updateScrollBars();
}

void SeqEditor::postInfo(const QString & msg)
{
	emit info(msg);
}
		
//
// Private members
//

void SeqEditor::init()
{
	
	project_=NULL;
	readOnly_=false;
	
	loadingSequences_=false;
	
	numRows_= 0;
	numCols_= 0; // columns in the Residue View
	
	rowHeight_=16;
	columnWidth_=16;
	
	currGroupColour_=0;
}

void SeqEditor::connectSignals()
{
	connect(&(project_->sequences),SIGNAL(sequenceAdded(Sequence *)),this,SLOT(sequenceAdded(Sequence *)));
	connect(&(project_->sequences),SIGNAL(cleared()),this,SLOT(sequencesCleared()));
	connect(project_,SIGNAL(loadingSequences(bool)),this,SLOT(loadingSequences(bool)));
}

void SeqEditor::disconnectSignals()
{
	disconnect(&(project_->sequences),SIGNAL(sequenceAdded(Sequence *)),this,SLOT(sequenceAdded(Sequence *)));
	disconnect(&(project_->sequences),SIGNAL(cleared()),this,SLOT(sequencesCleared()));
	disconnect(project_,SIGNAL(loadingSequences(bool)),this,SLOT(loadingSequences(bool)));
}

void SeqEditor::updateScrollBars()
{
	QScrollBar *sb = seqInfoScrollArea_->verticalScrollBar();
	sb->setSingleStep(rowHeight_);
	//qDebug() << trace.header(__PRETTY_FUNCTION__) << sb->minimum() << " " << sb->maximum() << " " << sb->pageStep() << " " << sb->singleStep();	
	//qDebug() << trace.header(__PRETTY_FUNCTION__) << seqInfoView_->height();
	vscroller_->setMinimum(sb->minimum());
	vscroller_->setMaximum(sb->maximum());
	vscroller_->setSingleStep(sb->singleStep());
	vscroller_->setPageStep(sb->pageStep());
	vscroller_->setValue(sb->value());
}

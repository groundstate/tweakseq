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

#include <QBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>

#include "Project.h"
#include "Sequence.h"
#include "SeqEditor.h"
#include "SeqInfoView.h"
#include "SeqResidueView.h"

#define ROW_PADDING 1.3

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



QColor SeqEditor::getSequenceGroupColour()
{
	return QColor();
}

void SeqEditor::updateViewport()
{
	vscroller_->setRange(0,numRows_-1);
	vscroller_->setPageStep((int) (splitter_->height()/rowHeight_));
	seqInfoView_->updateViewport();
	qDebug() << seqInfoScrollArea_->verticalScrollBar()->maximum() << " " << seqInfoScrollArea_->verticalScrollBar()->pageStep();
}


void SeqEditor::cutSelectedResidues()
{
}

void SeqEditor::cutSelectedSequences()
{
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
	setFont(ftmp); 

	seqInfoView_->setViewFont(f);
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
	vscroller_->setRange(0,0);
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
	//qDebug() << trace.header(__PRETTY_FUNCTION__) << sb->minimum() << " " << sb->maximum() << " " << sb->pageStep() << " " << sb->singleStep();	
	//qDebug() << trace.header(__PRETTY_FUNCTION__) << seqInfoView_->height();
	vscroller_->setMinimum(sb->minimum());
	vscroller_->setMaximum(sb->maximum());
	vscroller_->setSingleStep(sb->singleStep());
	vscroller_->setPageStep(sb->pageStep());
	vscroller_->setValue(sb->value());
}

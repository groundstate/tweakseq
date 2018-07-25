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

#include <QFont>
#include <QLabel>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTime>

#include "AminoAcids.h"
#include "Application.h"
#include "MoveCmd.h"
#include "DNA.h"
#include "Project.h"
#include "ResidueSelection.h"
#include "SearchResult.h"
#include "Sequence.h"
#include "Sequences.h"
#include "SequenceGroup.h"
#include "SequenceEditor.h"
#include "SequenceFile.h"
#include "SequenceSelection.h"
#include "Utility.h"
#include "XMLHelper.h"

#define INDEX_WIDTH 3
#define LABEL_WIDTH 16
#define HEADER_HEIGHT 2
#define FOOTER_HEIGHT 8
#define MIN_FLAGS_COL_WIDTH 18 

#define N_GROUP_COLOURS 10

// Colours chosen for maximum  contrast, from Kenneth Kelly's sequence
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


QPixmap *lockpm=NULL;
QPixmap *bookmarkpm=NULL;

//
// Public members
//

SequenceEditor::SequenceEditor(Project *project,QWidget *parent): QWidget(parent)
{
	setContentsMargins(0,0,0,0);
	setFocusPolicy(Qt::StrongFocus);
	
	if (lockpm == NULL)
		lockpm = new QPixmap(":/images/lock.png");
	if (bookmarkpm == NULL)
		bookmarkpm = new QPixmap(":/images/bookmark.png");
	
	init();

	project_=project;
	
	setMouseTracking(true);
	
	setMinimumSize(400,600);
	
	connectToProject();
	
}

void SequenceEditor::setProject(Project *project)
{
	disconnectFromProject();
	
	init();
	project_=project;
	sequenceDataType_=project_->sequenceDataType();
	connectToProject();
	
}

void SequenceEditor::writeSettings(QDomDocument &doc,QDomElement &parentElem)
{
	QDomElement pelem = doc.createElement("sequence_editor_ui");
	parentElem.appendChild(pelem);
	XMLHelper::addElement(doc,pelem,"font",font().toString());
	
	QString view;
	switch (residueView_)
	{
		case StandardView:view="standard";break;
		case InvertedView:view="inverted";break;
		case SolidView   :view="solid";break;
	}
	XMLHelper::addElement(doc,pelem,"view",view);
	
	QDomElement celem = doc.createElement("colourmaps");
	pelem.appendChild(celem);
	
	QString colMap;
	if (sequenceDataType_==SequenceFile::Proteins){
		switch (colourMap_)
		{
			case PhysicoChemicalMap:colMap="physico-chemical";break;
			case RasMolMap:colMap="rasmol";break;
			case TaylorMap:colMap="taylor";break;
			case MonoMap:colMap="mono";break;
		}
		XMLHelper::addElement(doc,celem,"proteins",colMap);
		
		switch (defaultDNAColourMap_)// this is for "defaults.xml"
		{
			case StandardDNAMap:colMap="standard";break;
			case MonoDNAMap:colMap="standard";break;
		}
		XMLHelper::addElement(doc,celem,"dna",colMap); 
	}
	else if (sequenceDataType_==SequenceFile::DNA){
		switch (colourMap_)
		{
			case StandardDNAMap:colMap="standard";break;
			case MonoDNAMap:colMap="mono";break;
		}
		XMLHelper::addElement(doc,celem,"dna",colMap);
		switch (defaultProteinColourMap_) // this is for "defaults.xml"
		{
			case PhysicoChemicalMap:colMap="physico-chemical";break;
			case RasMolMap:colMap="rasmol";break;
			case TaylorMap:colMap="taylor";break;
			case MonoMap:colMap="mono";break;
		}
		XMLHelper::addElement(doc,celem,"proteins",colMap);
	}
}

void SequenceEditor::readSettings(QDomDocument &doc)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	QDomNodeList nl = doc.elementsByTagName("sequence_editor_ui");
	if (nl.count() == 1){
		QDomNode gNode = nl.item(0);
		QDomElement elem = gNode.firstChildElement();
		QFont editorFont = font();
		while (!elem.isNull()){
			if (elem.tagName() == "font"){
				editorFont.fromString(elem.text());
			}
			else if (elem.tagName() == "view"){
				if (elem.text()=="standard")
					residueView_=StandardView;
				else if (elem.text()=="inverted")
					residueView_=InvertedView;
				else if (elem.text()=="solid")
					residueView_=SolidView;
			}
			else if (elem.tagName() == "colourmaps"){
				QDomElement celem = elem.firstChildElement();
				while ((!celem.isNull())){
					if (celem.tagName() == "proteins"){
						if (celem.text()=="physico-chemical")
							defaultProteinColourMap_=PhysicoChemicalMap;
						else if (celem.text()=="rasmol")
							defaultProteinColourMap_ = RasMolMap;
						else if (celem.text()=="taylor")
							defaultProteinColourMap_=TaylorMap;
						else if (celem.text()=="mono")
							defaultProteinColourMap_=MonoMap;
					}
					else if (celem.tagName() == "dna"){
						if (celem.text()=="standard"){
							defaultDNAColourMap_=StandardDNAMap;
						}
						else if (celem.text()=="mono"){
							defaultDNAColourMap_=MonoDNAMap;
						}
					}
					celem = celem.nextSiblingElement();
				}
			}
			elem=elem.nextSiblingElement();
		}
		setEditorFont(editorFont);
	}
	


}
		
void SequenceEditor::setEditorFont(const QFont &f)
{
	setFont(f); // presumption is that the requested font is available
	QFontMetrics fm(f);
	int h = fm.width('W'); // a wide character
	int w = h;
	rowHeight_= (int) h*rowPadding_;
	colWidth_= (int) w*columnPadding_;
	charWidth_ = w;
	
	flagsColWidth_=charWidth_;
	if (flagsColWidth_ < MIN_FLAGS_COL_WIDTH)
		flagsColWidth_ = MIN_FLAGS_COL_WIDTH;
	
	bookmarkPos_= 0;
	indexPos_ = flagsColWidth_;
	lockPos_= indexPos_+ INDEX_WIDTH*charWidth_;
	expanderPos_=lockPos_+flagsColWidth_;
	
	flagsWidth_=expanderPos_+flagsColWidth_;
	labelWidth_= charWidth_*LABEL_WIDTH;
	headerHeight_=rowHeight_*HEADER_HEIGHT;
	footerHeight_=rowHeight_*FOOTER_HEIGHT;
	
	updateViewExtents();
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	
	repaint();
}

void SequenceEditor::setReadOnly(bool readOnly)
{
	readOnly_=readOnly;
}



QColor SequenceEditor::getNextGroupColour()
{
	currGroupColour_++;
	if (currGroupColour_ == N_GROUP_COLOURS+1)
		currGroupColour_=1;
	return QColor(groupColours[currGroupColour_-1][0],groupColours[currGroupColour_-1][1],groupColours[currGroupColour_-1][2]);
}


void SequenceEditor::setResidueView(int rv)
{
	residueView_=rv;
	repaint();
}

void SequenceEditor::setColourMap(int colourMap)
{
	colourMap_=colourMap;
	repaint();
}

// Normally only called once, when data is first imported into a new Project
void SequenceEditor::setSequenceDataType(int dataType)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "data = " << ((dataType==0)?"proteins":"DNA");
	sequenceDataType_=dataType;
	if (sequenceDataType_ == SequenceFile::Proteins)
		colourMap_=defaultProteinColourMap_;
	else if (sequenceDataType_ == SequenceFile::DNA)
		colourMap_=defaultDNAColourMap_;
}

void SequenceEditor::updateViewport()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	updateViewExtents();
	repaint();
}


bool SequenceEditor::isBookmarked(Sequence *seq)
{
	return bookmarks_.contains(seq);
}

void SequenceEditor::setSearchResults(QList<SearchResult *> &results)
{
	searchResults_=results;
	// Make all results viisble
	for (int sr=0;sr < searchResults_.size();sr++)
		searchResults_.at(sr)->sequence->visible=true;
}


void SequenceEditor::visibleRows(int *start,int *stop)
{
	*start = firstVisibleRow_;
	*stop  = lastVisibleRow_;
}

void SequenceEditor::selectSequence(const QString &label)
{
	
	Sequence *seq = project_->sequences.getSequence(label);
	if (seq){
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "found " <<seq->label;
		project_->sequenceSelection->set(seq);
		makeVisible(seq);
	}
}

//
// Public slots
//

void SequenceEditor::undo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	project_->undo();
	buildBookmarks(); 
	updateViewport();
	emit edited();  // signal must be emitted AFTER all editing is done
}

void SequenceEditor::redo()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	project_->redo();
	buildBookmarks(); 
	updateViewport();
	emit edited();
}

void SequenceEditor::cutSelection()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	if (project_->residueSelection->isInsertionsOnly()){
		project_->cutSelectedResidues();
	}
	else if (!project_->sequenceSelection->empty()){
		project_->cutSelectedSequences();
		buildBookmarks();
	}
	updateViewport();
	emit edited();
}

void SequenceEditor::pasteClipboard()
{
	// FIXME need to be able to paste into an empty project ie nothing to select
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "clipboard contains : "<< app->clipboard().sequences().size();
	project_->pasteClipboard(project_->sequenceSelection->itemAt(0));
	buildBookmarks();
	updateViewport();
	app->clipboard().clear();
	emit edited();
}

void SequenceEditor::moveSelection(int delta)
{
	// Find the first and last sequence in the selection
	
	Sequences &allseqs = project_->sequences;
	int first = allseqs.size()-1; // it has to be <= to this
	int last = 0; // it has to be >= to this
	int nAllSeqs=allseqs.size();
	for (int s=0;s<nAllSeqs;s++){
		Sequence *seq = allseqs.sequences().at(s);
		if (project_->sequenceSelection->contains(seq)){
			if (seq->visible){
				int row = allseqs.visibleIndex(seq);
				if (row < first) first=row;
				if (row > last)  last = row;
			}
		}
	}
	
	// Clamp delta 
	if (delta > 0){
		if (last + delta >= nAllSeqs){
			delta = nAllSeqs- 1 - last;
			if (delta == 0) return;
		}
	}
	else if (delta < 0){
		if (first + delta < 0){
			delta = -first;
			if (delta == 0) return;
		}
	}
	else // delta == 0
		return;
	
	project_->undoStack().push(new MoveCmd(project_,project_->sequenceSelection->sequences(),delta,"move sequences"));
	emit edited();
}

void SequenceEditor::groupSequences()
{
	// Groups the current selection of sequences
	if (!project_->groupSelectedSequences(getNextGroupColour())){
		emit statusMessage("Grouping unsuccessful",0);
		// FIXME the group colour should be released
	}
	repaint();
}

void SequenceEditor::ungroupSequences()
{
	project_->ungroupSelectedSequences();
	updateViewport(); // number of rows may have changed because of unhiding
}

void SequenceEditor::ungroupAllSequences()
{
	project_->ungroupAllSequences();
	updateViewport();
}

void SequenceEditor::lockSelectedGroups()
{
	project_->lockSelectedGroups(true);
	repaint();
}

void SequenceEditor::unlockSelectedGroups()
{
	project_->lockSelectedGroups(false);
	repaint();
}

void SequenceEditor::hideNonSelectedGroupMembers()
{
	project_->hideNonSelectedGroupMembers();
	updateViewport();
}

void SequenceEditor::unhideAllGroupMembers()
{
	project_->unhideAllGroupMembers();
	updateViewport();
}

void SequenceEditor::unhideAll()
{
	project_->sequences.unhideAll();
	updateViewport();
}

void SequenceEditor::excludeSelectedResidues()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
			
	if (selectingResidues_){ // flag selected residues as being excluded from the
		selectingResidues_=false; // alignment
		project_->excludeSelectedResidues(true);
		emit edited();
		repaint();	
	}
}

void SequenceEditor::removeExclusions()
{
	// Selected cells that are marked are unmarked
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	if (selectingResidues_){ // flag selected residues as being excluded from the
		selectingResidues_=false; // alignment
		project_->excludeSelectedResidues(false);
		emit edited();
		repaint();	
	}
}

void SequenceEditor::sequencesCleared()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	numRows_=0;
	numCols_=0;
	bookmarks_.clear();
	currBookmark_=-1;
}

void SequenceEditor::postLoadTidy()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// Project data type is now known
	sequenceDataType_ = project_->sequenceDataType();
	
	if (sequenceDataType_ ==SequenceFile::Proteins)
		colourMap_=defaultProteinColourMap_;
	else if (sequenceDataType_ ==SequenceFile::DNA)
		colourMap_=defaultDNAColourMap_;
	
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
	
	buildBookmarks();
	
}

void SequenceEditor::enableUpdates(bool enable)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << enable;
	enableUpdates_= enable;
	if (enableUpdates_){ // refresh everything
		buildBookmarks(); 
		updateViewport();
		emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	}
}

// This is connected to the vertical scrollbar
void SequenceEditor::setFirstVisibleRow(int val)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << val << endl;
	if (val <0) val=0; // the slider can return -1
	firstVisibleRow_=val;
	updateViewExtents();
	repaint();
}

// This is connected to the horizontal scrollbar
void SequenceEditor::setFirstVisibleColumn(int val)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << val << endl;
	if (val <0) val=0; // the slider can return -1
	firstVisibleCol_=val;
	updateViewExtents();
	repaint();
}

void SequenceEditor::buildBookmarks()
{
	// It's quicker to rebuild than to try to add a new bookmark in the right place
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	bookmarks_.clear();
	for (int s=0;s<project_->sequences.size();s++){
		Sequence *seq = project_->sequences.sequences().at(s);
		if (seq->bookmarked)
			bookmarks_.append(seq);
	}
}

void SequenceEditor::createBookmark()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	if (project_->sequenceSelection->size() == 1){
		// bookmarks need to be ordered so that they are traversed sequentially
		project_->sequenceSelection->itemAt(0)->bookmarked=true;
		buildBookmarks();
		repaint();
	}
}

void SequenceEditor::createBookmark(Sequence *seq)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	
	if (!(bookmarks_.contains(seq))){
		seq->bookmarked=true;
		buildBookmarks();
		repaint();
	}
}

void SequenceEditor::removeBookmark()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	if (project_->sequenceSelection->size() == 1){
		// may need to move the pointer to the current bookmark
		int index = bookmarks_.indexOf(project_->sequenceSelection->itemAt(0));
		if (currBookmark_ > index){
			currBookmark_--; // because after removal, we wato stay pointing to the current bookmark
		}
		project_->sequenceSelection->itemAt(0)->bookmarked=false;
		bookmarks_.removeOne(project_->sequenceSelection->itemAt(0));
		if (bookmarks_.isEmpty())
			currBookmark_=-1;
		repaint();
	}
}

void SequenceEditor::removeBookmark(Sequence *seq)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	int index = bookmarks_.indexOf(seq);
	if (index == -1) return; // FIXME warning
	if (currBookmark_ > index){
		currBookmark_--; 
	}
	seq->bookmarked=false;
	bookmarks_.removeOne(project_->sequenceSelection->itemAt(0));
	if (bookmarks_.isEmpty())
		currBookmark_=-1;
	repaint();
}

void SequenceEditor::moveToNextBookmark()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "n=" << bookmarks_.size() << " curr=" << currBookmark_;
	if (bookmarks_.isEmpty()) return;
	currBookmark_++;
	if (currBookmark_>= bookmarks_.size()) // wrap back to beginning
		currBookmark_=0;
	selectSequence(bookmarks_.at(currBookmark_)->label);
}

void SequenceEditor::moveToPreviousBookmark()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	if (bookmarks_.isEmpty()) return;
	currBookmark_--;
	if (currBookmark_< 0) // wrap back to beginning
		currBookmark_= bookmarks_.size() - 1;
	selectSequence(bookmarks_.at(currBookmark_)->label);
	qDebug() << trace.header(__PRETTY_FUNCTION__);
}

void SequenceEditor::goToSearchResult(int pos)
{
	currSearchResult_=searchResults_.at(pos);
	qDebug() << trace.header(__PRETTY_FUNCTION__) << currSearchResult_->sequence->label;
	project_->sequenceSelection->set(currSearchResult_->sequence);
	project_->residueSelection->clear();
	selectingResidues_=false; // may not have changed selection state so this clears any temporary selection
	makeVisible(currSearchResult_->sequence,currSearchResult_->start,currSearchResult_->stop);
}

void SequenceEditor::clearSearchResults()
{
	searchResults_.clear();
	currSearchResult_=NULL;
}

//
// Protected members
//

void SequenceEditor::resizeEvent(QResizeEvent *)
{
	updateViewExtents();
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_); // FIXME
}

void SequenceEditor::paintEvent(QPaintEvent *pev)
{
	//qDebug() << trace.header(__PRETTY_FUNCTION__) << pev->rect();
	QPainter p(this);

	QTime t;
	t.start();
	p.fillRect(pev->rect(),QColor(0,0,0));

	if (project_->sequences.isEmpty()){
		QColor txtColor;
		txtColor.setRgb(228,228,228);
		p.setPen(txtColor);
		QString joke("Choose the form of the destroyer");
		p.drawText(width()/2,height()/2,fontMetrics().width(joke),fontMetrics().height(),Qt::AlignCenter,joke);
		return;
	}
	
	paintHeader(&p);
	int startRow=firstVisibleRow_;
	int stopRow = lastVisibleRow_;
	if (repaintDirtyRows_){
		startRow=firstDirtyRow_;
		stopRow=lastDirtyRow_;
	}
	for (int r=startRow;r<=stopRow;r++){
		paintRow(&p,r);
	}
	repaintDirtyRows_=false;
	qDebug() << trace.header(__PRETTY_FUNCTION__) << t.elapsed() << "ms";
}

void SequenceEditor::mousePressEvent( QMouseEvent *ev )
{
	
	totalWheelRotation_ = 0;
	cleanupTimer();
	
	if (readOnly_) return;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (seq.count() == 0) return;
	
	QPoint clickedPos;
	int clickedRow,clickedCol;
	int startRow,stopRow,startCol,stopCol;
	
	clickedPos = ev->pos();		
	clickedRow=rowAt( clickedPos.y() + contentsRect().y());
	clickedCol=columnAt(clickedPos.x() + contentsRect().x());
	
	// If we clicked outside the editing area ... well ... do nothing
	if ((clickedRow < 0) || (clickedRow > lastVisibleRow_) || clickedCol > lastVisibleCol_){
		switch (ev->button()){
			case Qt::LeftButton:
				break;
			default:break;
		} // end switch (e->button
		return;
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "row=" << clickedRow << " col=" << clickedCol;
	
	// For the moment we do nothing with the flags area
	if (clickedPos.x() < flagsWidth_) return;
	
	// otherwise ...
	
	if (clickedPos.x() < flagsWidth_+labelWidth_){
		Sequence *selSeq =  project_->sequences.visibleAt(clickedRow);
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "selected " << selSeq->label;

		// Remove any residue selection
		selAnchorRow_=selAnchorCol_=selDragRow_=selDragCol_=-1;
		project_->residueSelection->clear(); // FIXME better done with a signal ?
		selectingResidues_=false;	
		selectingSequences_=true;
		currFocus_ = SequenceView;
		switch (ev->button()){
			case Qt::LeftButton:
			{
				switch (ev->modifiers()){
					case Qt::NoModifier:
						// Check the selection: if we clicked on a contiguous selection, then
						// it is likely that the selection is being dragged
						if (project_->sequenceSelection->contains(selSeq)){
							selectingSequences_=false;
							draggingSequences_=true; // well,maybe
							seqSelectionAnchor_=seqSelectionDrag_=clickedRow;
						}
						else{
							project_->sequenceSelection->clear();
							seqSelectionAnchor_=seqSelectionDrag_=clickedRow;
						}
						break;
					case Qt::ShiftModifier:
					{
						// don't clear selection
						// if there is something in the selection, then this defines
						// the current anchor
						// But we need to scrub non-visible sequences from the selection
						QList<Sequence *> visSeqs ;
						for (int s=0;s< project_->sequenceSelection->size();s++){
							Sequence *seq = project_->sequenceSelection->sequences().at(s);
							if (seq->visible) visSeqs.append(seq);
						}
						int firstSeqInSelection = rowFirstVisibleSequence(visSeqs);
						int lastSeqInSelection = rowLastVisibleSequence(visSeqs);
						if (clickedRow <= firstSeqInSelection)
							seqSelectionAnchor_=lastSeqInSelection;
						else if (clickedRow >= lastSeqInSelection)
							seqSelectionAnchor_= firstSeqInSelection;
						else // in between
							seqSelectionAnchor_= firstSeqInSelection;
						seqSelectionDrag_=clickedRow;
						break;
					}
					case Qt::ControlModifier:
						// don't clear selection
						seqSelectionAnchor_=seqSelectionDrag_=clickedRow;
						break;
					default:
						break;
					}
				leftDown_=true; // not set in mouse move event
				break;
				
			}
			case Qt::RightButton:
			{
					// If a context menu is being requested for a non-selected item
					// make it the new selection
					if (!project_->sequenceSelection->contains(selSeq)){
						project_->sequenceSelection->set(selSeq);
					}
					selectingResidues_=false;	
					selectingSequences_=false;
			}
			default:
				break;
		}

		repaint();
		return;
	}
	
	// The user has clicked inside the sequence area so start selecting residues
	project_->sequenceSelection->clear(); // clear the current selection
	currFocus_ = ResidueView;
	switch (ev->button()){
		case Qt::LeftButton:
			leftDown_=true;
			if (selectingResidues_){
				// If we have already selected a block of cells
				// then we need to update these viz remove the highlight mark
				selectingResidues_=false; // this will block a redraw of the highlight
				if (selAnchorRow_ > selDragRow_){ 
					startRow=selDragRow_;
					stopRow=selAnchorRow_;
				}
				else{
					startRow=selAnchorRow_;
					stopRow=selDragRow_;
				}
				if (selAnchorCol_ > selDragCol_){ 
					startCol=selDragCol_;
					stopCol=selAnchorCol_;
				}
				else{
					startCol=selAnchorCol_;
					stopCol=selDragCol_;
				}
				//for (row=startRow;row<=stopRow;row++)
					//for (col=startCol;col<=stopCol;col++)
						//updateCell(row,col);
			}
			// A new selection has been made
			selectingResidues_ = true;
			
			selAnchorRow_ = clickedRow;   // map to row; set current cell
			selAnchorCol_ = clickedCol;   // map to col; set current cell
			selDragRow_ = selAnchorRow_;
			selDragCol_ = selAnchorCol_;
			//updateCell(selAnchorRow,selAnchorCol);
			break;
		case Qt::MidButton:
			break;
		case Qt::RightButton:
			break;
		default:break;
	}
	repaint();
		
}

void SequenceEditor::mouseReleaseEvent( QMouseEvent *ev )
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	
	// clean up any events we might be tracking
	totalWheelRotation_ = 0;
	cleanupTimer();
	
	if (readOnly_) return;
	bool controlModifier = (ev->modifiers() == Qt::ControlModifier);
	bool shiftModifier = (ev->modifiers() == Qt::ShiftModifier);
	switch (ev->button()){
		case Qt::LeftButton:
		{
			leftDown_=false; // have finished selection
			if (selectingResidues_){
				//selectingResidues_=false; // somewhat illogically, don't drop the mode because painting depends on it still being true
				qDebug() << trace.header(__PRETTY_FUNCTION__) << selAnchorRow_ << " " << selAnchorCol_ << " " << selDragRow_ << " " << selDragCol_;
				int startRow=selAnchorRow_,stopRow=selDragRow_;
				int startCol=selAnchorCol_,stopCol=selDragCol_;
				if (stopRow < startRow) swap_int(&startRow,&stopRow);
				if (stopCol < startCol) swap_int(&startCol,&stopCol);
				qDebug() << trace.header(__PRETTY_FUNCTION__) << startRow << " " << startCol << " " << stopRow << " " << stopCol;
				QList<ResidueGroup *> resSel;
				// startRow and stopRow are for the visible sequences but we need to loop over the hidden ones too
				startRow = project_->sequences.visibleToActual(startRow);
				stopRow  = project_->sequences.visibleToActual(stopRow);
				for (int r=startRow;r<=stopRow;r++)
					resSel.append(new ResidueGroup(project_->sequences.sequences().at(r),startCol,stopCol));
				project_->residueSelection->set(resSel);
				// DO NOT delete the selection
			}
			else if (draggingSequences_){
				draggingSequences_=false;
				qDebug() << "Hello " << seqSelectionAnchor_ << " " << seqSelectionDrag_;
				if (seqSelectionAnchor_ == seqSelectionDrag_){ // didn't move so presume intent was to select
					Sequence *selSeq=project_->sequences.visibleAt(seqSelectionAnchor_);
					project_->sequenceSelection->set(selSeq);
				}
			}
			else if (selectingSequences_){
				selectingSequences_=false;
			
				int startRow = seqSelectionAnchor_,stopRow=seqSelectionDrag_;
				if (stopRow < startRow) swap_int(&startRow,&stopRow);
				int visStart = startRow,visStop=stopRow;
				startRow = project_->sequences.visibleToActual(startRow);
				stopRow  = project_->sequences.visibleToActual(stopRow);
				
				// Find all groups which are in the raw selection
				QList<SequenceGroup *> groups;
				for (int r=visStart;r<=visStop;r++){
					int rr = project_->sequences.visibleToActual(r);
					SequenceGroup *sg = project_->sequences.sequences().at(rr)->group;
					if (sg){
						if (!(groups.contains(sg)))
							groups.append(sg);
					}
				}
				
				// If all visible members of a group are in the selection, select the whole group
				bool cleared = false;
				QList<Sequence*> groupedSequences;
				for (int g=0;g<groups.size();g++){
					SequenceGroup *sg = groups.at(g);
					int s;
					for (s=0;s<sg->size();s++){
						Sequence *seq = sg->itemAt(s);
						if (seq->visible){
							int index = project_->sequences.visibleIndex(seq);
							qDebug() << trace.header(__PRETTY_FUNCTION__) << "checking group:" << seq->label << " visible at " << index;
							if (index < visStart || index > visStop) break; // not in selection, so bail out
						}
					}
					// At least one sequence must have been visible and tested so we can't have got here
					// without one test at least
					if (s==sg->size()){
						qDebug() << trace.header(__PRETTY_FUNCTION__) << "group selected";
						if (!cleared && !controlModifier &&!shiftModifier){
							project_->sequenceSelection->clear();
							cleared=true;
						}
						for (s=0;s<sg->size();s++){
							if (controlModifier ){
								project_->sequenceSelection->toggle(sg->itemAt(s));
								groupedSequences.append(sg->itemAt(s));
							}
							else if (shiftModifier){
								project_->sequenceSelection->add(sg->itemAt(s));
								//groupedSequences.append(sg->itemAt(s));
							}
							else
								project_->sequenceSelection->add(sg->itemAt(s));
						}
					}
				}
				// Hidden members are part of groups and will have been added to the selection
				// in the previous step.
				// Select only the visible members in this step )
				if (!cleared && !controlModifier &&!shiftModifier){
					project_->sequenceSelection->clear();
					cleared=true;
				}
				for (int r=startRow;r<=stopRow;r++){
					if (project_->sequences.sequences().at(r)->visible){
						if (controlModifier){
							// don't toggle any grouped sequences - already done
							if (!(groupedSequences.contains(project_->sequences.sequences().at(r))))
								project_->sequenceSelection->toggle(project_->sequences.sequences().at(r));
						}
						else if (shiftModifier){
							project_->sequenceSelection->add(project_->sequences.sequences().at(r));
						}
						else{
							// add()ing again is OK - this will result in no add due to the check in add()
							project_->sequenceSelection->add(project_->sequences.sequences().at(r));
						}
					}
				}
			}
			break;
		}
		default:break;
	}
	repaint();
}

void SequenceEditor::mouseMoveEvent(QMouseEvent *ev)
{

	QPoint pos;
	int row,currRow,currCol;
	//int startRow,stopRow;
	//int startCol,stopCol;
	int clickedRow,clickedCol;

	totalWheelRotation_ = 0;
	
	if (numRows_== 0) return; // so we don't have to guard against null pointers
	
	pos = ev->pos();              		
	clickedRow=rowAt( pos.y() + contentsRect().y());
	clickedCol=columnAt(pos.x() + contentsRect().x());
	
	// clamp to bounds
	if (clickedRow < 0){
		//if (pos.y() >=0) // clamp to bottom // FIXME looks like Qt3 cruft
		//	clickedRow = numRows_ -1;
		//else // clamp to top
			clickedRow = 0;
	}
	
	if (clickedRow >= numRows_)
		clickedRow = numRows_ -1;
	
	if (clickedCol < 0)
		clickedCol=0;
	else if (clickedCol >= numCols_)
		clickedCol=numCols_-1;
	
	// show the label of the sequence we are moving over
	Sequence *currSeq = project_->sequences.visibleAt(clickedRow);
	if (currSeq->label != lastInfo_){
		lastInfo_ = currSeq->label;
		emit info(lastInfo_);
	}
	
	if (readOnly_) return;
	
	if ((selectingSequences_ || selectingResidues_ || draggingSequences_) && leftDown_){ // only scroll if we are selecting (and we can't select if the widget is read-only)
		// If we go out of the bounds of the window, then the view is scrolled at a rate proportional to the distance
		// we have moved out of the window
		if (pos.y() > height()-footerHeight_ || pos.y() < headerHeight_){ // scroll up/down
			if (!scrollRowTimer_.isActive()){
				if (pos.y() > height()-footerHeight_)
					scrollRowIncrement_=1;
				else 
					scrollRowIncrement_=-1;
				scrollRowTimer_.start(); 
			}
			else{
				int newTimeout=baseTimeout_;
				if (pos.y() > height()-footerHeight_)
					newTimeout = baseTimeout_/rint(( pos.y() - (height()-footerHeight_))/5); // scrolling is accelerated proportional to displacement
				else if (pos.y() < headerHeight_)
					newTimeout = baseTimeout_/rint( (headerHeight_-pos.y())/5);
				if (newTimeout < 100) newTimeout=100;
				if (newTimeout != currentTimeout_){
					currentTimeout_=newTimeout;
					scrollRowTimer_.start(currentTimeout_);
				}
			}
		}
		else if (selectingResidues_ && ((pos.x() < flagsWidth_+labelWidth_ )|| pos.x() > width())){ // scroll left/right
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "scrollin";
			if (!scrollColTimer_.isActive()){
				if (pos.x() > width())
					scrollColIncrement_=1;
				else
					scrollColIncrement_=-1;
				scrollColTimer_.start(); 
			}
			else{
				int newTimeout=baseTimeout_;
				if (pos.x() > width())
					newTimeout = baseTimeout_/rint(( pos.x() - width())/5); // scrolling is accelerated proportional to displacement
				else if (pos.x() < flagsWidth_+labelWidth_ )
					newTimeout = baseTimeout_/rint( flagsWidth_+labelWidth_-pos.x()/5);
				if (newTimeout < 100) newTimeout=100;
				if (newTimeout != currentTimeout_){
					currentTimeout_=newTimeout;
					scrollColTimer_.start(currentTimeout_);
				}
				selDragRow_=clickedRow;
			}
		}
		else{ // the usual case
			if (selectingSequences_){
				cleanupTimer();
				if (seqSelectionDrag_ != clickedRow){
					currRow=clickedRow;
					firstDirtyRow_ = find_smallest_int(currRow,seqSelectionAnchor_,seqSelectionDrag_);
					lastDirtyRow_  = find_largest_int(currRow,seqSelectionAnchor_,seqSelectionDrag_);
					repaintDirtyRows_=true;
					seqSelectionDrag_=clickedRow;
					repaint(dirtyRowsRect(firstDirtyRow_,lastDirtyRow_));
				}
			}
			else if (draggingSequences_){
				cleanupTimer();
				if (seqSelectionDrag_ != clickedRow){
					// Get the actual row of what we clicked on, so that non-visible sequences are skitruepped
					QList<Sequence *> & sel = project_->sequenceSelection->sequences();
					int firstVis = rowFirstVisibleSequence(sel);
					int lastVis  = rowLastVisibleSequence(sel); 
					int delta = project_->sequences.visibleToActual(clickedRow) - project_->sequences.visibleToActual(seqSelectionDrag_);
					currRow=clickedRow;
					if (delta < 0){
						firstDirtyRow_ =firstVis+delta;
						if (firstDirtyRow_ < 0) firstDirtyRow_=0;
						lastDirtyRow_  =lastVis;
					}
					else{
						firstDirtyRow_ =firstVis;
						lastDirtyRow_  =lastVis+delta; 
						if (lastDirtyRow_ >= numRows_) lastDirtyRow_=numRows_-1;
					}
					repaintDirtyRows_=true;
					moveSelection(delta);
					seqSelectionDrag_=clickedRow;
					repaint(dirtyRowsRect(firstDirtyRow_,lastDirtyRow_));
					//repaint();
				}
			}
			else if (selectingResidues_){
				cleanupTimer();
				if (clickedCol < 0)
					clickedCol = firstVisibleCol_;
				currRow = clickedRow;    // map to row; set current cell
				currCol = clickedCol;    // map to col; set current cell
				// Determine the bounds of the rectangle enclosing both
				// the old highlight box and the new highlight box (so we redraw cells in the old box)
				// Determine the vertical bounds
				firstDirtyRow_ = find_smallest_int(currRow,selAnchorRow_,selDragRow_);
				lastDirtyRow_  = find_largest_int(currRow,selAnchorRow_,selDragRow_);
				selDragRow_=currRow;
				selDragCol_=currCol;
				repaintDirtyRows_=true;
				if (firstDirtyRow_>lastDirtyRow_) swap_int(&firstDirtyRow_,&lastDirtyRow_);
				repaint(dirtyRowsRect(firstDirtyRow_,lastDirtyRow_));
			}
		}
	}	
	
}

void SequenceEditor::mouseDoubleClickEvent(QMouseEvent *ev)
{
	// Double clicking on a group member selects the whole group
	
	if (readOnly_) return;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (seq.count() == 0) return;
	
	QPoint clickedPos = ev->pos();		
	int clickedRow=rowAt( clickedPos.y() + contentsRect().y());

	if (clickedRow >= 0 && clickedPos.x() >= flagsWidth_ && clickedPos.x() <= flagsWidth_ + labelWidth_){
		Sequence *selseq = project_->sequences.visibleAt(clickedRow);
		if (selseq->group){
			project_->sequenceSelection->clear();
			project_->addGroupToSelection(selseq->group);
			repaint();
		}
	}
	
}

void SequenceEditor::focusInEvent(QFocusEvent *ev)
{
	if (ev->reason() == Qt::TabFocusReason){
		qDebug() << "tab";
	}
	QWidget::focusInEvent(ev);
}

void SequenceEditor::wheelEvent(QWheelEvent *ev)
{

	int nvis = lastVisibleRow_ - firstVisibleRow_ + 1;
	totalWheelRotation_ += ev->angleDelta().y();
	int nToScroll = totalWheelRotation_/120;
	totalWheelRotation_ -= 120*nToScroll;
	firstVisibleRow_ -= nToScroll;
	if (firstVisibleRow_< 0)
		firstVisibleRow_= 0;
	if (firstVisibleRow_ + nvis -1 >= numRows_)
		firstVisibleRow_ = numRows_- nvis;
	lastVisibleRow_ = firstVisibleRow_ + nvis -1;
	
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	
	repaint();
}
		
void SequenceEditor::keyPressEvent( QKeyEvent *ev )
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	if (readOnly_) return;
	
	QString l;
	int startRow=selAnchorRow_,stopRow=selDragRow_,
			startCol=selAnchorCol_,stopCol=selDragCol_,row;	

	switch (ev->key()){
		case Qt::Key_0:case Qt::Key_1:case Qt::Key_2:case Qt::Key_3:case Qt::Key_4:
		case Qt::Key_5:case Qt::Key_6:case Qt::Key_7:case Qt::Key_8:case Qt::Key_9:
			if (selectingResidues_){
				if (startCol != stopCol)
					return;
				else{
					//build up a string
					numStr_.append(ev->text());
				}
			}
			break;
		case Qt::Key_Plus:
		{ 
			QFont f = font();
			f.setPointSize(f.pointSize()+1);
			setEditorFont(f);
			break;
		}
		case Qt::Key_Minus:
		{
			QFont f = font();
			int sz= f.pointSize()-1;
			if (sz < 1) sz=1;
			f.setPointSize(sz);
			setEditorFont(f);
			break;
		}
		case Qt::Key_Space: // add insertions
			if (selectingResidues_){
				
				// Check that selection is only one column wide - if not
				// do nothing because the user probably hit the spacebar by mistake
				if (startCol != stopCol)
					return;
				else{
					if (startRow > stopRow) swap_int(&startRow,&stopRow);
			
					QList<SequenceGroup *> sgl = project_->residueSelection->uniqueSequenceGroups();
					qDebug() << trace.header(__PRETTY_FUNCTION__) << "unique groups " << sgl.count(); 

					QList<Sequence *> insSeqs;
					
					// Add all of the sequences in each group to the list of sequences receiving insertions
					int firstVisibleLockedSequence = startRow;
					int lastVisibleLockedSequence= stopRow;

					for (int g=0;g<sgl.count();g++){
						SequenceGroup *sg = sgl.at(g);
						if (sg->locked()){
							for (int s=0;s<sg->size();s++){
								Sequence *seq = sg->itemAt(s);
								insSeqs.append(seq); // this adds non-visible sequences too
								if (seq->visible){
									row = rowVisibleSequence(seq);
									if (row >= 0){
										if (row < firstVisibleLockedSequence) firstVisibleLockedSequence = row;
										if (row > lastVisibleLockedSequence)  lastVisibleLockedSequence= row;
									}
								}
							}
						}
						qDebug() << trace.header(__PRETTY_FUNCTION__) << insSeqs.size() << " seqs to insert in"; 
					}

					// Now add in the rest of the selection to the list of sequences receiving insertions
					for (row=startRow;row<=stopRow;++row){
						Sequence *seq = project_->sequences.visibleAt(row);
						if (!insSeqs.contains(seq))
							insSeqs.append(seq);
					}
					qDebug() << trace.header(__PRETTY_FUNCTION__) << insSeqs.size() << " total seqs to insert in"; 
					
					// Is there a numeric argument to the insertion ?
					if (!(numStr_.isEmpty())){
						stopCol=startCol+numStr_.toInt()-1;
						numStr_=""; // reset for reuse ...
					}
					else
						stopCol=startCol;
					
					bool postInsert = !(ev->modifiers() & Qt::ShiftModifier);
					
					project_->addInsertions(insSeqs,startCol,stopCol,postInsert);
					updateViewExtents();
					if (firstVisibleLockedSequence  < startRow) startRow = firstVisibleLockedSequence;
					if (lastVisibleLockedSequence   > stopRow)   stopRow = lastVisibleLockedSequence;
					//for (row=startRow;row<=stopRow;++row){
					//	for (col=startCol;col< numCols();col++)
					//		updateCell(row,col);
					//}	
					repaint();
					emit edited();
				//emit alignmentChanged();
				} // end of if ... else
			} // end of if
			break;
		case Qt::Key_PageUp:
		{
	
			if (ev->modifiers() & Qt::AltModifier){ // scroll left EXCEL style
				
				scrollColIncrement_ = - (lastVisibleCol_-firstVisibleCol_+1);
				scrollCol();
			}
			else{ // scroll up
				scrollRowIncrement_ = -(lastVisibleRow_-firstVisibleRow_+1);
				scrollRow();
			}
			break;
		}
		case Qt::Key_PageDown:
			if (ev->modifiers() & Qt::AltModifier){ // scroll right
				scrollColIncrement_ =  (lastVisibleCol_-firstVisibleCol_+1);
				scrollCol();
			}
			else{ // scroll down
				scrollRowIncrement_ = (lastVisibleRow_-firstVisibleRow_+1);
				scrollRow();
			}
			break;
		case Qt::Key_Up:
		{
			if (currFocus_== ResidueView && startCol == stopCol && startRow==stopRow){
				selAnchorRow_--;
				if (selAnchorRow_ <0) selAnchorRow_=0;
				selDragRow_=selAnchorRow_;
				if (selAnchorRow_ == firstVisibleRow_-1){
					scrollRowIncrement_=-1;
					scrollRow();
				}
			}
			else if (currFocus_ == SequenceView && (seqSelectionAnchor_==seqSelectionDrag_)){
				if (ev->modifiers() == Qt::ShiftModifier){ // selection is moved up, if it continuous
					
				}
				else{
					seqSelectionAnchor_--;
					if (seqSelectionAnchor_<0) seqSelectionAnchor_=0;
					seqSelectionDrag_=seqSelectionAnchor_;
					project_->sequenceSelection->set(project_->sequences.visibleAt(seqSelectionAnchor_));
					if (seqSelectionAnchor_ == firstVisibleRow_-1){
						scrollRowIncrement_=-1;
						scrollRow();
					}
				}
			}
			repaint();
			break;
		}
		case Qt::Key_Down:
		{
			if (currFocus_== ResidueView && startCol == stopCol && startRow==stopRow){
				selAnchorRow_++;
				if (selAnchorRow_ >=numRows_) selAnchorRow_=numRows_-1;
				selDragRow_=selAnchorRow_;
				if (selAnchorRow_ == lastVisibleRow_+1){
					scrollRowIncrement_=1;
					scrollRow();
				}
			}
			else if (currFocus_ == SequenceView && (seqSelectionAnchor_==seqSelectionDrag_)){
				
				seqSelectionAnchor_++;
				if (seqSelectionAnchor_>=numRows_) seqSelectionAnchor_=numRows_-1;
				seqSelectionDrag_=seqSelectionAnchor_;
				project_->sequenceSelection->set(project_->sequences.visibleAt(seqSelectionAnchor_));
				if (seqSelectionAnchor_ == lastVisibleRow_+1){
					scrollRowIncrement_=1;
					scrollRow();
				}
			}
			repaint();
			break;
		}
		case Qt::Key_Right:
		{
			if (currFocus_== ResidueView && startCol == stopCol && startRow==stopRow){
				selAnchorCol_++;
				if (selAnchorCol_ >=numCols_) selAnchorCol_=numCols_-1;
				selDragCol_=selAnchorCol_;
				if (selAnchorCol_ == lastVisibleCol_+1){
					scrollColIncrement_=1;
					scrollCol();
				}
			}
			repaint();
			break;
		}
		case Qt::Key_Left:
		{
			if (currFocus_== ResidueView && startCol == stopCol && startRow==stopRow){
				selAnchorCol_--;
				if (selAnchorCol_ < 0) selAnchorCol_= 0;
				selDragCol_=selAnchorCol_;
				if (selAnchorCol_ == firstVisibleCol_-1){
					scrollColIncrement_=-1;
					scrollCol();
				}
			}
			repaint();
			break;
		}
		default:
			QWidget::keyPressEvent(ev);
			return;
			break;
	} // end of switch()
	
	
}

//
// Private members
//

void SequenceEditor::scrollRow()
{
	// Scrolls the view by one row
	qDebug() << trace.header(__PRETTY_FUNCTION__) << scrollRowIncrement_;
	int nvis = lastVisibleRow_ - firstVisibleRow_ + 1;
	
	firstVisibleRow_ += scrollRowIncrement_;
	if (firstVisibleRow_< 0)
		firstVisibleRow_= 0;
	if (firstVisibleRow_ + nvis -1 >= numRows_)
		firstVisibleRow_ = numRows_- nvis;
	lastVisibleRow_ = firstVisibleRow_ + nvis -1;
	
	if (leftDown_){ // only use scrollRow in this context but just in case I forget
		if (selectingSequences_){
			if (scrollRowIncrement_ > 0)
				seqSelectionDrag_ = lastVisibleRow_;
			else
				seqSelectionDrag_ = firstVisibleRow_;
		}
		else if (selectingResidues_){
			if (scrollRowIncrement_ > 0)
				selDragRow_ = lastVisibleRow_;
			else
				selDragRow_ = firstVisibleRow_;
		}
		else if (draggingSequences_){
			int newDragPosition;
			if (scrollRowIncrement_ > 0)
				newDragPosition = lastVisibleRow_;
			else
				newDragPosition = firstVisibleRow_;
			if (newDragPosition != seqSelectionDrag_){
				int actualRow = project_->sequences.visibleToActual(newDragPosition);
				moveSelection(actualRow-project_->sequences.visibleToActual(seqSelectionDrag_));
				seqSelectionDrag_=newDragPosition;
			}
		}
	}
	
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	
	repaint();
}

void SequenceEditor::scrollCol()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << scrollColIncrement_;
	
	int nvis = lastVisibleCol_ - firstVisibleCol_ + 1;
	
	firstVisibleCol_ += scrollColIncrement_;
	if (firstVisibleCol_< 0)
		firstVisibleCol_= 0;
	if (firstVisibleCol_ + nvis -1 >= numCols_)
		firstVisibleCol_ = numCols_- nvis;
	lastVisibleCol_ = firstVisibleCol_ + nvis -1;
	
	if (leftDown_){ 
		if (scrollColIncrement_ > 0)
			selDragCol_ = lastVisibleCol_;
		else
			selDragCol_ = firstVisibleCol_;
	}
	
	emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	
	repaint();
}

	
//
// Private members
//

void SequenceEditor::init()
{
	project_=NULL;
	
	readOnly_=false;
	
	defaultProteinColourMap_= SequenceEditor::PhysicoChemicalMap;
	defaultDNAColourMap_ = SequenceEditor::StandardDNAMap;
	
	sequenceDataType_= SequenceFile::Proteins;
	residueView_ = SequenceEditor::StandardView;
	colourMap_ = defaultProteinColourMap_;
	
	currBookmark_=-1;
	
	numRows_=0;
	numCols_=0;
	
	firstVisibleRow_ = 0;
	lastVisibleRow_=1;
	firstVisibleCol_=0;
	lastVisibleCol_=1;
	
	rowPadding_=1.3;
	columnPadding_=1.3;
	
	int h = fontMetrics().width('W'); // a wide character
	int w = h;
	rowHeight_= h*rowPadding_;
	colWidth_ = w*columnPadding_;
	charWidth_ = w;
	flagsColWidth_=charWidth_;
	if (flagsColWidth_ < MIN_FLAGS_COL_WIDTH)
		flagsColWidth_ = MIN_FLAGS_COL_WIDTH;
	
	bookmarkPos_= 0;
	indexPos_ = flagsColWidth_;
	lockPos_= indexPos_+ INDEX_WIDTH*charWidth_;
	expanderPos_=lockPos_+flagsColWidth_;
	
	flagsWidth_=expanderPos_+flagsColWidth_;
	labelWidth_=charWidth_*LABEL_WIDTH;
	headerHeight_=rowHeight_*HEADER_HEIGHT;
	
	enableUpdates_=true;
	selectingSequences_=false;
	selectingResidues_=false;
	draggingSequences_=false;
	selAnchorRow_=selAnchorCol_=selDragRow_=selDragCol_=-1; 
	seqSelectionAnchor_=seqSelectionDrag_=-1;
	leftDown_=false;
	
	lastInfo_="";
	
	currGroupColour_=0;
	
	totalWheelRotation_=0;
	
	baseTimeout_=500;
	currentTimeout_=baseTimeout_;
	
	scrollRowTimer_.setInterval(baseTimeout_);
	connect(&scrollRowTimer_, SIGNAL(timeout()), this, SLOT(scrollRow()) );
	
	scrollColTimer_.setInterval(baseTimeout_);
	connect(&scrollColTimer_, SIGNAL(timeout()), this, SLOT(scrollCol()) );
	
	currFocus_ = SequenceView;
	
	currSearchResult_=NULL;
	
	repaintDirtyRows_=false;
	
}


void SequenceEditor::updateViewExtents()
{
	// round up to catch fractional bits ?
	int displayableRows=ceil((double)(height()-headerHeight_-footerHeight_)/((double) rowHeight_));
	lastVisibleRow_ = firstVisibleRow_ + displayableRows-1; // zero indexed, so subtract 1
	
	numRows_ = project_->sequences.numVisible();
	if (lastVisibleRow_ >= numRows_){
		lastVisibleRow_ = numRows_-1; // could be < 0 but next test fixes that
		//firstVisibleRow_ = lastVisibleRow_ - displayableRows + 1;
	}
	if (lastVisibleRow_ < firstVisibleRow_)
		lastVisibleRow_=firstVisibleRow_;
	
	numCols_= project_->sequences.maxLength();
	int displayableCols=ceil((double)(width() - (flagsWidth_+labelWidth_))/((double) colWidth_));
	lastVisibleCol_ = firstVisibleCol_ + displayableCols -1;
	
	if (lastVisibleCol_ >= numCols_)
		lastVisibleCol_ = numCols_-1; 
	if (lastVisibleCol_ < firstVisibleCol_)
		lastVisibleCol_=firstVisibleCol_;
	
	qDebug() << trace.header(__PRETTY_FUNCTION__) << firstVisibleRow_ << " " << lastVisibleRow_ << " " << displayableCols << " " << firstVisibleCol_ << " " << lastVisibleCol_;
}

QChar SequenceEditor::cellContent(int row, int col, int maskFlags, Sequence *currSeq )
{
	
	QChar pChar;
	QString s;
	
	QList<Sequence *> &seq = project_->sequences.sequences();
	
	if (NULL == currSeq)	
		currSeq = project_->sequences.visibleAt(row);
	
	if (!seq.isEmpty()){
		s=currSeq->residues;
		pChar= s[col];
		if (pChar.unicode() != 0){
			if (col <  s.length())
				return QChar(pChar.unicode() & maskFlags);
			else
				return QChar(0);
		}
		else
			return QChar(0);
	}
	else 
		return QChar(0);

}


void SequenceEditor::getResidueColour(int ch,QColor &colour,bool cellSelected)
{
	int ich = ch-65;
	
	if (sequenceDataType_ == SequenceFile::DNA){
		switch (colourMap_)
		{
			case StandardDNAMap:
				switch (ch)
				{
					case '-':
					{
						if (cellSelected)
							colour.setRgb(0,0,0);
						else
							colour.setRgb(224,224,224);
						break;
					}
					case '?': // internal use only
						colour.setRgb(0,0,0);
						break;
					default:
						colour.setRgb(StandardDNAColours[ich][0],StandardDNAColours[ich][1],StandardDNAColours[ich][2]);
						break;
				}
				break;
			case MonoDNAMap:
				colour.setRgb(224,224,224);
				break;
		} // of switch
	}
	else if (sequenceDataType_ == SequenceFile::Proteins)
	{
		switch (colourMap_)
		{
			case PhysicoChemicalMap:
				switch (ch)
				{
					case 'A': case 'I': case 'L' : case 'V' : case 'G': case '-':case '!':case '.':
					{
						if (cellSelected)
							colour.setRgb(0,0,0);
						else
							colour.setRgb(224,224,224);
						break;
					}
					case '?': // internal use only
						colour.setRgb(0,0,0);
						break;
					default:
						colour.setRgb(PhysicoChemicalColours[ich][0],PhysicoChemicalColours[ich][1],PhysicoChemicalColours[ich][2]);
						break;
				}
				break;
			case RasMolMap:
				switch (ch)
				{
					case '-':case '!':case '.':
					{
						if (cellSelected)
							colour.setRgb(0,0,0);
						else
							colour.setRgb(224,224,224);
						break;
					}
					default:
						colour.setRgb(RasMolColours[ich][0],RasMolColours[ich][1],RasMolColours[ich][2]);
						break;
				}
				break;
			case TaylorMap:
				switch (ch)
				{
					case '-':case '!':case '.':
					{
						if (cellSelected)
							colour.setRgb(0,0,0);
						else
							colour.setRgb(224,224,224);
						break;
					}
					default:
						colour.setRgb(TaylorColours[ich][0],TaylorColours[ich][1],TaylorColours[ich][2]);
						break;
				}
				break;
			case MonoMap:
			{
				colour.setRgb(224,224,224);
				break;
			}
		}
	}
}

void SequenceEditor::paintCell( QPainter* p, int row, int col, Sequence *currSeq )
{
	QChar c,cwflags;
	QColor txtColor,fillColour;
	int cellSelected=false;
	
	if (NULL == currSeq){
		currSeq = project_->sequences.visibleAt(row);
	}
	c=cellContent(row,col,REMOVE_FLAGS,currSeq);
	
	if (c.unicode()==0) return;
	
	int yrow = headerHeight_+ rowHeight_*(row-firstVisibleRow_);
	int xcol = flagsWidth_ + labelWidth_ + (col-firstVisibleCol_)*colWidth_;
	p->translate(xcol,yrow);
	
	int w = colWidth_;
	int h = rowHeight_;
	
	cwflags=cellContent(row,col,KEEP_FLAGS,currSeq);
	
	// If the cell is highlighted then do it
	
	if (selectingResidues_ && currSeq->visible){
		//qDebug() << trace.header(__PRETTY_FUNCTION__) << selAnchorRow_  << " " << selDragRow_;
		if (selAnchorRow_ <= selDragRow_){ // dragging top to bottom
			if (selAnchorCol_ <= selDragCol_){ // left to right
				if (row >= selAnchorRow_ && row <= selDragRow_ && 
					col >= selAnchorCol_ && col<=selDragCol_){
						cellSelected=true;
				}
			}
			else{
				if (row >= selAnchorRow_ && row <= selDragRow_ && 
					col <= selAnchorCol_ && col>=selDragCol_){
						cellSelected=true;
				}
			}
		}
		else{ // dragging bottom to top
			if (selAnchorCol_ <= selDragCol_){ // left to right
				if (row <= selAnchorRow_ && row >= selDragRow_ && 
					col >= selAnchorCol_ && col<=selDragCol_){
						cellSelected=true;
				}
			}
			else{
				if (row <= selAnchorRow_ && row >= selDragRow_ && 
					col <= selAnchorCol_ && col>=selDragCol_){
						cellSelected=true;
				}
			}
		}
	}
	
	if (cellSelected){
		fillColour.setRgb(192,192,192);
		p->fillRect(0,0,w,h,fillColour);
	}
	
	//  Draw cell content 
	// Physico-chemical properties 
	
	char ch = c.toLatin1();
	getResidueColour(ch,txtColor,cellSelected);
	
		// FIXME not so useful if the group is not contiguous
	//if (currSeq->group != NULL){
	//	int start = rowFirstVisibleSequenceInGroup(currSeq->group);
	//	int stop  = rowLastVisibleSequenceInGroup(currSeq->group);
	//	if (row==start){
	//		p->setPen(currSeq->group->textColour());
	//		p->drawLine(0,2,w-1,2);
	//	}
	//	if (row==stop){
	//		p->setPen(currSeq->group->textColour());
	//		p->drawLine(0,h-2,w-1,h-2);
	//	}
	//}
	
	
	QPen xPen;
	xPen.setWidth(2);
	
	QColor currSearchResultColour(255,255,0);
	QColor searchResultColour(255,165,0);
	
	bool fillCell=false;
	if (currSeq->visible){
		switch (residueView_){
			case StandardView:
				
				if (NULL != currSearchResult_){
					if (currSeq == currSearchResult_->sequence){
						if (col >= currSearchResult_->start && col <= currSearchResult_->stop){
							fillCell=true;
							fillColour=currSearchResultColour;
							txtColor.setRgb(0,0,0);
						}
					}
					else if ((cwflags.unicode() & HIGHLIGHT_CELL) ){
						fillColour = searchResultColour;
						fillCell=true;
						txtColor.setRgb(0,0,0);
					}
				}
				if (fillCell){
					p->fillRect(0,2,w,h-2,fillColour);
				}
				if ((cwflags.unicode() & EXCLUDE_CELL) ){
					xPen.setColor(QColor(240,240,16));
					p->setPen(xPen);
					p->drawLine(2,2,w-2,h-2); // X marks the spot ...
					p->drawLine(w-2,2,2,h-2);
				}
				p->setPen(txtColor);
				p->drawText( 0, 0, w, h, Qt::AlignCenter, c);
				break;
			case InvertedView:
				if (NULL != currSearchResult_){
					if (currSeq == currSearchResult_->sequence){
						if (col >= currSearchResult_->start && col <= currSearchResult_->stop){
							fillCell=true;
							fillColour.setRgb(255,0,0);
							txtColor.setRgb(0,0,0);
						}
					}
					else if ((cwflags.unicode() & HIGHLIGHT_CELL) ){
						fillColour.setRgb(192,0,0);
						fillCell=true;
						txtColor.setRgb(0,0,0);
					}
				}
				if (!fillCell && !cellSelected  && ch != '-'){
					fillCell=true;
					fillColour=txtColor;
				}
				if (fillCell){
					p->fillRect(0,2,w,h-2,fillColour);
				}
				if ((cwflags.unicode() & EXCLUDE_CELL)){
					xPen.setColor(QColor(32,32,32));
					p->setPen(xPen);
					p->drawLine(2,2,w-2,h-2); // X marks the spot ...
					p->drawLine(w-2,2,2,h-2);
				}
				if (ch != '-')
					txtColor.setRgb(0,0,0);
				p->setPen(txtColor);
				p->drawText( 0, 0, w, h, Qt::AlignCenter, c);
				break;
			case SolidView:
				if (NULL != currSearchResult_){
					if (currSeq == currSearchResult_->sequence){
						if (col >= currSearchResult_->start && col <= currSearchResult_->stop){
							fillCell=true;
							fillColour=currSearchResultColour;
						}
					}
					else if ((cwflags.unicode() & HIGHLIGHT_CELL) ){
						fillColour = searchResultColour;
						fillCell=true;
					}
				}
				if (!fillCell && !cellSelected  && ch != '-'){
					fillCell=true;
					fillColour=txtColor;
				}
				if (fillCell){
					p->fillRect(0,2,w,h-2,fillColour);
				}
				if ((cwflags.unicode() & EXCLUDE_CELL) ){
					xPen.setColor(QColor(32,32,32));
					p->setPen(xPen);
					p->drawLine(2,2,w-2,h-2); // X marks the spot ...
					p->drawLine(w-2,2,2,h-2);
				}
				if (ch=='-'){
					p->setPen(txtColor);
					p->drawText( 0, 0, w, h, Qt::AlignCenter, c);
				}
				break;
		}
	}

	p->translate(-xcol,-yrow);
	
}

void SequenceEditor::paintRow(QPainter *p,int row)
{
	QColor labelColor,txtColor;
	
	Sequence *currSeq = project_->sequences.visibleAt(row);
	if (currSeq == NULL) return; // this happens with an empty project
	
	int yrow = headerHeight_+rowHeight_*(row-firstVisibleRow_);
	
	if (project_->sequenceSelection->contains(currSeq)) // highlight if selected
		p->fillRect(flagsWidth_, yrow, labelWidth_,rowHeight_,QColor(128,128,128)); // add one to fill properly
	
	// The selection isn't filled until the mouse is released
	// so a possible selection event has to be handled on the fly
	if (selectingSequences_ && leftDown_){
		int startRow = seqSelectionAnchor_;
		int stopRow  = seqSelectionDrag_;
		if (startRow > stopRow) swap_int(&startRow,&stopRow);
		if (row >= startRow && row <=stopRow)
			p->fillRect(flagsWidth_, yrow, labelWidth_,rowHeight_,QColor(128,128,128));
	}
		
	if (currSeq->group != NULL){
		
		int groupBegin = rowFirstVisibleSequenceInGroup(currSeq->group);
		if (row == groupBegin){
			
			if (currSeq->group->locked()){
				int xpm = lockPos_ + (flagsColWidth_ - lockpm->width())/2;
				int ypm = yrow + (rowHeight_ - lockpm->height())/2;
				p->drawPixmap(xpm, ypm,*lockpm);
			}
		
			if (currSeq->group->hasHiddenSequences()){
				txtColor.setRgb(255,215,0);
				p->setPen(txtColor);
				p->drawText( expanderPos_, yrow, flagsColWidth_, rowHeight_, Qt::AlignCenter, "+");
			}
		}
		
		labelColor=currSeq->group->textColour(); // group colour for sequence label
	}
	else{
		labelColor.setRgb(255,255,255); //default colour 
	}
	
	if (currSeq->bookmarked){
		int xpm = bookmarkPos_ + (flagsColWidth_ - bookmarkpm->width())/2;
		int ypm = yrow + (rowHeight_ - bookmarkpm->height())/2;
		p->drawPixmap(xpm, ypm,*bookmarkpm);
	}
	
	txtColor.setRgb(255,255,255);
	p->setPen(txtColor);
	p->drawText(indexPos_, yrow,charWidth_*INDEX_WIDTH,rowHeight_, Qt::AlignRight, QString::number(row));
	
	p->setPen(labelColor);
	p->drawText( flagsWidth_, yrow, labelWidth_,rowHeight_,Qt::AlignLeft, currSeq->label);
	
	for (int col=firstVisibleCol_;col<=lastVisibleCol_;col++)
		paintCell(p,row,col,currSeq); 
	
}

void SequenceEditor::paintHeader(QPainter *p)
{
	QColor txtColor(255,255,255);
	p->setPen(txtColor);
	int y0 = headerHeight_- 2*rowHeight_;
	int x0 = flagsWidth_+labelWidth_-firstVisibleCol_*colWidth_;
	for (int col=firstVisibleCol_;col<=lastVisibleCol_;col++){
		if (col % 10 == 0)
			p->drawText(x0+(col-1)*colWidth_,y0,colWidth_*3,rowHeight_,Qt::AlignCenter,QString::number(col));
	}
	
	paintConsensusSequence(p);
	
}

void SequenceEditor::paintConsensusSequence(QPainter *p)
{
	QColor txtColour,fillColour;
	txtColour.setRgb(255,255,255);
	p->setPen(txtColour);
	int y0 = headerHeight_- rowHeight_;
	int x0 = flagsWidth_+labelWidth_;
	int tw = fontMetrics().width("Consensus");
	p->drawText(x0-tw-4,y0,tw,rowHeight_,Qt::AlignRight,"Consensus");
	
	QString &seq = project_->consensusSequence.sequence();
	for (int col=firstVisibleCol_;col<=lastVisibleCol_;col++){
		if (col >= seq.size()) break;
		QChar c = seq[col];
		char ch = c.toLatin1(); 
		if (ch=='?') continue;
		getResidueColour(ch,txtColour,false);
		p->setPen(txtColour);
		int xcol = x0 + (col-firstVisibleCol_)*colWidth_;
		int w = colWidth_;
		int h = rowHeight_;
		switch (residueView_){
			case StandardView:
				p->drawText( xcol, y0, w, h, Qt::AlignCenter, c);
				break;
			case InvertedView:
				if (ch != '-'){
					p->fillRect(xcol,y0+2,w,h-2,txtColour);
					txtColour.setRgb(0,0,0);
				}
				p->setPen(txtColour);
				p->drawText( xcol, y0, w, h, Qt::AlignCenter, c);
				break;
			case SolidView:
				if (ch=='-')
					p->drawText( xcol, y0, w, h, Qt::AlignCenter, c);
				else
					p->fillRect(xcol,y0,w,h-2,txtColour);
				break;
		}
	}
	
	
	if (!project_->consensusSequence.isValid() && seq.size() > 0){
		fillColour.setRgb(128,128,128,128);
		p->fillRect(x0,y0,width()-x0,rowHeight_,fillColour);
	}
}

int SequenceEditor::rowAt(int ypos)
{
	if (ypos - headerHeight_ <0) return -1;
	if (ypos > height()-FOOTER_HEIGHT) return -1;
	return (int) ((ypos-headerHeight_)/rowHeight_) + firstVisibleRow_;
}

int SequenceEditor::columnAt(int xpos)
{
	// The return result only makes sense for the FLAGS area
	return (int) ((xpos-(flagsWidth_ + labelWidth_))/colWidth_) + firstVisibleCol_;
}

int SequenceEditor::rowFirstVisibleSequenceInGroup(SequenceGroup *sg)
{
	int visIndex=0;
	for (int s=0;s<project_->sequences.size();s++){ // checked OK
		Sequence *seq = project_->sequences.sequences().at(s);
		if (sg==seq->group && seq->visible)
			return visIndex;
		if (seq->visible)
			visIndex++;
	}
	return -1;
}

// FIXME untested
int SequenceEditor::rowLastVisibleSequenceInGroup(SequenceGroup *sg)
{
	int visIndex=0;
	int ret=visIndex;
	for (int s=0;s<project_->sequences.size();s++){ // checked OK
		Sequence *seq = project_->sequences.sequences().at(s);
		if (sg==seq->group && seq->visible){
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "row = " << visIndex;
			return visIndex;
		}
		if (seq->visible)
			visIndex--;
	}
	return -1;
}

int SequenceEditor::rowVisibleSequence(Sequence *seq)
{
	int visIndex=0;
	for (int s=0;s<project_->sequences.size();s++){ 
		Sequence *tmpseq = project_->sequences.sequences().at(s);
		if (tmpseq==seq)
			return visIndex;
		if (tmpseq->visible)
			visIndex++;
	}
	
	return -1;
}

int SequenceEditor::rowFirstVisibleSequence(QList<Sequence *> &seqs)
{
	int ret = 0;
	for (int s=0;s<project_->sequences.size();s++){
		if (seqs.contains(project_->sequences.sequences().at(s))){
			break;
		}
		if (project_->sequences.sequences().at(s)->visible) ret++;
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "row = " << ret;
	return ret;
}

int SequenceEditor::rowLastVisibleSequence(QList<Sequence *> &seqs)
{
	int visIndex=0;
	int ret = visIndex;
	for (int s=0;s<project_->sequences.size();s++){
		if (seqs.contains(project_->sequences.sequences().at(s))){
			ret=visIndex; // this gets the last match
		}
		if (project_->sequences.sequences().at(s)->visible) visIndex++;
	}
	
	qDebug() << trace.header(__PRETTY_FUNCTION__) << seqs.size() << " row = " << ret;
	return ret;
}

void SequenceEditor::makeVisible(Sequence *seq,int startCol, int stopCol)
{

	qDebug() << trace.header(__PRETTY_FUNCTION__) << startCol << " " << stopCol;
	if (!seq->visible){
		seq->visible=true;
		updateViewExtents();
	}
	
	bool changed=false;
	
	if (startCol >= 0){ // startCol <0 means ignore
		if (startCol < firstVisibleCol_){
			if (startCol != 0) // looks neater
				firstVisibleCol_=startCol+1;
			else
				firstVisibleCol_=startCol;
			if (firstVisibleCol_>=numCols_)
				firstVisibleCol_=numCols_-1;
			changed=true;
		}
		else if (startCol > lastVisibleCol_){
			firstVisibleCol_+= stopCol-lastVisibleCol_+1;
			if (firstVisibleCol_ >=numCols_)
				firstVisibleCol_= numCols_-1;
			changed=true;
		}
	}
	
	int r = project_->sequences.visibleIndex(seq);
	if (r < firstVisibleRow_){
		r--; // add a bit more space
		if (r<0) r=0;
		firstVisibleRow_=r;
		changed=true;
	}
	else if (r>lastVisibleRow_){
		r++; // add a bit more space
		if (r>=numRows_) r=numRows_-1;
		firstVisibleRow_=r-(lastVisibleRow_-firstVisibleRow_);
		changed=true;
	}
	
	if (changed){
		qDebug() << trace.header(__PRETTY_FUNCTION__) << firstVisibleCol_;
		updateViewExtents();
		emit viewExtentsChanged(firstVisibleRow_,lastVisibleRow_,numRows_,firstVisibleCol_,lastVisibleCol_,numCols_);
	}
	
	repaint(); //  repaint() to show selection

}

void SequenceEditor::connectToProject()
{
	connect(&(project_->sequences),SIGNAL(cleared()),this,SLOT(sequencesCleared()));
	connect(project_,SIGNAL(uiUpdatesEnabled(bool)),this,SLOT(enableUpdates(bool)));
	connect(project_,SIGNAL(searchResultsCleared()),this,SLOT(clearSearchResults()));
}

void SequenceEditor::disconnectFromProject()
{
	disconnect(&(project_->sequences),SIGNAL(cleared()),this,SLOT(sequencesCleared()));
	disconnect(project_,SIGNAL(uiUpdatesEnabled(bool)),this,SLOT(enableUpdates(bool)));
	disconnect(project_,SIGNAL(searchResultsCleared()),this,SLOT(clearSearchResults()));
}

void SequenceEditor::cleanupTimer()
{
	//qDebug() << trace.header(__PRETTY_FUNCTION__);
	scrollRowTimer_.stop();
	scrollRowTimer_.setInterval(baseTimeout_);
	scrollColTimer_.stop();
	scrollColTimer_.setInterval(baseTimeout_);
	currentTimeout_ = baseTimeout_;
}

QRect SequenceEditor::dirtyRowsRect(int startRow,int stopRow)
{
	QRect r;
	r.setX(0);
	r.setY(headerHeight_+rowHeight_*(startRow-firstVisibleRow_));
	r.setWidth(width());
	r.setHeight(rowHeight_*(stopRow-startRow+1));
	return r;
}
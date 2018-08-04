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

// 120507 MJW BUG FIX in readAlignment(). The last residue before an insertion block was being displaced
//						to the end of the insertion block
//						Excluded residues now removed from an aligned sequence
// 
//

#include <QtDebug>
#include "DebuggingInfo.h"

#include <iostream>


#include <QBoxLayout>
#include <QClipboard>
#include <QColor>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>
#include <QPrintDialog>
#include <QProcess>
#include <QPushButton>
#include <QScrollBar>
#include <QSet>
#include <QStatusBar>
#include <QSplitter>
#include <QTemporaryFile>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>


#include "Application.h"
#include "AlignmentTool.h"
#include "AlignmentToolDlg.h"
#include "Clipboard.h"
#include "ClustalFile.h"
#include "ClustalO.h"
#include "FASTAFile.h"
#include "GoToTool.h"
#include "ImportDialog.h"
#include "MessageWin.h"
#include "Muscle.h"
#include "PDBFile.h"
#include "Project.h"
#include "ResidueSelection.h"
#include "SearchTool.h"
#include "SequenceEditor.h"
#include "SeqEditMainWin.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SequencePropertiesDialog.h"
#include "SequenceSelection.h"
#include "AlignmentCmd.h"
#include "XMLHelper.h"

#include "Consensus.h"

#include "PDB.h"

using namespace std;

// Tool button help

const char *lockText = "Click this button to add and edit a new"
"sequence lock.";

//
// SeqEditMainWin - public members
// 

SeqEditMainWin::SeqEditMainWin(Project *project)
	:QMainWindow()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << " creating";
	
	init();
	project_=project;
	
	setWindowTitle("tweakseq - " + project->name());
	setGeometry(0,0,800,600);
	//setWindowIcon(QIcon(seqedit_xpm));

	split = new QSplitter(Qt::Vertical,this);
	
	QWidget *w = new QWidget(split);
	QBoxLayout *vl = new QBoxLayout(QBoxLayout::TopToBottom,w);
	QBoxLayout *hl = new QBoxLayout(QBoxLayout::LeftToRight);
	vl->addLayout(hl);
	se = new SequenceEditor(project_,w);
	hl->addWidget(se);
	// Units of the vscroller are ROWS
	vscroller_ = new QScrollBar(Qt::Vertical,w);
	hl->addWidget(vscroller_);
	
	// Units of the hscroller are CELLS (residues)
	// The hscroller does not control the leftmost information field
	hscroller_ = new QScrollBar(Qt::Horizontal,w);
	vl->addWidget(hscroller_);
		
	mw = new MessageWin(split);
	
	QList<int> wsizes;
	wsizes.append(600);wsizes.append(200);
	split->setSizes(wsizes);
	
	setCentralWidget(split);

	// need to connect to other widgets so do create actions last
	createActions();
	createMenus();
	createStatusBar(); 
	createToolBars();
	
	printer = new QPrinter();
	printer->setFullPage(true);
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this,SIGNAL(customContextMenuRequested ( const QPoint & )),this,SLOT(createContextMenu(const QPoint &)));
	connect(se,SIGNAL(viewExtentsChanged(int,int,int,int,int,int)),this,SLOT(updateScrollBars(int,int,int,int,int,int)));
	
	connect(vscroller_,SIGNAL(valueChanged(int)),se,SLOT(setFirstVisibleRow(int)));
	connect(hscroller_,SIGNAL(valueChanged(int)),se,SLOT(setFirstVisibleColumn(int)));
	connect(se,SIGNAL(edited()),this,SLOT(setupEditActions()));
	
	connect(goToTool_,SIGNAL(find(const QString &)),se,SLOT(selectSequence(const QString &)));
	
	connectToProject();
	
	statusBar()->showMessage("Ready");
}

SeqEditMainWin::~SeqEditMainWin(){
	delete printer;
	if (alignmentFileIn_ != NULL) delete alignmentFileIn_;
	if (alignmentFileOut_ != NULL) delete alignmentFileOut_;
}


void SeqEditMainWin::doAlignment(){
}

// This is called by a Project after loading is completed
// Because the Project may have customised settings, actions are updated here
void SeqEditMainWin::postLoadTidy()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	se->postLoadTidy();
	updateGoToTool();
	setupAlignmentActions();
	updateSettingsActions();
	setWindowTitle("tweakseq - " + project_->name());
}

void SeqEditMainWin::writeSettings(QDomDocument &doc,QDomElement &parentElem)
{
	QDomElement pelem = doc.createElement("main_window_ui");
	parentElem.appendChild(pelem);
	XMLHelper::addElement(doc,pelem,"width",QString::number(size().width()));
	XMLHelper::addElement(doc,pelem,"height",QString::number(size().height()));
	QList<int> splitterHeights = split->sizes();
	XMLHelper::addElement(doc,pelem,"editor_window_height",QString::number(splitterHeights.at(0)));
	XMLHelper::addElement(doc,pelem,"message_window_height",QString::number(splitterHeights.at(1)));
	
	se->writeSettings(doc,parentElem);
}

void SeqEditMainWin::readSettings(QDomDocument &doc)
{
	QDomNodeList nl = doc.elementsByTagName("main_window_ui");
	if (nl.count() == 1){
		QDomNode gNode = nl.item(0);
		QDomElement elem = gNode.firstChildElement();
		int w = width();
		int h = height();
		QList<int> wsizes=split->sizes();
		while (!elem.isNull()){
			if (elem.tagName() == "width"){
				w=elem.text().toInt();
			}
			else if (elem.tagName() == "height"){
				h=elem.text().toInt();
			}
			else if (elem.tagName() == "editor_window_height"){
				wsizes.replace(0,elem.text().toInt());
			}
			else if (elem.tagName() == "message_window_height"){
				wsizes.replace(1,elem.text().toInt());
			}
			elem=elem.nextSiblingElement();
		}
		setGeometry(0,0,w,h);
		split->setSizes(wsizes);
	}
	se->readSettings(doc);
	setupAlignmentActions(); 
	updateSettingsActions();  
}

//
// Public slots
//	


//
// Protected
//
	
void SeqEditMainWin::closeEvent(QCloseEvent *ev)
{
	if (maybeSave()) {
		// FIXME do some more stuff ?
		ev->accept();
	} else {
		ev->ignore();
	}
}

//
// SeqEditMainWin - private slots
// 

// File menu slots

void SeqEditMainWin::fileNewProject()
{
	Project *proj = app->createProject();
	proj->createMainWindow();
	
}

void SeqEditMainWin::fileOpenProject()
{
	if (!project_->empty()){
	}
		
	QString fname = QFileDialog::getOpenFileName(this,
    tr("Open Project"), "./", tr("Project Files (*.tsq)"));
	if (fname.isNull()) return;
	
	// Replace the existing project
	disconnectFromProject();
	Project *oldProject = project_;
	
	init(); // start in a clean state
	project_=app->createProject();
	project_->setMainWindow(this);
	se->setProject(project_);
	connectToProject();
	delete oldProject; // and the undo stack disappears with it
	project_->load(fname);
	
	updateSettingsActions();
}

void SeqEditMainWin::fileSaveProject()
{
	QString fpathname;
	if (!project_->named()){ // never saved so need to get a project name and path
		 fpathname = QFileDialog::getSaveFileName(this, tr("Save project"));
		if (fpathname.isNull()) return;
	}
	
	project_->save(fpathname);
	setWindowTitle("tweakseq - " + project_->name());
}

void SeqEditMainWin::fileSaveProjectAs()
{
	QString fpathname = QFileDialog::getSaveFileName(this, tr("Save project as"));
	if (fpathname.isNull()) return;
	project_->save(fpathname);
	setWindowTitle("tweakseq - " + project_->name());
}
	
void SeqEditMainWin::fileImport(){
	
	// This will only run the first time sequence are imported
	bool firstRun=false;
	if (project_->sequenceDataType() == SequenceFile::Unknown){
		firstRun=true;
		ImportDialog idlg(SequenceFile::Proteins,this);
		if (QDialog::Accepted == idlg.exec()){
			int ret = idlg.dataType();
			qDebug() << trace.header(__PRETTY_FUNCTION__) << ret;
			project_->setSequenceDataType(ret);
			se->setSequenceDataType(ret);
		}
		else{
			statusBar()->showMessage("The import was canceled");
			return;
		}
	}
	
	FASTAFile ff;
	ClustalFile cf;
	PDBFile    pf;
	
	QString allext="";
	
	QStringList ext = ff.extensions(project_->sequenceDataType());
	for (int s=0;s<ext.size();s++)
		allext = allext + ext.at(s) + " ";
	
	ext = cf.extensions(project_->sequenceDataType());
	for (int s=0;s<ext.size();s++)
		allext = allext + ext.at(s) + " ";
	
	if (project_->sequenceDataType() == SequenceFile::Proteins){
		ext = pf.extensions( SequenceFile::Proteins);
		for (int s=0;s<ext.size();s++)
			allext = allext + ext.at(s) + " ";
	}
	
	allext.append(")");
	allext.prepend("Sequence Files (");
	
	QString startDir = "./";
	if (!lastImportedFile.isEmpty()){
		QFileInfo fi(lastImportedFile);
		if (!fi.absolutePath().isEmpty())
			startDir = fi.absolutePath();
	}
	QString msg = "Open protein sequence";
	if (project_->sequenceDataType() == SequenceFile::DNA)
		msg = "Open DNA sequence";
	QStringList files = QFileDialog::getOpenFileNames(this,
    msg,startDir, allext);
	qDebug() << trace.header() << files;
	if (files.isEmpty()){ 
		if (firstRun){ // cancelling means reset back to unknown 
			project_->setSequenceDataType(SequenceFile::Unknown); 
			se->setSequenceDataType(SequenceFile::Unknown);
		}
		return;
	}
	QString errmsg;
	bool ok=project_->importSequences(files,errmsg);
	if (!ok)
		QMessageBox::critical(this, tr("Error during import"),errmsg);
	else
		lastImportedFile=files.at(files.size()-1);
	
	updateGoToTool();
	
}

void SeqEditMainWin::fileExportFASTA(){
	QString fname = QFileDialog::getSaveFileName(this,tr("Export as FASTA"));
	if (fname.isNull()) return;
	project_->exportFASTA(fname,true); // FIXME hardcoded
}

void SeqEditMainWin::fileExportClustalW()
{
	QString fname = QFileDialog::getSaveFileName(this,tr("Export as FASTA"));
	if (fname.isNull()) return;
	project_->exportClustalW(fname,true); // FIXME hardcoded
}

void SeqEditMainWin::filePrint(){
	
	int pg,numPages;
	int counterSpc = 0; // extra space between counter line and residue
	int lineSpc=0;      // extra interline spacing
	int minBlockSpc=20;    // min spacing between residue blocks
	double blockSpc; // to avoid rounding errors ...
	double currBlockSpc=0;
	int headerSpc=20;   // extra space underneath page header
	int wrap; // number of residues to print per line
	
	int resWidth; // width of page section showing residues
	int seqNameX0;
	int resX0;
	int topMargin,bottomMargin,rightMargin,leftMargin;
	int blocksPerPage;
	int nBlocks=0;
	int blocksDone=0;
	int x,y,yt,i,j,k;
	int nTicks;
	int maxLength=0;
	int blocksToDo=0;
	
	QString tmp;
	
	// Generates a dud Postscript file if I use typewriter
	QFont mainFont("courier",10);
	QFont counterFont("courier",5);
	QFontMetrics cfm(counterFont);
	
	topMargin=20;
	bottomMargin=20;
	leftMargin=20;
	rightMargin=20;
	
	QPrintDialog printDialog(printer,this);
	
	if (printDialog.exec()==QDialog::Accepted){
		statusBar()->showMessage("Printing ...");
		
		QPainter p;
		p.begin(printer);
		p.setFont(mainFont);
		QFontMetrics fm = p.fontMetrics();
	
		int lMargin = printer->paperRect().left() - printer->pageRect().left();
		int tMargin = printer->paperRect().top() - printer->pageRect().top();
		QSize m(lMargin,tMargin);
		
		// Calculate the available vertical space for writing the sequences
		
		int vSpace = printer->height()-2*m.height()-fm.height()-headerSpc-
			topMargin-bottomMargin;
		
		// We want to divide this up so that sequence blocks are not broken
		// across a page. 
		
		int blockHeight = 
			fm.height()+counterSpc+
			project_->sequences.size()*fm.height()+(project_->sequences.size()-1)*lineSpc ;
		
		// There could of course be so many sequences that they won't
		// even fit on one line
		if (blockHeight > vSpace){
			blocksPerPage=0;
			blockSpc=minBlockSpc;
		}
		else{
			//Estimate the number of blocks per page
			blocksPerPage = vSpace/(blockHeight+minBlockSpc);
			//Can we fit another block ?
			if (((blocksPerPage+1)*(blockHeight+minBlockSpc) - minBlockSpc) <= vSpace)
				blocksPerPage++;
			if (blocksPerPage == 1)
				blockSpc=0;
			else{
				blockSpc=(vSpace-blocksPerPage*blockHeight)/(blocksPerPage-1);
				if (blockSpc>3*minBlockSpc) // TO DO a better criterion for too much
					blockSpc=minBlockSpc;
			}
		}
		  
		// Calculate the number of residues to print per line
		// Want a multiple of 10
		// At a guess, W is the widest character we might want to print
		int cw=fm.width('W');
		// We'll allow a 10 character column for the sequence name
		int rw=printer->width() - 2*m.width()-leftMargin-rightMargin - 10*cw;
		wrap=rw/(10*cw); // BUG for weird font/paper choices might not get 10/line
		wrap*=10;
		
		// How many pages ?
		// Need to find the longest sequence 
		
		if (project_->sequences.size()==0)
			numPages=1;
		else{
			// Find the longest sequence
			maxLength=project_->getResidues(0,KEEP_FLAGS).length();
			for (i=0;i<project_->sequences.size();i++)
				if (project_->getResidues(i,KEEP_FLAGS).length()>maxLength)
					maxLength=project_->getResidues(i,KEEP_FLAGS).length();
			if (blocksPerPage==0)
				numPages=99; // TO DO
			else{
				numPages = maxLength/(blocksPerPage*wrap)+ 1;
				nBlocks  = maxLength/wrap;
				if (maxLength % wrap != 0)
					nBlocks++;
			}
		}
		printf("nBlocks=%i\n",nBlocks);
		printf("maxLength=%i\n",maxLength);
		printf("rw = %i\n",rw);
		printf("cw = %i\n",cw);
		printf("wrap=%i\n",wrap);
		printf("vspace=%i\n",vSpace);
		printf("blockHeight=%i\n",(int) blockHeight);
		printf("blocksPerPge=%i\n",blocksPerPage);
		printf("blockSpc=%i\n",(int) blockSpc);
		
		// Now layout the pages
		resWidth = wrap*cw;
	  seqNameX0 = m.width()+leftMargin;
		resX0 = seqNameX0+10*cw;
		
		y=m.height()+topMargin+fm.ascent(); // subsequently add fm.lineSpacing()
		
		int resCnt=0;
		blocksDone=0;
		pg=0;
		
		while (blocksDone < nBlocks){
		
			// Start a new page
			pg++;
			// First line of each page contains
			// Project name   Creation date  Page numbering
			y=m.height()+topMargin+fm.ascent();
			x=resX0;
			p.drawText(QPoint(x,y),windowTitle()+ " " + QDate::currentDate().toString());
		  tmp.sprintf("%i/%i",pg,numPages);
			p.drawText(QPoint(printer->width()-m.width()-leftMargin-fm.width(tmp),y),tmp);
				
			p.drawRect(m.width(),m.height(),printer->width()-2*m.width(),
				printer->height()-2*m.height());
		
			p.drawRect(m.width()+leftMargin,m.height()+topMargin,
				printer->width()-2*m.width()-leftMargin-rightMargin,
				printer->height()-2*m.height()-topMargin-bottomMargin);
			
		  y+= headerSpc;
			
			if (blocksPerPage == 0){
				// Formatting of this case is a bit different
				
				// Have to determine whether we will finish a block on this
				// page - if so allocate space for the break and counter
				
				// Each page has a counter on it regardless
				
			}
			else{
				if (pg < numPages){ // completely filled page
					currBlockSpc=blockSpc;
					blocksToDo=blocksPerPage;
				}
				else{// partially filled page so these blocks have minBlockSpc;
					// Works for the case where there is one block per page as well
			 	 	currBlockSpc=minBlockSpc;
					blocksToDo=nBlocks-blocksDone;
				}
			}
			
		  for (i=0;i<blocksToDo;i++){
				// Print the counters
				yt=y;
				x=resX0-cw; // so that ticks are left aligned
				yt+=fm.ascent();
				if ((maxLength - resCnt) > wrap)
					nTicks=wrap/10;
				else
					nTicks=(maxLength-resCnt)/10;
				p.setFont(counterFont);
				for (k=1;k<=nTicks;k++){
					x+=cw*10;
					tmp.sprintf("%i",k*10+resCnt);
					p.drawText(QPoint(x,yt),tmp);
				}
				p.setFont(mainFont);	
				for (j=0;j<project_->sequences.size();j++){
					// Print the sequence name
					x=seqNameX0;
					yt+=fm.height();
					p.drawText(QPoint(x,yt),project_->getLabelAt(j)); // TO DO bounding rect
					// Print the residues
					tmp=project_->getResidues(j,KEEP_FLAGS);
					x=resX0;
					for (k=resCnt;k<resCnt+wrap && k < tmp.length();k++){
						printRes(&p,QChar(tmp[k-resCnt]),x,yt);
						x+=cw;
					}	
				}
				blocksDone++;
				resCnt+=wrap;
				y=(int) (blocksDone*(currBlockSpc+blockHeight)
					+m.height()+fm.height()+headerSpc+topMargin);
			}
		
			// for (i=0;i<blocksPerPage;i++){
			//	x=resX0;
			//	p.drawRect(x,y+i*(blockHeight+currBlockSpc),resWidth,blockHeight);
			//}
			
			if (pg<numPages) printer->newPage(); // to avoid extra page
			
	  } // while (blocksDone < nBlocks)
		p.end();
		statusBar()->showMessage("Printing completed",2000);
	} // if printer->setup(this)
	else
		statusBar()->showMessage("Printing cancelled",2000);
}

void SeqEditMainWin::fileClose(){
	close();
}

// Edit menu slots


// Connected to  aboutToShow()
void SeqEditMainWin::setupEditActions()
{
	if (se->isReadOnly()){
		cutAction->setEnabled(false);
		pasteAction->setEnabled(false);
		// find is allowed
		findAction->setEnabled(project_->residueSelection->size()==1);
		undoAction->setEnabled(false);
		redoAction->setEnabled(false);
		groupSequencesAction->setEnabled(false);
		ungroupSequencesAction->setEnabled(false);
		ungroupAllAction->setEnabled(false);
		hideNonSelectedGroupMembersAction->setEnabled(false);
		unhideAllGroupMembersAction->setEnabled(false);
		unhideAllAction->setEnabled(true);
		lockAction->setEnabled(false);
		unlockAction->setEnabled(false);
		excludeAction->setEnabled(false);
		removeExcludeAction->setEnabled(false);
		renameSequenceAction->setEnabled(false);
		return;
	}
	
	cutAction->setEnabled(project_->residueSelection->isInsertionsOnly() || (!project_->sequenceSelection->empty()));
	copyAction->setEnabled(!project_->residueSelection->empty() || (!project_->sequenceSelection->empty()));
	pasteAction->setEnabled(project_->sequenceSelection->size()==1 && (!app->clipboard().isEmpty()));
	
	findAction->setEnabled(project_->residueSelection->size()==1);
	
	if (project_->undoStack().canUndo()){
		undoAction->setEnabled(true);
		undoAction->setText("Undo " + project_->undoStack().undoText());
	}
	else{
		undoAction->setEnabled(false);
		undoAction->setText("Undo");
	}
	
	if (project_->undoStack().canRedo()){
		redoAction->setEnabled(true);
		redoAction->setText("Redo " + project_->undoStack().redoText() );
	}
	else{
		redoAction->setEnabled(false);
		redoAction->setText("Redo");
	}
	
	groupSequencesAction->setEnabled(project_->canGroupSelectedSequences());
	// If the selection contains any grouped sequences then we can ungroup
	ungroupSequencesAction->setEnabled(false);
	for (int s=0;s<project_->sequenceSelection->size();s++){
		if (project_->sequenceSelection->itemAt(s)->group != NULL)
			ungroupSequencesAction->setEnabled(true);
	}
	
	lockAction->setEnabled(project_->canToggleLock());
	unlockAction->setEnabled(project_->canToggleLock());
	
	// If the selection contains only sequences within a single group then the non-selected sequences can be hidden
	// or unhidden
	QList<SequenceGroup *> groups;
	for (int s=0;s<project_->sequenceSelection->size();s++){
		SequenceGroup *g = project_->sequenceSelection->itemAt(s)->group;
		if (g != NULL){
			if (!groups.contains(g))
				groups.append(g);
		}
	}
	hideNonSelectedGroupMembersAction->setEnabled(groups.size()==1);
	if (groups.size()!=1){
		unhideAllGroupMembersAction->setEnabled(false);
	}
	else{
		unhideAllGroupMembersAction->setEnabled(true);
	}
	excludeAction->setEnabled(!(project_->residueSelection->empty())); // sloppy - don't worry about already excluded etc.
	removeExcludeAction->setEnabled(!(project_->residueSelection->empty())); 
	
	renameSequenceAction->setEnabled(project_->sequenceSelection->size()==1);
	sequencePropertiesAction->setEnabled(project_->sequenceSelection->size()==1);
}

void SeqEditMainWin::editCopy()
{
	// This only copies to the system clipboard
	// If Sequences are selected, then label+sequence is copied as straight text
	// If Residues are selected, then label+selected residues are copied
	if (project_->sequenceSelection->size() > 0){
		QString txt("");
		for (int s=0;s<project_->sequenceSelection->size();s++){
			Sequence *seq = project_->sequenceSelection->itemAt(s);
			txt.append(seq->label);
			txt.append(" ");
			txt.append(seq->filter());
			txt.append('\n'); // FIXME does this get translated ?
		}
		QApplication::clipboard()->setText(txt);
	}
	else if (project_->residueSelection->size() > 0){
		QString txt("");
		for (int r=0;r<project_->residueSelection->size();r++){
			ResidueGroup *rg = project_->residueSelection->itemAt(r);
			txt.append(rg->sequence->label);
			txt.append(" ");
			txt.append(rg->sequence->filter().mid(rg->start,rg->stop-rg->start+1));
			txt.append('\n');
		}
		QApplication::clipboard()->setText(txt);
	}
}



void SeqEditMainWin::editReadOnly(){
	se->setReadOnly(readOnlyAction->isChecked());
}

void SeqEditMainWin::renameSequence(){
	bool ok;
	Sequence *selSeq=project_->sequenceSelection->itemAt(0);
	QString newName = QInputDialog::getText(this, tr("Rename sequence"),
		tr("Name:"), QLineEdit::Normal,selSeq->label, &ok);
		if (ok && !newName.isEmpty() && newName != selSeq->label){
			if (project_->renameSequence(selSeq,newName)){
				updateGoToTool();
			}
			else{
				QMessageBox::critical(this, tr("Rename failed"),"The name \"" + newName + "\" is already used in this project");
			}
		}
}

void SeqEditMainWin::sequenceProperties()
{
	Sequence *selSeq=project_->sequenceSelection->itemAt(0);
	SequencePropertiesDialog spd(project_,selSeq,this);
	if (spd.exec()==QDialog::Accepted) {
		if (!spd.structureFile().isEmpty())
			selSeq->structureFile=spd.structureFile();
		selSeq->comment=spd.comment();
		updateGoToTool(); // in case the name was changed
	}
}


void SeqEditMainWin::editFind()
{
	if (project_->residueSelection->size() != 1) return;
	QString res = project_->residueSelection->selectedResidues(0) ;
	searchTool_->setSearchText(res);
	
}

void SeqEditMainWin::setupAlignmentActions()
{
	alignAllAction->setEnabled(project_->sequences.size() >= 2);
	alignSelectionAction->setEnabled(project_->sequenceSelection->size() >= 2);
	if (NULL != alignmentProc_){
		alignStopAction->setEnabled(alignmentProc_->state() == QProcess::Running);
	}
}

void SeqEditMainWin::alignmentAll()
{
	alignAll=true;
	startAlignment();
}

void SeqEditMainWin::alignmentSelection()
{
	alignAll=false;
	startAlignment();
}

void SeqEditMainWin::alignmentStop()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	if (NULL != alignmentProc_){
		alignmentProc_->kill();
		// FIXME clean up ??
		alignAllAction->setEnabled(true);
		alignStopAction->setEnabled(false);
	}
}

void SeqEditMainWin::alignmentStarted()
{
	qDebug() << trace.header() << "seqEditMainWin::alignmentStarted()";
	statusBar()->showMessage("Alignment running");
	alignAllAction->setEnabled(false);
	alignStopAction->setEnabled(true);
}

void SeqEditMainWin::alignmentReadyReadStdOut()
{
	//qDebug() << trace.header() << "SeqEditMainWin::alignmentReadyReadStdOut()";
	QString msg  = QString(alignmentProc_->readAllStandardOutput());
	mw->addMessage(msg);
}

void SeqEditMainWin::alignmentReadyReadStdErr()
{
	qDebug() << trace.header() << "SeqEditMainWin::alignmentReadyReadStdErr()";
	QString msg  = QString(alignmentProc_->readAllStandardError());
	mw->addMessage(msg,MessageWin::Error);
}

void SeqEditMainWin::alignmentFinished(int exitCode,QProcess::ExitStatus exitStatus)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << " exitCode=" << exitCode << " exitStatus=" << exitStatus;;
	if (exitStatus == 0 && exitCode == 0){
		QFile f(alignmentFileOut_->fileName());
		if (alignmentProc_->state() == QProcess::NotRunning && f.exists()){
			statusBar()->showMessage("Alignment finished");
			// delete alignmentFileIn_; // FIXME reinstate
			// alignmentFileIn_=NULL;
			if (alignAll)
				readNewAlignment(true);
			else{
				readNewAlignment(false);
			}
		}
	}
	else{
		QString msg = "Alignment not completed";
		if (exitCode == 9) // FIXME linux specific
			msg += " (user interrupted)";
		statusBar()->showMessage(msg);
	}
	
	alignAllAction->setEnabled(true);
	alignStopAction->setEnabled(false);
}

void SeqEditMainWin::setupColourMapMenu()
{
	colourMapMenu->clear();
	int dType = project_->sequenceDataType();
	if (dType == SequenceFile::Proteins){
		for (int a=0;a<settingsProteinColourMapActions.size();a++)
			colourMapMenu->addAction(settingsProteinColourMapActions.at(a));
	}
	else if (dType == SequenceFile::DNA){
		for (int a=0;a<settingsDNAColourMapActions.size();a++)
			colourMapMenu->addAction(settingsDNAColourMapActions.at(a));
	}
}

void SeqEditMainWin::settingsEditorFont()
{
	QFont currFont = se->editorFont();
	QFontDialog fd(currFont, this);
	connect(&fd,SIGNAL(currentFontChanged(const QFont &)),se,SLOT(setEditorFont(const QFont &)));
	if (fd.exec()==QDialog::Rejected) {
		se->setEditorFont(currFont);
	}
}

//
//
//

void SeqEditMainWin::setupAnnotationMenu()
{
	annotationConsensusAction->setEnabled(project_->aligned());
}

void SeqEditMainWin::annotationConsensus()
{
	//Consensus c(&(project_->sequences));
	//c.calculate();
}

//
//
//
void SeqEditMainWin::settingsViewTool(QAction *a)
{
	if (a->text()=="Standard")
		se->setResidueView(SequenceEditor::StandardView);
	else if (a->text() == "Inverted")
		se->setResidueView(SequenceEditor::InvertedView);
	else if (a->text() == "Solid")
		se->setResidueView(SequenceEditor::SolidView);
}

void SeqEditMainWin::settingsColourMap(QAction *a)
{
	if (a->text()=="Physico-chemical")
		se->setColourMap(SequenceEditor::PhysicoChemicalMap);
	else if (a->text()=="RasMol")
		se->setColourMap(SequenceEditor::RasMolMap);
	else if (a->text()=="Taylor")
		se->setColourMap(SequenceEditor::TaylorMap);
	else if (a->text()=="Simple DNA")
		se->setColourMap(SequenceEditor::StandardDNAMap);
	else if (a->text()=="None"){
		if (project_->sequenceDataType() == SequenceFile::DNA)
			se->setColourMap(SequenceEditor::MonoDNAMap);
		else if (project_->sequenceDataType() == SequenceFile::Proteins)
			se->setColourMap(SequenceEditor::MonoMap);
	}
}

void SeqEditMainWin::settingsAlignmentToolClustalO()
{
	if (project_->alignmentTool()->name() != "clustalo"){
		project_->setAlignmentTool("clustalo");
		settingsAlignmentToolPropertiesAction->setText("clustalo");
	}
}

void SeqEditMainWin::settingsAlignmentToolMUSCLE()
{
	if (project_->alignmentTool()->name() != "MUSCLE"){
		project_->setAlignmentTool("MUSCLE");
		settingsAlignmentToolPropertiesAction->setText("MUSCLE");
	}
}

void SeqEditMainWin::settingsAlignmentToolMAFFT()
{
	if (project_->alignmentTool()->name() != "MAFFT"){
		project_->setAlignmentTool("MAFFT");
		settingsAlignmentToolPropertiesAction->setText("MAFFT");
	}
}

void SeqEditMainWin::settingsAlignmentToolProperties()
{
		AlignmentToolDlg ad(project_->alignmentTool(),this); 
		ad.exec();
}


void SeqEditMainWin::settingsSaveAppDefaults()
{
	app->saveDefaultSettings(project_);
}


void SeqEditMainWin::helpHelp(){
	app->showHelp("");
}

void SeqEditMainWin::helpAbout(){
	app->showAboutDialog(this);
}

void SeqEditMainWin::test1()
{
}

void SeqEditMainWin::search(const QString &txt)
{
	qDebug()<< trace.header(__PRETTY_FUNCTION__);
	int nmatches = project_->search(txt);
	statusBar()->showMessage("Found " + QString::number(nmatches)+ " matches");
	if (nmatches == 0){
		return;
	}
	bool showAll=true;
	if (nmatches > 100){
			// FIXME
	}
	QList<SearchResult*> results =project_->searchResults();
	searchTool_->setSearchResults(results);
	if (showAll)
		se->setSearchResults(results);
	se->goToSearchResult(0);
}


void SeqEditMainWin::createContextMenu(const QPoint &)
{
	QMenu *cm = new QMenu(this);
	
	setupEditActions();
	
	cm->addAction(undoAction);
	cm->addAction(redoAction);
	cm->addSeparator();
	
	cm->addAction(cutAction);
	cm->addAction(pasteAction);
	cm->addSeparator();
	
	if (project_->sequenceSelection->size() > 0){
		cm->addAction(renameSequenceAction);
		cm->addAction(sequencePropertiesAction);
		cm->addSeparator();
	}
	
	if (project_->residueSelection->size() > 0){
		cm->addAction(findAction);
		cm->addSeparator();
	}
	
	if (project_->sequenceSelection->size() > 0){
		cm->addAction(groupSequencesAction);
		cm->addAction(ungroupSequencesAction);
		cm->addAction(lockAction);
		cm->addAction(unlockAction);
		cm->addAction(hideNonSelectedGroupMembersAction);
		cm->addAction(unhideAllGroupMembersAction);
		cm->addSeparator();
	}
	
	if (project_->residueSelection->size() > 0){
		cm->addAction(excludeAction);
		cm->addAction(removeExcludeAction);
		cm->addSeparator();
	}
	
	if (project_->sequenceSelection->size() > 0){
		createBookmarkAction->setEnabled(false);
		removeBookmarkAction->setEnabled(false);
		if (project_->sequenceSelection->size()==1){
			Sequence *seq = project_->sequenceSelection->itemAt(0);
			if (se->isBookmarked(seq))
				removeBookmarkAction->setEnabled(true);
			else
				createBookmarkAction->setEnabled(true);
		}
		cm->addAction(createBookmarkAction);
		cm->addAction(removeBookmarkAction);
	}
		
	cm->exec(QCursor::pos());
	delete cm;
}

void SeqEditMainWin::updateScrollBars(int startRow,int stopRow,int numRows,int startCol,int stopCol,int numCols)
{
	// Triggered by resizeEvent() in SequenceEditor, ...
	
	qDebug() << trace.header(__PRETTY_FUNCTION__) << startRow << " " << stopRow << " " << numRows << " " << startCol << " " << stopCol << " " << numCols;
	
	int nvis = stopRow - startRow + 1;
	vscroller_->setMinimum(0);
	vscroller_->setPageStep(nvis);
	vscroller_->setMaximum(numRows-nvis);
	vscroller_->setValue(startRow);
	
	int nviscol = stopCol - startCol + 1;
	hscroller_->setMinimum(0);
	hscroller_->setPageStep(nviscol);
	hscroller_->setMaximum(numCols-nviscol);
	hscroller_->setValue(startCol);

}

void SeqEditMainWin::sequenceSelectionChanged()
{
	// cutAction->setEnabled(!(project_->sequenceSelection->empty())); // FIXME disabled for the moment
	groupSequencesAction->setEnabled(!(project_->sequenceSelection->empty()));
	// If the selection contains any grouped sequences then we can ungroup
	ungroupSequencesAction->setEnabled(false);
	for (int s=0;s<project_->sequenceSelection->size();s++){
		if (project_->sequenceSelection->itemAt(s)->group != NULL)
			ungroupSequencesAction->setEnabled(true);
	}
}

void SeqEditMainWin::residueSelectionChanged()
{
	qDebug() << trace.header() << "SeqEditMainWin::residueSelectionChanged()";
	// We can only cut insertions
	// So check that the selection is insertions only
	cutAction->setEnabled(project_->residueSelection->isInsertionsOnly());
}

void SeqEditMainWin::clearSearch(){
	
}

//
// SeqEditMainWin - private members
// 

void SeqEditMainWin::init()
{
	lastImportedFile="";
	alignmentProc_=NULL;
	alignmentFileOut_=alignmentFileIn_=NULL;
}
			
void SeqEditMainWin::createActions()
{
	// File actions
	
	newProjectAction = new QAction( tr("&New project"), this);
	newProjectAction->setStatusTip(tr("Open a new sequence alignment project"));
	newProjectAction->setShortcut(QKeySequence::New);
	addAction(newProjectAction);
	connect(newProjectAction, SIGNAL(triggered()), this, SLOT(fileNewProject()));
	
	openProjectAction = new QAction( tr("&Open project ..."), this);
	openProjectAction->setStatusTip(tr("Open an existing project"));
	openProjectAction->setShortcut(QKeySequence::Open);
	addAction(openProjectAction);
	connect(openProjectAction, SIGNAL(triggered()), this, SLOT(fileOpenProject()));
	openProjectAction->setEnabled(true);
	
	saveProjectAction = new QAction( tr("&Save project"), this);
	saveProjectAction->setStatusTip(tr("Save the project"));
	saveProjectAction->setShortcut(QKeySequence::Save);
	addAction(saveProjectAction);
	connect(saveProjectAction, SIGNAL(triggered()), this, SLOT(fileSaveProject()));
	
	saveProjectAsAction = new QAction( tr("Save project as ..."), this);
	saveProjectAsAction->setStatusTip(tr("Save the project under a new name"));
	saveProjectAsAction->setShortcut(QKeySequence::SaveAs);
	addAction(saveProjectAsAction);
	connect(saveProjectAsAction, SIGNAL(triggered()), this, SLOT(fileSaveProjectAs()));
	
	importAction = new QAction( tr("&Import sequences ..."), this);
	importAction->setStatusTip(tr("Import a sequence file"));
	addAction(importAction);
	connect(importAction, SIGNAL(triggered()), this, SLOT(fileImport()));
	
	exportFASTAAction = new QAction( tr("&Export as FASTA ..."), this);
	exportFASTAAction->setStatusTip(tr("Export all project sequences in FASTA format"));
	addAction(exportFASTAAction);
	connect(exportFASTAAction, SIGNAL(triggered()), this, SLOT(fileExportFASTA()));
	
	exportClustalWAction = new QAction( tr("&Export as ClustalW ..."), this);
	exportClustalWAction->setStatusTip(tr("Export all project sequences in ClustalW format"));
	addAction(exportClustalWAction);
	connect(exportClustalWAction, SIGNAL(triggered()), this, SLOT(fileExportClustalW()));
	
	printAction = new QAction( tr("&Print ..."), this);
	printAction->setStatusTip(tr("Print current "));
	addAction(printAction);
	connect(printAction, SIGNAL(triggered()), this, SLOT(filePrint()));
	
	closeAction = new QAction( tr("Close"), this);
	closeAction->setStatusTip(tr("Close the project "));
	closeAction->setShortcut(QKeySequence::Close);
	addAction(closeAction);
	connect(closeAction, SIGNAL(triggered()), this, SLOT(fileClose()));
	
	quitAction = new QAction( tr("&Quit"), this);
	quitAction->setStatusTip(tr("Quit"));
	quitAction->setShortcut(QKeySequence::Quit);
	addAction(quitAction);
	connect(quitAction, SIGNAL(triggered()), app, SLOT(closeAllWindows()));
	
	// Edit actions
	undoAction = new QAction( tr("&Undo"), this);
	undoAction->setStatusTip(tr("Undo"));
	undoAction->setShortcut(QKeySequence::Undo);
	addAction(undoAction);
	connect(undoAction, SIGNAL(triggered()), se, SLOT(undo()));
	undoAction->setEnabled(false);
	
	redoAction = new QAction( tr("&Redo"), this);
	redoAction->setStatusTip(tr("Redo"));
	redoAction->setShortcut(QKeySequence::Redo);
	addAction(redoAction);	
	connect(redoAction, SIGNAL(triggered()), se, SLOT(redo()));
	redoAction->setEnabled(false);
	
	cutAction = new QAction( tr("Cu&t"), this);
	cutAction->setStatusTip(tr("Cut"));
	cutAction->setShortcut(QKeySequence::Cut);
	addAction(cutAction);
	connect(cutAction, SIGNAL(triggered()), se, SLOT(cutSelection()));
	cutAction->setEnabled(false);
	
	copyAction = new QAction( tr("Copy (to system clipboard)"), this);
	copyAction->setStatusTip(tr("Copy"));
	addAction(copyAction);
	connect(copyAction, SIGNAL(triggered()), this, SLOT(editCopy()));
	copyAction->setEnabled(false);
	
	pasteAction = new QAction( tr("Paste"), this);
	pasteAction->setStatusTip(tr("Paste"));
	pasteAction->setShortcut(QKeySequence::Paste);
	addAction(pasteAction);
	connect(pasteAction, SIGNAL(triggered()), se, SLOT(pasteClipboard()));
	pasteAction->setEnabled(false);
	
	findAction = new QAction( tr("Find"), this);
	findAction->setStatusTip(tr("Find selected residues"));
	findAction->setShortcut(QKeySequence::Find);
	addAction(findAction);
	connect(findAction, SIGNAL(triggered()), this, SLOT(editFind()));
	findAction->setEnabled(false);
	
	groupSequencesAction = new QAction( tr("Group sequences"), this);
	groupSequencesAction->setStatusTip(tr("Group the selected sequences"));
	addAction(groupSequencesAction);
	connect(groupSequencesAction, SIGNAL(triggered()), se, SLOT(groupSequences()));
	groupSequencesAction->setEnabled(false);
	
	ungroupSequencesAction = new QAction( tr("Ungroup sequences"), this);
	ungroupSequencesAction->setStatusTip(tr("Ungroup the selected sequences"));
	addAction(ungroupSequencesAction);
	connect(ungroupSequencesAction, SIGNAL(triggered()), se, SLOT(ungroupSequences()));
	ungroupSequencesAction->setEnabled(false);
	
	ungroupAllAction = new QAction( tr("Ungroup all sequences"), this);
	ungroupAllAction->setStatusTip(tr("Ungroup all sequences"));
	addAction(ungroupAllAction);
	connect(ungroupAllAction, SIGNAL(triggered()), se, SLOT(ungroupAllSequences()));
	ungroupAllAction->setEnabled(true);
	
	lockAction = new QAction( tr("Lock groups(s)"), this);
	lockAction->setStatusTip(tr("Lock selected group(s)"));
	//lockAction->setIcon(QIcon(lock_xpm));
	addAction(lockAction);
	connect(lockAction, SIGNAL(triggered()), se, SLOT(lockSelectedGroups()));
	
	unlockAction = new QAction( tr("Unlock groups(s)"), this);
	unlockAction->setStatusTip(tr("Unlock selected group(s)"));
	addAction(unlockAction);
	connect(unlockAction, SIGNAL(triggered()), se, SLOT(unlockSelectedGroups()));
	
	hideNonSelectedGroupMembersAction = new QAction( tr("Hide non-selected"), this);
	hideNonSelectedGroupMembersAction ->setStatusTip(tr("Hide non-selected sequences in the group"));
	addAction(hideNonSelectedGroupMembersAction);
	connect(hideNonSelectedGroupMembersAction, SIGNAL(triggered()), se, SLOT(hideNonSelectedGroupMembers()));
	
	unhideAllGroupMembersAction = new QAction( tr("Unhide all in group"), this);
	unhideAllGroupMembersAction ->setStatusTip(tr("Hide non-selected sequences in the group"));
	addAction(unhideAllGroupMembersAction);
	connect(unhideAllGroupMembersAction, SIGNAL(triggered()), se, SLOT(unhideAllGroupMembers()));
	
	unhideAllAction = new QAction( tr("Unhide all hidden sequences"), this);
	unhideAllAction ->setStatusTip(tr("Unhide all hidden sequences"));
	addAction(unhideAllAction);
	connect(unhideAllAction, SIGNAL(triggered()), se, SLOT(unhideAll()));
	
	excludeAction = new QAction( tr("Exclude residues"), this);
	excludeAction->setStatusTip(tr("Exclude residues"));
	addAction(excludeAction);
	connect(excludeAction, SIGNAL(triggered()), se, SLOT(excludeSelectedResidues()));
	
	removeExcludeAction = new QAction( tr("Remove exclusion"), this);
	removeExcludeAction->setStatusTip(tr("Remove exclusion"));
	addAction(removeExcludeAction);
	connect(removeExcludeAction, SIGNAL(triggered()), se, SLOT(removeExclusions()));
	
	readOnlyAction = new QAction( tr("Read only"), this);
	readOnlyAction->setStatusTip(tr("Set the alignment to read only"));
	addAction(readOnlyAction);
	readOnlyAction->setCheckable(true);
	readOnlyAction->setChecked(false);
	connect(readOnlyAction, SIGNAL(triggered()), this, SLOT(editReadOnly()));
	
	renameSequenceAction = new QAction( tr("Rename sequence"), this);
	renameSequenceAction->setStatusTip(tr("Rename the selected sequence"));
	addAction(renameSequenceAction);
	connect(renameSequenceAction, SIGNAL(triggered()), this, SLOT(renameSequence()));
	renameSequenceAction->setEnabled(false);

	sequencePropertiesAction = new QAction( tr("Properties"), this);
	sequencePropertiesAction->setStatusTip(tr("Sequence properties dialog"));
	addAction(sequencePropertiesAction);
	connect(sequencePropertiesAction, SIGNAL(triggered()), this, SLOT(sequenceProperties()));
	sequencePropertiesAction->setEnabled(false);
	
	// Alignment actions
	alignAllAction = new QAction( tr("&Align all"), this);
	alignAllAction->setStatusTip(tr("Run alignment on all sequences"));
	addAction(alignAllAction);
	connect(alignAllAction, SIGNAL(triggered()), this, SLOT(alignmentAll()));
	alignAllAction->setEnabled(false);
	alignAllAction->setIcon(QIcon(":/images/go.png"));
	
	alignSelectionAction = new QAction( tr("&Align selection"), this);
	alignSelectionAction->setStatusTip(tr("Run alignment on the current selection"));
	addAction(alignSelectionAction);
	connect(alignSelectionAction, SIGNAL(triggered()), this, SLOT(alignmentSelection()));
	alignSelectionAction->setEnabled(false);
	
	alignStopAction = new QAction( tr("Stop alignment"), this);
	alignStopAction->setStatusTip(tr("Stop the running alignment"));
	addAction(alignStopAction);
	connect(alignStopAction, SIGNAL(triggered()), this, SLOT(alignmentStop()));
	alignStopAction->setEnabled(false);
	alignStopAction->setIcon(QIcon(":/images/stop.png"));
	
	// Settings actions
	settingsEditorFontAction = new QAction( tr("Editor font"), this);
	settingsEditorFontAction->setStatusTip(tr("Choose the font used in the sequence editor"));
	addAction(settingsEditorFontAction);
	connect(settingsEditorFontAction, SIGNAL(triggered()), this, SLOT(settingsEditorFont()));

	
	settingsAlignmentToolClustalOAction = new QAction( tr("ClustalO"), this);
	settingsAlignmentToolClustalOAction->setStatusTip(tr("Select Clustal Omega"));
	addAction(settingsAlignmentToolClustalOAction);
	connect(settingsAlignmentToolClustalOAction, SIGNAL(triggered()), this, SLOT(settingsAlignmentToolClustalO()));
	settingsAlignmentToolClustalOAction->setCheckable(true);
	settingsAlignmentToolClustalOAction->setChecked(project_->alignmentTool()->name()=="clustalo");
	settingsAlignmentToolClustalOAction->setEnabled(app->alignmentToolAvailable("clustalo"));
	
	settingsAlignmentToolMUSCLEAction = new QAction( tr("MUSCLE"), this);
	settingsAlignmentToolMUSCLEAction->setStatusTip(tr("Select MUSCLE"));
	addAction(settingsAlignmentToolMUSCLEAction);
	connect(settingsAlignmentToolMUSCLEAction, SIGNAL(triggered()), this, SLOT(settingsAlignmentToolMUSCLE()));
	settingsAlignmentToolMUSCLEAction->setCheckable(true);
	settingsAlignmentToolMUSCLEAction->setChecked(project_->alignmentTool()->name()=="MUSCLE");
	settingsAlignmentToolMUSCLEAction->setEnabled(app->alignmentToolAvailable("MUSCLE"));
	
	settingsAlignmentToolMAFFTAction = new QAction( tr("MAFFT"), this);
	settingsAlignmentToolMAFFTAction->setStatusTip(tr("Select MAFFT"));
	addAction(settingsAlignmentToolMAFFTAction);
	connect(settingsAlignmentToolMAFFTAction, SIGNAL(triggered()), this, SLOT(settingsAlignmentToolMAFFT()));
	settingsAlignmentToolMAFFTAction->setCheckable(true);
	settingsAlignmentToolMAFFTAction->setChecked(project_->alignmentTool()->name()=="MAFFT");
	settingsAlignmentToolMAFFTAction->setEnabled(app->alignmentToolAvailable("MAFFT"));
	
	QActionGroup *ag = new QActionGroup(this);
	ag->setExclusive(true);
	ag->addAction(settingsAlignmentToolClustalOAction);
	ag->addAction(settingsAlignmentToolMUSCLEAction);
	ag->addAction(settingsAlignmentToolMAFFTAction);
	
	settingsAlignmentToolPropertiesAction = new QAction( project_->alignmentTool()->name(), this);
	settingsAlignmentToolPropertiesAction->setStatusTip(tr("Alignment tool properties"));
	addAction(settingsAlignmentToolPropertiesAction);
	connect(settingsAlignmentToolPropertiesAction, SIGNAL(triggered()), this, SLOT(settingsAlignmentToolProperties()));
	
	settingsSaveAppDefaultsAction = new QAction( tr("Save as application defaults"), this);
	settingsSaveAppDefaultsAction->setStatusTip(tr("Save settings as application defaults"));
	addAction(settingsSaveAppDefaultsAction);
	connect(settingsSaveAppDefaultsAction, SIGNAL(triggered()), this, SLOT(settingsSaveAppDefaults()));
	
	// Annotations 
	
	annotationConsensusAction = new QAction( tr("Consensus sequence"), this);
	annotationConsensusAction->setStatusTip(tr("Enable display and calculation of the consensus sequence"));
	addAction(annotationConsensusAction);
	connect(annotationConsensusAction, SIGNAL(triggered()), this, SLOT(annotationConsensus()));;
	
	// Help actions
	helpAction = new QAction( tr("&Help"), this);
	helpAction->setStatusTip(tr("Help"));
	addAction(helpAction);
	connect(helpAction, SIGNAL(triggered()), this, SLOT(helpHelp()));
	
	aboutAction = new QAction( tr("About"), this);
	aboutAction->setStatusTip(tr("About"));
	addAction(aboutAction);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(helpAbout()));
	

	// Miscellaneous actions
	createBookmarkAction = new QAction( tr("Create bookmark"), this);
	createBookmarkAction->setStatusTip(tr("Bookmark the selected sequence"));
	addAction(createBookmarkAction);
	connect(createBookmarkAction, SIGNAL(triggered()), se, SLOT(createBookmark()));
	
	removeBookmarkAction = new QAction( tr("Remove bookmark"), this);
	removeBookmarkAction->setStatusTip(tr("Remove bookmark from the selected sequence"));
	addAction(removeBookmarkAction);
	connect(removeBookmarkAction, SIGNAL(triggered()), se, SLOT(removeBookmark()));
	
	nextBookmarkAction = new QAction( tr("Next bookmark"), this);
	nextBookmarkAction->setStatusTip(tr("Move to the next bookmarked sequence"));
	nextBookmarkAction->setIcon(QIcon(":/images/go-next.png"));
	addAction(nextBookmarkAction);
	connect(nextBookmarkAction, SIGNAL(triggered()), se, SLOT(moveToNextBookmark()));
	
	prevBookmarkAction = new QAction( tr("Previous bookmark"), this);
	prevBookmarkAction->setStatusTip(tr("Move to the previous bookmarked sequence"));
	prevBookmarkAction->setIcon(QIcon(":/images/go-previous.png"));
	addAction(prevBookmarkAction);
	connect(prevBookmarkAction, SIGNAL(triggered()), se, SLOT(moveToPreviousBookmark()));
	
	ag = new QActionGroup(this);
	ag->setExclusive(true);
	QAction *colourMapAction = new QAction("Physico-chemical",this);
	colourMapAction->setCheckable(true);
	colourMapAction->setChecked(true);
	ag->addAction(colourMapAction);
	settingsProteinColourMapActions.append(colourMapAction);
	
	colourMapAction =new QAction("RasMol",this);
	colourMapAction->setCheckable(true);
	ag->addAction(colourMapAction);
	settingsProteinColourMapActions.append(colourMapAction);
	
	colourMapAction =new QAction("Taylor",this);
	colourMapAction->setCheckable(true);
	ag->addAction(colourMapAction);
	settingsProteinColourMapActions.append(colourMapAction);
	
	colourMapAction =new QAction("None",this);
	colourMapAction->setCheckable(true);
	ag->addAction(colourMapAction);
	settingsProteinColourMapActions.append(colourMapAction);
	
	ag = new QActionGroup(this);
	ag->setExclusive(true);
	colourMapAction = new QAction("Simple DNA",this);
	colourMapAction->setCheckable(true);
	colourMapAction->setChecked(true);
	ag->addAction(colourMapAction);
	settingsDNAColourMapActions.append(colourMapAction);
	
	colourMapAction = new QAction("None",this);
	colourMapAction->setCheckable(true);
	ag->addAction(colourMapAction);
	settingsDNAColourMapActions.append(colourMapAction);
	
	//nextSearchResult_ = new QAction("Next",this);
	//prevSearchResult_ = new QAction("Previous",this);
	
	testAction = new QAction( tr("Test1"), this);
	testAction->setStatusTip(tr("Test1"));
	addAction(testAction);
	connect(testAction, SIGNAL(triggered()), this, SLOT(test1()));
}

void SeqEditMainWin::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newProjectAction);
	fileMenu->addAction(openProjectAction);
	fileMenu->addAction(saveProjectAction);
	fileMenu->addAction(saveProjectAsAction);
	fileMenu->addSeparator();
	fileMenu->addAction(importAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exportFASTAAction);
	fileMenu->addAction(exportClustalWAction);
	fileMenu->addSeparator();
	fileMenu->addAction(printAction);
	fileMenu->addSeparator();
	fileMenu->addAction(closeAction);
	fileMenu->addAction(quitAction);
	
	editMenu = menuBar()->addMenu(tr("&Edit"));
	connect(editMenu,SIGNAL(aboutToShow()),this,SLOT(setupEditActions()));
	
	editMenu->addAction(undoAction);
	editMenu->addAction(redoAction);
	editMenu->addSeparator();
	
	editMenu->addAction(cutAction);
	editMenu->addAction(copyAction);
	editMenu->addAction(pasteAction);
	editMenu->addSeparator();
	
	editMenu->addAction(findAction);
	editMenu->addSeparator();
	
	editMenu->addAction(groupSequencesAction);
	editMenu->addAction(ungroupSequencesAction);
	editMenu->addAction(ungroupAllAction);
	editMenu->addAction(lockAction);
	editMenu->addAction(unlockAction);
	editMenu->addAction(hideNonSelectedGroupMembersAction);
	editMenu->addAction(unhideAllGroupMembersAction);
	editMenu->addAction(unhideAllAction);
	editMenu->addSeparator();

	editMenu->addAction(excludeAction);
	editMenu->addAction(removeExcludeAction);
	editMenu->addSeparator();
	
	editMenu->addAction(readOnlyAction);
	
	alignmentMenu = menuBar()->addMenu(tr("Alignment"));
	connect(alignmentMenu,SIGNAL(aboutToShow()),this,SLOT(setupAlignmentActions()));
	alignmentMenu->addAction(alignAllAction);
	alignmentMenu->addAction(alignSelectionAction);
	alignmentMenu->addAction(alignStopAction);
	
	//annotationMenu = menuBar()->addMenu(tr("Annotations"));
	//connect(annotationMenu,SIGNAL(aboutToShow()),this,SLOT(setupAnnotationMenu()));
	//annotationMenu->addAction(annotationConsensusAction);
	
	//
	// Settings
	//
	settingsMenu = menuBar()->addMenu(tr("Settings"));
	
	settingsMenu->addAction(settingsEditorFontAction);
	
	QMenu* viewToolMenu = settingsMenu->addMenu(tr("Residue view"));
	QActionGroup *agView = new QActionGroup(this);
	agView->setExclusive(true);
	
	QAction *viewAction  = viewToolMenu->addAction("Standard");
	viewAction ->setStatusTip(tr("Standard residue view"));
	viewAction ->setCheckable(true);
	viewAction ->setChecked(true);
	agView->addAction(viewAction );
	settingsViewActions.append(viewAction);
	
	viewAction = viewToolMenu->addAction("Inverted");
	viewAction->setStatusTip(tr("Inverted residue view"));
	viewAction->setCheckable(true);
	agView->addAction(viewAction);
	settingsViewActions.append(viewAction);
	
	viewAction = viewToolMenu->addAction("Solid");
	viewAction ->setStatusTip(tr("Solid residue view (no label)"));
	viewAction ->setCheckable(true);
	agView->addAction(viewAction );
	settingsViewActions.append(viewAction);
	
	connect(viewToolMenu,SIGNAL(triggered(QAction*)),this,SLOT(settingsViewTool(QAction *)));
	
	colourMapMenu = settingsMenu->addMenu(tr("Residue colour map"));
	connect(colourMapMenu,SIGNAL(triggered(QAction*)),this,SLOT(settingsColourMap(QAction *)));
	connect(colourMapMenu,SIGNAL(aboutToShow()),this,SLOT(setupColourMapMenu()));
	
	settingsMenu->addSeparator();
	
	QMenu* alignmentToolMenu = settingsMenu->addMenu(tr("Alignment tool"));
	alignmentToolMenu->addAction(settingsAlignmentToolClustalOAction);
	alignmentToolMenu->addAction(settingsAlignmentToolMUSCLEAction);
	alignmentToolMenu->addAction(settingsAlignmentToolMAFFTAction);
	
	settingsMenu->addAction(settingsAlignmentToolPropertiesAction);
	
	settingsMenu->addSeparator();
	settingsMenu->addAction(settingsSaveAppDefaultsAction);
	
	menuBar()->addSeparator();
	
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(helpAction);
	helpMenu->addAction(aboutAction);
	
	for (int a=0;a<settingsDNAColourMapActions.size();a++)
		colourMapMenu->addAction(settingsDNAColourMapActions.at(a));
	
	QMenu *testMenu=menuBar()->addMenu("Test");
	testMenu->addAction(testAction);
	
	
}


void SeqEditMainWin::createToolBars()
{
	seqEditTB =addToolBar("Sequence editor tools");
	
	goToTool_ = new GoToTool(this);
	seqEditTB->addWidget(goToTool_);
	
	seqEditTB->addAction(prevBookmarkAction);
	seqEditTB->addAction(nextBookmarkAction);
	
	seqEditTB->addSeparator();
	
	seqEditTB->addAction(alignAllAction);
	seqEditTB->addAction(alignStopAction);
	
	seqEditTB->addSeparator();
	
	searchTool_ = new SearchTool(this);
	seqEditTB->addWidget(searchTool_);
	connect(searchTool_,SIGNAL(search(const QString &)),this,SLOT(search(const QString &)));
	connect(searchTool_,SIGNAL(goToSearchResult(int)),se,SLOT(goToSearchResult(int)));
	
	QWidget *separator = new QWidget(this);
	separator->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	seqEditTB->addWidget(separator);

	QLabel *info = new QLabel(this);
	info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
 	info->setLineWidth(1);
	info->setFixedWidth(fontMetrics().width('W')*16);
	
	seqEditTB->addWidget(info);
	connect(se,SIGNAL(info(const QString &)),info,SLOT(setText(const QString &)));
	
	//seqEditTB->addAction(lockAction);
	//tb->setToggleButton(true);
	//connect(tb,SIGNAL(toggled(bool)),se,SLOT(lockMode(bool)));
	
	
}

void SeqEditMainWin::createStatusBar()
{
	
}

void SeqEditMainWin::startAlignment()
{
	if (NULL != alignmentProc_){
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "state=" << alignmentProc_->state();
		if (alignmentProc_->state() != QProcess::NotRunning)
			alignmentProc_->close();
		delete alignmentProc_;
		alignmentProc_=NULL;
	}
	
	alignmentProc_ = new QProcess(this);
	connect(alignmentProc_,SIGNAL(started()),this,SLOT(alignmentStarted()));
	connect(alignmentProc_,SIGNAL(readyReadStandardOutput()),this,SLOT(alignmentReadyReadStdOut()));
	connect(alignmentProc_,SIGNAL(readyReadStandardError()),this,SLOT(alignmentReadyReadStdErr()));
	connect(alignmentProc_,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(alignmentFinished(int,QProcess::ExitStatus)));
	
	QString exec;
	QStringList args;
	
	QFileInfo fiin(app->applicationTmpPath(),"tweakseq.in.XXXXXX.fa");
	alignmentFileIn_ = new QTemporaryFile(fiin.absoluteFilePath());
	alignmentFileIn_->open();
	alignmentFileIn_->close();
	
	QFileInfo fiout(app->applicationTmpPath(),"tweakseq.out.XXXXXX.fa");
	alignmentFileOut_ = new QTemporaryFile(fiout.absoluteFilePath());
	alignmentFileOut_->open();
	alignmentFileOut_->close();
	
	QString fin  = alignmentFileIn_->fileName();
	QString fout = alignmentFileOut_->fileName();
	if (alignAll){
		project_->exportFASTA(fin,true);
	}
	else{
		project_->exportSelectionFASTA(fin,true);
	}
	
	project_->alignmentTool()->makeCommand(fin,fout,exec,args);
	if (project_->alignmentTool()->usesStdOut()){
		alignmentProc_->setStandardOutputFile(fout);
		qDebug() <<  trace.header(__PRETTY_FUNCTION__) << "stdout redirected";
	}
	qDebug() <<  trace.header(__PRETTY_FUNCTION__) << exec << args;
	alignmentProc_->start(exec,args);
	alignAllAction->setEnabled(false);
	alignStopAction->setEnabled(true);
}

void SeqEditMainWin::readNewAlignment(bool isFullAlignment)
{
	project_->readNewAlignment(alignmentFileOut_->fileName(),isFullAlignment);
	se->updateViewport();
}

void SeqEditMainWin::printRes( QPainter* p,QChar r,int x,int y)
{

	QColor txtColor;
	QChar rNoFlags;
	QPen oldPen;
	
	oldPen = p->pen();
	rNoFlags = r.unicode() & REMOVE_FLAGS;
	
	switch (rNoFlags.toLatin1()){
		case 'D': case 'E': case 'S': case 'T':// red 
			txtColor.setRgb(255,0,0);
			break; 
		case 'R': case 'K': case 'H': // blue
			txtColor.setRgb(0,0,202);
			break;
		case 'Q': case 'N': // purple
			txtColor.setRgb(255,0,255);
			break; 
		case 'M': case 'C': // yellow
			txtColor.setRgb(255,255,0);
			break; 
		case 'A': case 'I': case 'L' : case 'V' : case 'G': case '-': // black			
			txtColor.setRgb(0,0,0);
			break; 
		case 'Y': case 'F': case 'W': // orange
			txtColor.setRgb(254,172,0);
			break; 
		case 'P': // green
			txtColor.setRgb(0,255,0);
			break;
	}
	
	p->setPen(txtColor);
	p->drawText( QPoint(x, y), r);
	
	if (r.unicode() & EXCLUDE_CELL){
		txtColor.setRgb(255,0,0);
		p->setPen(txtColor);
		//p->drawLine(0,0,w-2,h-2); // X marks the spot ...
		//p->drawLine(0,2,2,h-2);
	}
	
	p->setPen(oldPen);
}

void SeqEditMainWin::updateGoToTool()
{
	QList<Sequence *> &sequences = project_->sequences.sequences();
	QStringList labels;
	for (int s=0;s<sequences.count();s++)
		labels.append(sequences.at(s)->label);
	goToTool_->setCompleterModel(labels);
}

void SeqEditMainWin::updateSettingsActions()
{
	// Called after loading a Project
	settingsAlignmentToolPropertiesAction->setText(project_->alignmentTool()->name());
	
	int view = se->residueView();
	for (int a=0;a<settingsViewActions.size();a++)
		settingsViewActions.at(a)->setChecked(false);
	for (int a=0;a<settingsViewActions.size();a++){
		QAction *va = settingsViewActions.at(a);
		if (va->text()=="Standard" && view == SequenceEditor::StandardView){
			va->setChecked(true);
			break;
		}
		else if (va->text() == "Inverted" && view == SequenceEditor::InvertedView){
			va->setChecked(true);
			break;
		}
		else if (va->text() == "Solid" && view == SequenceEditor::SolidView){
			va->setChecked(true);
			break;
		}
	}
	
	int colourMap=se->colourMap();
	if (project_->sequenceDataType() == SequenceFile::DNA){
		for (int a=0;a<settingsDNAColourMapActions.size();a++)
			settingsDNAColourMapActions.at(a)->setChecked(false);
		for (int a=0;a<settingsDNAColourMapActions.size();a++){
			QAction *va = settingsDNAColourMapActions.at(a);
			if (va->text()=="Simple DNA" && colourMap == SequenceEditor::StandardDNAMap){
				va->setChecked(true);
				break;
			}
			if (va->text()=="None" && colourMap == SequenceEditor::MonoDNAMap){
				va->setChecked(true);
				break;
			}
		}
	}
	else if (project_->sequenceDataType() == SequenceFile::Proteins){
		for (int a=0;a<settingsProteinColourMapActions.size();a++)
			settingsProteinColourMapActions.at(a)->setChecked(false);
		for (int a=0;a<settingsProteinColourMapActions.size();a++){
			QAction *va = settingsProteinColourMapActions.at(a);
			if (va->text()=="Physico-chemical" && colourMap == SequenceEditor::PhysicoChemicalMap){
				va->setChecked(true);
				break;
			}
			else if (va->text() == "RasMol" && colourMap == SequenceEditor::RasMolMap){
				va->setChecked(true);
				break;
			}
			else if (va->text() == "Taylor" && colourMap == SequenceEditor::TaylorMap){
				va->setChecked(true);
				break;
			}
			else if (va->text() == "None" && colourMap == SequenceEditor::MonoMap){
				va->setChecked(true);
				break;
			}
		}
	}
}


bool SeqEditMainWin::maybeSave()
{
	if (project_->isModified()) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("tweakseq"),
			tr("The project has been modified.\n"
					"Do you want to save your changes?"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save){
			QString fpathname;
			if (!project_->named()){ // never saved so need a get a project name and path
				fpathname = QFileDialog::getSaveFileName(this, tr("Save project"));
				if (fpathname.isNull()) return false;
			}
			return project_->save(fpathname);
		}
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	return true;
}

void SeqEditMainWin::connectToProject()
{
	connect(project_,SIGNAL(searchResultsCleared()),searchTool_,SLOT(clearSearch()));
}

void SeqEditMainWin::disconnectFromProject()
{
	disconnect(project_,SIGNAL(searchResultsCleared()),searchTool_,SLOT(clearSearch()));
}

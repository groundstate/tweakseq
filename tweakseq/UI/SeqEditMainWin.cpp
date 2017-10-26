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

#include <Q3PaintDeviceMetrics> // FIXME

#include <QColor>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>
#include <QProcess>
#include <QStatusBar>
#include <QSplitter>
#include <QTemporaryFile>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>


#include "Application.h"
#include "ClustalFile.h"
#include "FASTAFile.h"
#include "MessageWin.h"
#include "Project.h"
#include "ResidueSelection.h"
#include "SeqEdit.h"
#include "SeqEditMainWin.h"
#include "Sequence.h"
#include "SequenceSelection.h"

#include "Resources/lock.xpm"
#include "Resources/seqedit.xpm"

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
	init();
	project_=project;
	
	setWindowTitle("tweakseq - " + project->name());
	setGeometry(0,0,800,600);
	setWindowIcon(QIcon(seqedit_xpm));
	
	createActions();
	createMenus();
	createToolBars();
	
	// Hmm what to do about number of columns
	// TO DO This needs to be resizeable
	// TO DO ought to check that creation of widget does not fail because
	// of lack of memory
	
	QSplitter *split = new QSplitter(Qt::Vertical,this,"sew_split");
	split->setMouseTracking(true);
	se = new SeqEdit(project_,split);
	
	mw = new MessageWin(split);
	
	createStatusBar(); // need to connect to ther widgets so do this here
	
	QList<int> wsizes;
	wsizes.append(600);wsizes.append(200);
	split->setSizes(wsizes);
	
	setCentralWidget(split);
	centralWidget()->setMouseTracking(true);
	
	printer = new QPrinter();
	printer->setFullPage(TRUE);
	
	statusBar()->message("Ready");

}

SeqEditMainWin::~SeqEditMainWin(){
	delete printer;
	if (alignmentFileIn_ != NULL) delete alignmentFileIn_;
	if (alignmentFileOut_ != NULL) delete alignmentFileOut_;
}

void SeqEditMainWin::doAlignment(){
}

//
// Public slots
//	


//
// protected
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
	Project *oldProject = project_;
	
	init(); // start in a clean state
	project_=app->createProject();
	project_->setMainWindow(this);
	se->setProject(project_);
	delete oldProject;
	project_->load(fname);
	se->postLoadTidy();
	setWindowTitle("tweakseq - " + project_->name());
}

void SeqEditMainWin::fileSaveProject()
{
	QString fpathname;
	if (!project_->named()){ // never saved so need a get a project name and path
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
	
	QString fname = QFileDialog::getOpenFileName(this,
    tr("Open Sequence"), "./", tr("Sequence Files (*.fasta *.aln *.clustal *.fa)"));
	if (fname.isNull()) return;
	
	//ClustalFile cf("/home/michael/src/da/src/alignmentin.aln");
	// ClustalFile cf("/home/michael/src/seqme/seqme/test/Bcl2l15_12_10.clustal");
	//QString fname = "/home/michael/src/seqme/seqme/test/Bcl2l15_12_10.fasta";
	
	FASTAFile cf(fname);
	QStringList seqnames,seqs,comments;
	if (cf.read(seqnames,seqs,comments)){
		for (int i=0;i<seqnames.size();i++){
			project_->addSequence(seqnames.at(i),seqs.at(i),comments.at(i),fname);
		}
		lastImportedFile=fname;
	}
	else{
	}
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
	double currBlockSpc;
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
	int maxLength;
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
	
	if (printer->setup(this)){
		statusBar()->message("Printing ...");
		
		QPainter p;
		p.begin(printer);
		p.setFont(mainFont);
		QFontMetrics fm = p.fontMetrics();
		Q3PaintDeviceMetrics metrics(printer);
		QSize m = printer->margins();
		
		// Calculate the available vertical space for writing the sequences
		
		int vSpace = metrics.height()-2*m.height()-fm.height()-headerSpc-
			topMargin-bottomMargin;
		
		// We want to divide this up so that sequence blocks are not broken
		// across a page. 
		
		int blockHeight = 
			fm.height()+counterSpc+
			project_->numSequences()*fm.height()+(project_->numSequences()-1)*lineSpc ;
		
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
		int rw=metrics.width() - 2*m.width()-leftMargin-rightMargin - 10*cw;
		wrap=rw/(10*cw); // BUG for weird font/paper choices might not get 10/line
		wrap*=10;
		
		// How many pages ?
		// Need to find the longest sequence 
		
		if (project_->numSequences()==0)
			numPages=1;
		else{
			// Find the longest sequence
			maxLength=project_->getSequence(0,KEEP_FLAGS).length();
			for (i=0;i<project_->numSequences();i++)
				if (project_->getSequence(i,KEEP_FLAGS).length()>maxLength)
					maxLength=project_->getSequence(i,KEEP_FLAGS).length();
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
			p.drawText(x,y,caption()+ " " + QDate::currentDate().toString(),-1);
		  tmp.sprintf("%i/%i",pg,numPages);
			p.drawText(metrics.width()-m.width()-leftMargin-fm.width(tmp),y,tmp,-1);
				
			p.drawRect(m.width(),m.height(),metrics.width()-2*m.width(),
				metrics.height()-2*m.height());
		
			p.drawRect(m.width()+leftMargin,m.height()+topMargin,
				metrics.width()-2*m.width()-leftMargin-rightMargin,
				metrics.height()-2*m.height()-topMargin-bottomMargin);
			
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
					p.drawText(x,yt,tmp,-1);
				}
				p.setFont(mainFont);	
				for (j=0;j<project_->numSequences();j++){
					// Print the sequence name
					x=seqNameX0;
					yt+=fm.height();
					p.drawText(x,yt,project_->getLabelAt(j),-1); // TO DO bounding rect
					// Print the residues
					tmp=project_->getSequence(j,KEEP_FLAGS);
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
		statusBar()->message("Printing completed",2000);
	} // if printer->setup(this)
	else
		statusBar()->message("Printing cancelled",2000);
}

void SeqEditMainWin::fileClose(){
	close();
}

// Edit menu slots
void SeqEditMainWin::setupEditMenu()
{
}

void SeqEditMainWin::editUndo()
{
	project_->undo();
}

void SeqEditMainWin::editRedo()
{
	project_->redo();
}

void SeqEditMainWin::editCut()
{
	project_->cutSelection();
	cutAction->setEnabled(false);
	se->updateViewport();
}

void SeqEditMainWin::editGroupSequences()
{
	statusBar()->clearMessage();
	// Groups the current selection of sequences
	if (!project_->groupSelectedSequences(se->getSequenceGroupColour())){
		statusBar()->showMessage("Grouping unsuccessful");
	}
	se->viewport()->repaint();
}

void SeqEditMainWin::editUngroupSequences()
{
	statusBar()->clearMessage();
	// Groups the current selection of sequences
	if (!project_->ungroupSelectedSequences())
		statusBar()->showMessage("Ungrouping unsuccessful");
	se->viewport()->repaint();
}

void SeqEditMainWin::editLock(){
	project_->lockSelectedGroups(true);
	se->viewport()->repaint();
}

void SeqEditMainWin::editUnlock(){
	project_->lockSelectedGroups(false);
	se->viewport()->repaint();
}

void SeqEditMainWin::editExclude(){
	se->excludeSelection();
}

void SeqEditMainWin::editRemoveExclude(){
	se->removeExcludeSelection();
}
	
void SeqEditMainWin::setupAlignmentMenu()
{
	goAction->setEnabled(project_->numSequences() >= 2);
}

void SeqEditMainWin::alignmentGo()
{
	
	if (NULL != alignmentProc_){
		qDebug() << alignmentProc_->state();
		if (alignmentProc_->state() != QProcess::NotRunning)
			alignmentProc_->close();
		delete alignmentProc_;
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
	project_->exportFASTA(fin,true);
	
	project_->alignmentTool()->makeCommand(fin,fout,exec,args);
	qDebug() <<  trace.header() << exec << args;
	alignmentProc_->start(exec,args);
	nAlignments++;
	// Update menu items
	undoLastAction->setEnabled(true);
}

void SeqEditMainWin::alignmentStarted()
{
	qDebug() << trace.header() << "seqEditMainWin::alignmentStarted()";
	statusBar()->showMessage("Alignment running");
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
	qDebug() << trace.header() << "SeqEditMainWin::alignmentFinished()";
	
	QFile f(alignmentFileOut_->fileName());
	if (alignmentProc_->state() == QProcess::NotRunning && f.exists()){
		statusBar()->showMessage("Alignment finished");
		// delete alignmentFileIn_; // FIXME reinstate
		// alignmentFileIn_=NULL;
		readNewAlignment();
	}
	
}

void SeqEditMainWin::alignmentUndo()
{
	qDebug() << trace.header() << "SeqEditMainWin::alignmentUndo()";
	// Check whether there are still any alignments left to undo
	// and update menu items
	if (nAlignments > 0){
		project_->undoLastAlignment();
		nAlignments--;
		undoLastAction->setEnabled(nAlignments==0);
	}
}

void SeqEditMainWin::helpHelp(){
	app->showHelp("");
}

void SeqEditMainWin::helpAbout(){
	app->showAboutDialog(this);
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
	
//
// SeqEditMainWin - private members
// 

void SeqEditMainWin::init()
{
	nAlignments=0;
	lastImportedFile="";
	alignmentProc_=NULL;
	alignmentFileOut_=alignmentFileIn_=NULL;
}
			
void SeqEditMainWin::createActions()
{
	// File actions
	
	newProjectAction = new QAction( tr("&New project"), this);
	newProjectAction->setStatusTip(tr("Open a new project"));
	addAction(newProjectAction);
	connect(newProjectAction, SIGNAL(triggered()), this, SLOT(fileNewProject()));
	
	openProjectAction = new QAction( tr("&Open project"), this);
	openProjectAction->setStatusTip(tr("Open an existing project"));
	addAction(openProjectAction);
	connect(openProjectAction, SIGNAL(triggered()), this, SLOT(fileOpenProject()));
	openProjectAction->setEnabled(true);
	
	saveProjectAction = new QAction( tr("&Save project"), this);
	saveProjectAction->setStatusTip(tr("Save the project"));
	addAction(saveProjectAction);
	connect(saveProjectAction, SIGNAL(triggered()), this, SLOT(fileSaveProject()));
	
	saveProjectAsAction = new QAction( tr("Save project as"), this);
	saveProjectAsAction->setStatusTip(tr("Save the project under a new name"));
	addAction(saveProjectAsAction);
	connect(saveProjectAsAction, SIGNAL(triggered()), this, SLOT(fileSaveProjectAs()));
	
	importAction = new QAction( tr("&Import sequences"), this);
	importAction->setStatusTip(tr("Import a sequence file"));
	addAction(importAction);
	connect(importAction, SIGNAL(triggered()), this, SLOT(fileImport()));
	
	exportFASTAAction = new QAction( tr("&Export as FASTA"), this);
	exportFASTAAction->setStatusTip(tr("Export all project sequences in FASTA format"));
	addAction(exportFASTAAction);
	connect(exportFASTAAction, SIGNAL(triggered()), this, SLOT(fileExportFASTA()));
	
	exportClustalWAction = new QAction( tr("&Export as ClustalW"), this);
	exportClustalWAction->setStatusTip(tr("Export all project sequences in ClustalW format"));
	addAction(exportClustalWAction);
	connect(exportClustalWAction, SIGNAL(triggered()), this, SLOT(fileExportClustalW()));
	
	printAction = new QAction( tr("&Print"), this);
	printAction->setStatusTip(tr("Print current "));
	addAction(printAction);
	connect(printAction, SIGNAL(triggered()), this, SLOT(filePrint()));
	
	closeAction = new QAction( tr("Close"), this);
	closeAction->setStatusTip(tr("Close the project "));
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
	connect(undoAction, SIGNAL(triggered()), this, SLOT(editUndo()));
	undoAction->setEnabled(false);
	
	redoAction = new QAction( tr("&Redo"), this);
	redoAction->setStatusTip(tr("Redo"));
	redoAction->setShortcut(QKeySequence::Redo);
	addAction(redoAction);	
	connect(redoAction, SIGNAL(triggered()), this, SLOT(editRedo()));
	redoAction->setEnabled(false);
	
	undoLastAction = new QAction( tr("Undo &Last"), this);
	undoLastAction->setStatusTip(tr("Undo last alignment"));
	addAction(undoLastAction);
	connect(undoLastAction, SIGNAL(triggered()), this, SLOT(alignmentUndo()));
	undoLastAction->setEnabled(false);
	
	cutAction = new QAction( tr("Cu&t"), this);
	cutAction->setStatusTip(tr("Cut"));
	addAction(cutAction);
	connect(cutAction, SIGNAL(triggered()), this, SLOT(editCut()));
	cutAction->setEnabled(false);
	
	groupSequencesAction = new QAction( tr("Group sequences"), this);
	groupSequencesAction->setStatusTip(tr("Group the selected sequences"));
	addAction(groupSequencesAction);
	connect(groupSequencesAction, SIGNAL(triggered()), this, SLOT(editGroupSequences()));
	groupSequencesAction->setEnabled(false);
	
	ungroupSequencesAction = new QAction( tr("Ungroup sequences"), this);
	ungroupSequencesAction->setStatusTip(tr("Ungroup the selected sequences"));
	addAction(ungroupSequencesAction);
	connect(ungroupSequencesAction, SIGNAL(triggered()), this, SLOT(editUngroupSequences()));
	ungroupSequencesAction->setEnabled(false);
	
	lockAction = new QAction( tr("Lock groups(s)"), this);
	lockAction->setStatusTip(tr("Lock selected group(s)"));
	//lockAction->setIcon(QIcon(lock_xpm));
	addAction(lockAction);
	connect(lockAction, SIGNAL(triggered()), this, SLOT(editLock()));
	
	unlockAction = new QAction( tr("Unlock groups(s)"), this);
	unlockAction->setStatusTip(tr("Unlock selected group(s)"));
	addAction(unlockAction);
	connect(unlockAction, SIGNAL(triggered()), this, SLOT(editUnlock()));
	
	excludeAction = new QAction( tr("Exclude residues"), this);
	excludeAction->setStatusTip(tr("Exclude residues"));
	addAction(excludeAction);
	connect(excludeAction, SIGNAL(triggered()), this, SLOT(editExclude()));
	
	removeExcludeAction = new QAction( tr("Remove exclusion"), this);
	removeExcludeAction->setStatusTip(tr("Remove exclusion"));
	addAction(removeExcludeAction);
	connect(removeExcludeAction, SIGNAL(triggered()), this, SLOT(editRemoveExclude()));
	
	// Alignment actions
	goAction = new QAction( tr("&Go"), this);
	goAction->setStatusTip(tr("Run alignment"));
	addAction(goAction);
	connect(goAction, SIGNAL(triggered()), this, SLOT(alignmentGo()));
	goAction->setEnabled(false);
	
	// Help actions
	helpAction = new QAction( tr("&Help"), this);
	helpAction->setStatusTip(tr("Help"));
	addAction(helpAction);
	connect(helpAction, SIGNAL(triggered()), this, SLOT(helpHelp()));
	
	aboutAction = new QAction( tr("About"), this);
	aboutAction->setStatusTip(tr("About"));
	addAction(aboutAction);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(helpAbout()));
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
	connect(editMenu,SIGNAL(aboutToShow()),this,SLOT(setupEditMenu()));
	
	editMenu->addAction(undoAction);
	editMenu->addAction(redoAction);
	editMenu->addSeparator();
	
	editMenu->addAction(cutAction);
	editMenu->addSeparator();
	
	editMenu->addAction(groupSequencesAction);
	editMenu->addAction(ungroupSequencesAction);
	editMenu->addAction(lockAction);
	editMenu->addAction(unlockAction);
	editMenu->addSeparator();

	editMenu->addAction(excludeAction);
	editMenu->addAction(removeExcludeAction);
	
	alignmentMenu = menuBar()->addMenu(tr("Alignment"));
	connect(alignmentMenu,SIGNAL(aboutToShow()),this,SLOT(setupAlignmentMenu()));
	alignmentMenu->addAction(goAction);
	alignmentMenu->addAction(undoLastAction);
	
	menuBar()->insertSeparator();
	
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(helpAction);
	helpMenu->addAction(aboutAction);
}


void SeqEditMainWin::createToolBars()
{
	seqEditTB =addToolBar("Sequence editor tools");
	//seqEditTB->addAction(lockAction);
	//tb->setToggleButton(true);
	//connect(tb,SIGNAL(toggled(bool)),se,SLOT(lockMode(bool)));
	
	
}

void SeqEditMainWin::createStatusBar()
{
	QLabel *info = new QLabel(this);
	info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
 	info->setLineWidth(1);
	statusBar()->addPermanentWidget(info);
	connect(se,SIGNAL(info(const QString &)),info,SLOT(setText(const QString &)));
}
	

void SeqEditMainWin::readNewAlignment()
{
	FASTAFile fin(alignmentFileOut_->fileName());
	QStringList newlabels,newseqs,newcomments;
	fin.read(newlabels,newseqs,newcomments);
	
	qDebug() << trace.header() << newseqs.size() << " " << project_->sequences.size();

	// Groupings are preserved
	
	// Preserve tree-order
	for (int snew=0;snew<newseqs.size();snew++){
		int sold=0;
		while ((sold < project_->sequences.size())){
			if (newlabels.at(snew) == (project_->sequences.at(sold)->label.trimmed())) // remember, labels are padded at the end with spaces
				break;
			sold++;
		}
		if (sold < project_->sequences.size()){
			project_->moveSequence(sold,snew);
		}
		else{
			qDebug() << trace.header() << "SeqEditMainWin::readNewAlignment() missed " << newlabels.at(snew); 
		}
	}
	
	// Update each sequence
	int snew =0;
	int sold =0;
	while (sold < project_->sequences.size() && snew<newseqs.size()){
		project_->sequences.at(sold)->residues = newseqs.at(snew);
		snew++;
		sold++;
	}
	
	se->updateViewport();
}


void SeqEditMainWin::printRes( QPainter* p,QChar r,int x,int y)
{

	QColor txtColor;
	QChar rNoFlags;
	QPen oldPen;
	
	oldPen = p->pen();
	rNoFlags = r.unicode() & REMOVE_FLAGS;
	
	switch (rNoFlags.toAscii()){
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
	p->drawText( x, y, r,-1);
	
	if (r.unicode() & EXCLUDE_CELL){
		txtColor.setRgb(255,0,0);
		p->setPen(txtColor);
		//p->drawLine(0,0,w-2,h-2); // X marks the spot ...
		//p->drawLine(0,2,2,h-2);
	}
	
	p->setPen(oldPen);
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
 
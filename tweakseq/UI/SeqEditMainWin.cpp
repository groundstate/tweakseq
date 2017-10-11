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

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMenuBar>
#include <QPixmap>
#include <QToolBar>
#include <QToolButton>
#include <QPrinter>
#include <QDateTime>
#include <QStatusBar>
#include <QMessageBox>
#include <QSplitter>
#include <QPainter>

#include "Application.h"
#include "ClustalFile.h"
#include "FASTAFile.h"
#include "MessageWin.h"
#include "Project.h"
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
	
	setWindowTitle("tweakseq - " + project->name());
	setGeometry(0,0,640,400);
	setWindowIcon(QIcon(seqedit_xpm));
	
	project_=project;
	
	init();
	
	createActions();
	createMenus();
	//createToolBars();
	createStatusBar();

	
	// Hmm what to do about number of columns
	// TO DO This needs to be resizeable
	// TO DO ought to check that creation of widget does not fail because
	// of lack of memory
	
	QSplitter *split = new QSplitter(Qt::Vertical,this,"sew_split");
	
	se = new SeqEdit(project_,split);
	
	mw = new MessageWin(split);
	
	setCentralWidget(split);
		

	printer = new QPrinter();
	printer->setFullPage(TRUE);
	
	statusBar()->message("Ready");
	
}

SeqEditMainWin::~SeqEditMainWin(){
	delete printer;
}

void SeqEditMainWin::doAlignment(){
}

//
// SeqEditMainWin - private slots
// 

// File menu slots

void SeqEditMainWin::fileNewProject()
{
}

void SeqEditMainWin::fileSaveProject()
{
	project_->save();
	setWindowTitle("tweakseq - " + project_->name());
}

void SeqEditMainWin::fileSaveProjectAs()
{
	setWindowTitle("tweakseq - " + project_->name());
}
	
void SeqEditMainWin::fileImport(){
	
	//QString fname = QFileDialog::getOpenFileName(this,
  //   tr("Open Sequence"), "./", tr("Sequence Files (*.fasta *.aln *.clustal)"));
	//if (fname.isNull()) return;
	
	//ClustalFile cf("/home/michael/src/da/src/alignmentin.aln");
	// ClustalFile cf("/home/michael/src/seqme/seqme/test/Bcl2l15_12_10.clustal");
	QString fname = "/home/michael/src/seqme/seqme/test/Bcl2l15_12_10.fasta";
	
	FASTAFile cf(fname);
	QStringList seqnames,seqs,comments;
	if (cf.read(seqnames,seqs,comments)){
		for (int i=0;i<seqnames.size();i++){
			project_->addSequence(seqnames.at(i),seqs.at(i),comments.at(i));
		}
		lastImportedFile=fname;
	}
	else{
	}
}

void SeqEditMainWin::fileExportFASTA(){
	
	FASTAFile cf("/home/michael/src/seqme/seqme/test/out.fasta");
	//cf.write();
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
	se->cutSelection();
}

void SeqEditMainWin::editGroupSequences()
{
	statusBar()->clearMessage();
	// Groups the current selection of sequences
	if (!project_->groupSelectedSequences())
		statusBar()->showMessage("Grouping unsuccessful");
}

void SeqEditMainWin::editUngroupSequences()
{
}

void SeqEditMainWin::editExclude(){
	se->excludeSelection();
}

void SeqEditMainWin::editRemoveExclude(){
	se->removeExcludeSelection();
}
	
void SeqEditMainWin::editLock(){
	se->lockSelection();
}

void SeqEditMainWin::editUnlock(){
	se->unlockSelection();
}

void SeqEditMainWin::setupAlignmentMenu()
{
	goAction->setEnabled(project_->numSequences() >= 2);
}

void SeqEditMainWin::alignmentGo()
{

	qDebug() << trace.header() << "SeqEditMainWin::alignmentGo()";
	QString l;
	FILE *fd;
	char buf[120];
	int cr;
	
	// TO DO this needs to be generalized with respect to the choice of
	// alignment program
	// FIXME scatrch space ...
	
	writeAlignment(FASTA,"alignmentin.fasta");
	
	// TO DO run clustalw in a separate thread ?
	// FIXME use QProcess
	
	l=seqAlignmentCommand.copy();
	l.append("  ");
	// l.append(seqAlignmentFlags);
	// Output of the alignment program is redirected to a file 
	l.append(" alignmentin.fasta");
	
	// Open a pipe so that we can read back the output as it is produced
	// Seg faults if I use a text stream - spose this can be fixed later ...
	fd=popen((const char *) l,"r");
	
	while (!feof(fd)){	
		if (NULL == fgets(buf,120,fd))
			continue;
		// Strip carriage returns
		cr = strlen(buf);
		buf[cr-1]='\0';
		mw->addMessage(buf);
		qApp->processEvents();
	}
	
	pclose(fd);
	
	//QFile f("alignment.out");
	//f.open(IO_ReadOnly);
	//QTextStream ts(&f);
	//while (!ts.eof()){
	//	l=ts.readLine();
	//	mw->addMessage(l);
	//}
	//f.close();
	
	readAlignment(FASTA,"alignmentin.aln");
	
	nAlignments++;
	
	// Update menu items
	undoLastAction->setEnabled(true);
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
}

void SeqEditMainWin::helpAbout(){
	app->showAboutDialog(this);
}

void SeqEditMainWin::sequenceSelectionChanged()
{
	cutAction->setEnabled(!(project_->sequenceSelection->empty()));
	groupSequencesAction->setEnabled(!(project_->sequenceSelection->empty()));
}

void SeqEditMainWin::residueSelectionChanged()
{
}
	
//
// SeqEditMainWin - private members
// 

void SeqEditMainWin::init()
{
	nAlignments=0;
	seqAlignmentCommand=app->ClustalWPath();
	lastImportedFile="";
}
			
void SeqEditMainWin::createActions()
{
	// File actions
	
	newProjectAction = new QAction( tr("&New project"), this);
	newProjectAction->setStatusTip(tr("Open a new project"));
	addAction(newProjectAction);
	connect(newProjectAction, SIGNAL(triggered()), this, SLOT(fileNewProject()));
	newProjectAction->setEnabled(false); // FIXME disabled for now
	
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
	
	printAction = new QAction( tr("&Print"), this);
	printAction->setStatusTip(tr("Print current "));
	addAction(printAction);
	connect(printAction, SIGNAL(triggered()), this, SLOT(filePrint()));
	
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
	
	excludeAction = new QAction( tr("Exclude residues"), this);
	excludeAction->setStatusTip(tr("Exclude residues"));
	addAction(excludeAction);
	connect(excludeAction, SIGNAL(triggered()), this, SLOT(editExclude()));
	
	removeExcludeAction = new QAction( tr("Remove exclusion"), this);
	removeExcludeAction->setStatusTip(tr("Remove exclusion"));
	addAction(removeExcludeAction);
	connect(removeExcludeAction, SIGNAL(triggered()), this, SLOT(editRemoveExclude()));
	
	lockAction = new QAction( tr("Lock residues"), this);
	lockAction->setStatusTip(tr("Lock residues"));
	lockAction->setIcon(QIcon(lock_xpm));
	addAction(lockAction);
	connect(lockAction, SIGNAL(triggered()), this, SLOT(editLock()));
	
	unlockAction = new QAction( tr("Unlock residues"), this);
	unlockAction->setStatusTip(tr("Unlock residues"));
	addAction(unlockAction);
	connect(unlockAction, SIGNAL(triggered()), this, SLOT(editUnlock()));
	
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
	fileMenu->addAction(saveProjectAction);
	fileMenu->addAction(saveProjectAsAction);
	fileMenu->addSeparator();
	fileMenu->addAction(importAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exportFASTAAction);
	fileMenu->addSeparator();
	fileMenu->addAction(printAction);
	fileMenu->addSeparator();
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
	editMenu->addSeparator();
	
	editMenu->addAction(excludeAction);
	editMenu->addAction(removeExcludeAction);
	editMenu->addSeparator();
	
	editMenu->addAction(lockAction);
	editMenu->addAction(unlockAction);
	
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
	seqEditTB->addAction(lockAction);
	//tb->setToggleButton(true);
	//connect(tb,SIGNAL(toggled(bool)),se,SLOT(lockMode(bool)));
}

void SeqEditMainWin::createStatusBar()
{
}
	
void SeqEditMainWin::writeAlignment(int fileFormat,QString fname)
{
	qDebug() << trace.header() << "SeqEditMainWin::writeAlignment() " << fname;
  int i,j;
	QString s;
	
	QFile f(fname);
	f.open(IO_WriteOnly);
	QTextStream ts (&f);
	
	switch (fileFormat){
		case FASTA:
		  for (j=0;j<project_->numSequences();j++){
				// Write identifier
				ts << ">" << project_->getLabelAt(j) << "\n";
				// Write sequence in 50 residue chunks
				s=project_->getSequence(j,REMOVE_FLAGS);
				for (i=0;i<s.length();i+=50)
					ts << s.mid(i, 50 )<< "\n"; // QT truncates the last substring
		  }
			break;
	} // end of switch(fileFormat)
	
	f.close();
}

void SeqEditMainWin::readAlignment(int fileFormat,QString fname)
{
	
	
	
	// The displayed sequence is updated according to the new alignment.
	
	// First the displayed sequences are reordered according to the order of the
	// aligned sequences in the .aln file since we want to preserve this order
	
// 	for (j=0;j<se->numSequences();j++)
// 	{
// 		// Find the displayed sequence whose id matches the current aligned
// 		// sequence
// 		i=0;
// 		while((ids[j] != se->getLabel(i)) && i<se->numSequences())
// 			i++;
// 		// Now shift the sequence
// 		se->moveSequence(i,j);
// 	}
	
	// Then we pass through the original sequences making the insertions given by
	// the alignment. 
	// This is simplified by removing all insertions from the displayed sequence
	// prior to the matching process. 
	
	// The aligned sequence is searched for the an entry that is
	// not an insertion. The original sequence is then searched for the
	// corresponding entry exclusions. The number of insertions prior to
	// the entry is counted for each sequence. If there
	// are any new insertions prior to the entry in the aligned sequence,  these
	// new insertions are transferred to the displayed sequence.
	// The next entry in the aligned sequence is then found and the whole
	// process repeated and so on until the end of the aligned sequence.
	
	// Prints out the number of insertions made in each sequence as a check.
	
	// TO DO post alignment, lock marks will be invalid so these must be removed
	
	// All undos will be invalid
	
// 	QString newseq,oldseq;
// 	
// 	for (k=0;k<se->numSequences();k++)
// 	{
// 		newseq = alignment[k];
// 		oldseq = se->getSequence(k,KEEP_FLAGS);
// 		
// 		// Remove all insertions and exclusions from the old sequence
// 		i=0;
// 		while (i<oldseq.length())
// 		{
// 			if (oldseq[i] == '-' || (oldseq[i].unicode() & EXCLUDE_CELL))
// 				oldseq.remove(i,1); // note - do not increment j !
// 			else
// 				i++;
// 		}
// 		i=j=0; // index to current position in the old and new sequence
// 		
// 		while (j<newseq.length())
// 		{
// 			if (newseq[j]=='-') // find insertions and add them to the old sequence
// 			{
// 				oldseq.insert(i,'-');
// 				i++;
// 				j++;
// 			}
// 			else
// 			{
// 				// Found the next residue in the aligned sequence
// 				// so now we have to find the corresponding residue in the
// 				// original sequence.
// 				// Mainly, we have to skip the exclusions marked in the original sequence
// 				while (i<oldseq.length() && ((oldseq[i].unicode() & REMOVE_FLAGS) != newseq[j].unicode()
// 					|| (oldseq[i].unicode() & EXCLUDE_CELL)) )
// 				{ 
// 					i++;
// 				}
// 				i++; // i points at a match
// 				j++;	
// 				
// 			}
// 		}
// 		seq.append(new Sequence(ids[k],oldseq));
// 	}
// 
// 	// TO DO - bother to check for a change 
// 	se->newAlignment(seq);
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

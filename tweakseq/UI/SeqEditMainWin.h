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

#ifndef __SEQEDIT_MAINWIN_H_
#define __SEQEDIT_MAINWIN_H_

#include <QMainWindow>
#include <QProcess>

class QDomDocument;
class QDomElement;
class QPrinter;
class QSplitter;
class QTemporaryFile;
class QToolBar;

class MessageWin;
class Project;
class SeqEdit;
class SequenceGroup;

class SeqEditMainWin: public QMainWindow
{
Q_OBJECT

public:

	SeqEditMainWin(Project *);
	~SeqEditMainWin();
	
	void doAlignment();
	
	SeqEdit *se;
	
	void postLoadTidy();
	void writeSettings(QDomDocument &,QDomElement &);
	void readSettings(QDomDocument &);
	
public slots:
	
	void sequenceSelectionChanged();
	void residueSelectionChanged();
	
protected:
	
	virtual void closeEvent(QCloseEvent *);
	
private slots:

	void filePrint();
	void fileNewProject();
	void fileOpenProject();
	void fileSaveProject();
	void fileSaveProjectAs();
	void fileImport();
	void fileExportFASTA();
	void fileExportClustalW();
	void fileClose();
	
	void setupEditActions();
	void editUndo();
	void editRedo();
	void editCut();
	
	void editGroupSequences();
	void editUngroupSequences();
	
	void editExclude();
	void editRemoveExclude();
	void editLock();
	void editUnlock();
	
	void editReadOnly();
	
	void setupAlignmentMenu();
	void alignmentAll();
	void alignmentSelection();
	
	void alignmentStarted();
	void alignmentReadyReadStdOut();
	void alignmentReadyReadStdErr();
	void alignmentFinished(int,QProcess::ExitStatus);
	
	void settingsEditorFont();
	
	void helpHelp();
	void helpAbout();
	
	void alignmentPreviewClosed(int);
	
	void test();
	
	void createContextMenu(const QPoint &);
	
private:

	void init();
	
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	
	void startAlignment();
	void readNewAlignment(bool);
	void previewNewAlignment();
	
	void printRes( QPainter*,QChar,int,int );
	
	bool maybeSave();
	
	QMenu    *fileMenu,*alignmentMenu,*editMenu,*settingsMenu,*helpMenu,*testMenu;
	QAction  *newProjectAction,*openProjectAction,*saveProjectAction,*saveProjectAsAction;
	QAction  *importAction, *exportFASTAAction,*exportClustalWAction,*printAction, *closeAction, *quitAction;
	QAction  *alignAllAction,*alignSelectionAction,*undoLastAction;
	QAction  *undoAction,*redoAction,*cutAction,*excludeAction,*removeExcludeAction,*lockAction,*unlockAction;
	QAction  *groupSequencesAction,*ungroupSequencesAction;
	QAction  *readOnlyAction;
	QAction  *helpAction,*aboutAction;
	QAction *settingsEditorFontAction;
	QAction *testAction;
	
	QToolBar *seqEditTB;
	MessageWin *mw;
	QSplitter *split;
	
	QPrinter *printer;	
	
	QProcess *alignmentProc_;
	QTemporaryFile *alignmentFileIn_,*alignmentFileOut_;
	bool alignAll;
	
	QString lastImportedFile;
	
	Project *project_;
	
};

#endif

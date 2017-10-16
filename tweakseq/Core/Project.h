//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017  Michael J. Wouters, Merridee A. Wouters
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


#ifndef __PROJECT_H_
#define __PROJECT_H_

#include <QColor>
#include <QDir>
#include <QDomDocument>
#include <QList>
#include <QObject>
#include <QStack>
#include <QString>

#define KEEP_FLAGS 0XFFFF // TO DO change all this to allow higher order bits
#define REMOVE_FLAGS	0X007F	

#define EXCLUDE_CELL 0x0080

enum alignmentFormats {FASTA,CLUSTALW};

class Operation;
class Sequence;
class SequenceSelection;
class SequenceGroup;
class SeqEditMainWin;

class Project:public QObject
{
	Q_OBJECT
	
	public:
		
		Project();
		~Project();
	
		void newProject();
		void openProject();
		bool save();
		void load(QString &);
		
		QString name(){return name_;}
		void setName(QString &);
		
		bool empty();
		bool isModified(){return dirty_;}
		
		QList<Sequence *>  sequences;
		SequenceSelection *sequenceSelection;
		
		int numSequences(){return sequences.size();}
		
		QString getSequence(int,int);
		QString getSequence(QString);
		QString getLabelAt(int);
	
		void clearSequences();
		Sequence * addSequence(QString ,QString,QString,QString );
		int  deleteSequence(QString);
		void insertSequence(QString,QString,int);
		int  replaceSequence(QString,QString,QString);
		void moveSequence(int,int);
		void changeResidues(QString ,int pos);
		void newAlignment(QList <Sequence *>);
		void setAlignment(QList <Sequence *> );
		
		bool groupSelectedSequences(QColor);
		bool ungroupSelectedSequences();
		
		void addGroupToSelection(SequenceGroup *);
		void lockSelectedGroups(bool);
	
		void logOperation(Operation *);
		void undo();
		void redo();
		void undoLastAlignment();
		
		void exportFASTA(QString);
		void exportClustalW(QString);
		
	signals:
		
		void sequencesChanged(int,int);
		void sequenceAdded(Sequence *);
		void sequencesRemoved(int,int);
		
	public slots:
	
		
		void closeIt();
		
		void createMainWindow();
	
		void mainWindowClosed();
		
	private:
		
		void init();
		int  getSeqIndex(QString);
		
		// Widgets we keep track of
		SeqEditMainWin *mainWindow_;
		
		bool empty_;
		bool named_;
		bool dirty_;
		QString name_;
		QDir path_;
		
		int nAlignments;
		
		QStack<Operation *> undoStack;
		
		QList<SequenceGroup *> groups_;
	
};

#endif
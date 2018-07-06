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
#include <QUndoStack>

#include "AlignmentTool.h"
#include "Consensus.h"
#include "Sequences.h"

#define KEEP_FLAGS 0XFFFF // TO DO change all this to allow higher order bits
#define REMOVE_FLAGS	0X007F	

#define EXCLUDE_CELL 0x0080


class QDomDocumentFragment;

enum alignmentFormats {FASTA,CLUSTALW};


class AlignmentTool;
class Operation;
class ResidueSelection;
class Sequence;
class SequenceGroup;
class SequenceSelection;
class SeqEditMainWin;

class SearchResult
{
	public:
		SearchResult(Sequence *sequence,int start,int stop):sequence(sequence),start(start),stop(stop){}
		Sequence *sequence;
		int start,stop;
};

class Project:public QObject
{
	Q_OBJECT
	
	public:
		
		Project();
		~Project();
		
		void setMainWindow(SeqEditMainWin *);
		
		bool save(QString &);
		void load(QString &);
		void writeSettings(QDomDocument &,QDomElement &);
		void readSettings(QDomDocument &);
		
		QString name(){return name_;}
		void setName(QString &);
		bool named();
		
		bool empty();
		bool isModified(){return dirty_;}
		
		bool aligned(){return aligned_;}
		void setAligned(bool);
		
		int sequenceDataType(){return sequenceDataType_;}
		void setSequenceDataType(int seqData){sequenceDataType_=seqData;}
		Sequences  sequences;
		ResidueSelection *residueSelection;
		SequenceSelection *sequenceSelection;
		QList<SequenceGroup *> sequenceGroups;

		bool importSequences(QStringList &,QString &);
		
		QString getResidues(int,int);
		QString getLabelAt(int);

		void setAlignment(const QList<Sequence *> &,const QList<SequenceGroup *> &);
		
		void undo();
		void redo();
		void excludeSelectedResidues(bool);
		bool cutSelectedResidues();
		bool cutSelectedSequences();
		void pasteClipboard(Sequence *);
		
		bool canGroupSelectedSequences();
		bool groupSelectedSequences(QColor);
		void ungroupSelectedSequences();
		void ungroupAllSequences();
		void addGroupToSelection(SequenceGroup *);
		bool canToggleLock();
		void lockSelectedGroups(bool);
		void hideNonSelectedGroupMembers();
		void unhideAllGroupMembers();
		
		void addInsertions(QList<Sequence*> &,int,int,bool);
		
		QUndoStack &undoStack(){return undoStack_;} // main window needs access to this to validate actions
		
		AlignmentTool*  alignmentTool(){return alignmentTool_;}
		void setAlignmentTool(const QString &);
		
		void exportFASTA(QString,bool);
		void exportSelectionFASTA(QString,bool);
		void exportClustalW(QString,bool);
		
		void readNewAlignment(QString,bool);
	
		int search(const QString &);
		QList<SearchResult *> & searchResults(){return searchResults_;}
		
		Consensus consensusSequence;
		
	signals:
		
		void uiUpdatesEnabled(bool);
		
	public slots:
		
		void enableUIupdates(bool);
		
		void closeIt();

		void createMainWindow();
	
		void mainWindowClosed();
		
	private slots:
		
		void sequencesChanged();
		
	private:
		
		void init();
		void readAlignmentToolSettings(QDomDocument &);
		
		int  getSeqIndex(QString);
		int  getGroupIndex(SequenceGroup *sg);
		
		QStringList findDuplicates(QStringList &);
		
		// Widgets we keep track of
		SeqEditMainWin *mainWindow_;
		
		bool aligned_;
		bool empty_;
		bool dirty_;
		bool named_;
		QString name_;
		QDir path_;
		
		int sequenceDataType_;
		int nAlignments;
		AlignmentTool *alignmentTool_,*clustalOTool_,*muscleTool_;
		QUndoStack undoStack_;
	
		QList<SearchResult *> searchResults_;
		
		QDomDocumentFragment *clustaloSettings_,*muscleSettings_;
		
};

#endif
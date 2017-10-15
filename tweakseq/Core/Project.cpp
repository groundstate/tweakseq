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

#include <QtDebug>
#include "DebuggingInfo.h"

#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>

#include "Application.h"
#include "ClustalFile.h"
#include "FASTAFile.h"
#include "Operation.h"
#include "Project.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SequenceSelection.h"
#include "SeqEditMainWin.h"
#include "XMLHelper.h"

extern Application *app;

//
// Public members
//	

Project::Project()
{
	init();
}


		
Project::~Project()
{
	delete sequenceSelection;
	// FIXME and the rest ..
}

void Project::setName(QString &n)
{
	name_=n;
}

QString Project::getSequence(int i,int maskFlags)
{
	
	// First sequence has id 0
	// REMOVE_FLAGS in this context means return only those
	// residues which are not to be excluded from the alignment
	
	// It is intended that this function be mainly used to construct
	// a string suitable for use by an external alignment program
	
	QString r;
	int j,k=0;
	
	qDebug() << trace.header() << "Project::getSequence()" << i << " " << maskFlags;
	// Return NULL if the index is out of range
	if ( i > sequences.count()-1)
		return NULL;
	else{
		r=sequences.at(i)->residues;
		switch (maskFlags)
		{
			case KEEP_FLAGS:
				break;
			case REMOVE_FLAGS:
			  // Excluded residues are removed from the returned sequence
				for (j=0;j<r.length();j++)
				  if (!(r[j].unicode() & EXCLUDE_CELL))
					{
						r[k]=r[j];
						k++;
					}
				// Chop off any remaining residues
				// 09052007 BUG FIXED - was removing the last residue
				r.truncate(k);
				//if (k!=0)
				//	r.truncate(k-1);
				//else
				//	r.truncate(0);
					
				break;
		}
		return r;
	}
}


QString Project::getSequence(QString l)
{
	// Returns the (masked) sequence with label l
	// Mainly for use by other programs
	// Returns NULL if nothing matching is found
	qDebug() << trace.header() << "Project::getSequence() " << l;
	
	int i;
	if ((i=getSeqIndex(l)) == -1)
		return NULL;
	else
		return sequences.at(i)->residues;
}

QString Project::getLabelAt(int i)
{
	qDebug() << trace.header() << "Project::getLabel() " << i;
	// Return NULL if the index is out of range
	if (i<0 || i > sequences.size()-1)
		return NULL;
	else
		// strip white space from the end of the string
		return (sequences.at(i)->label).stripWhiteSpace(); 
}

void Project::clearSequences()
{
	// TO DO - more stuff ? Difference between clearing and a new project ?

	qDebug() << trace.header() << "Project::clearSequences()";
	sequences.clear();
	emit sequencesChanged(0,sequences.size());
	dirty_=true;
}

Sequence * Project::addSequence( QString l,QString s,QString c,QString f)
{
	qDebug() << trace.header() << "Project::addSequence()\n" << l << " " << s;
	empty_=false;
	Sequence * newSeq = new Sequence(l,s,c,f);
	sequences.append(newSeq);
	emit sequenceAdded(newSeq);
	dirty_=true;
	return newSeq;
}


int Project::deleteSequence(QString l){
	// TODO
	// Deletes the sequence with identifier id
	// Returns position of the deleted sequence, -1 if id was not found
	qWarning() << warning.header() << "Project::deleteSequence() NOT IMPLEMENTED!";
	int i;
	if ((i=getSeqIndex(l))>=0){ // found it
	}
	dirty_=true;
	return i;
}

void Project::insertSequence(QString ,QString ,int )
{
	qWarning() << warning.header() << "Project::insertSequence() NOT IMPLEMENTED!";
	dirty_=true;
}

int Project::replaceSequence(QString l,QString newLabel,QString newRes){
	// Replace sequencewith identifier id
	// Returns position of the replace sequence, -1 if id was not found
	qDebug() << trace.header() << "Project::replaceSequence()";
	int i;
	if ((i=getSeqIndex(l))>=0){ // found it
		deleteSequence(l);
		insertSequence(newLabel,newRes,i);
	}
	
	dirty_=true;
	return  i;
	
}

void Project::moveSequence(int i,int j)
{
	// Moves the sequence at position i to position j
	qDebug() << trace.header() << "Project::moveSequence() from " << i << " " << j;
	//Sequence *pSeq=sequences.take(i);
	//sequences.insert(j,pSeq);
	if (i<0 || j <0 || i >= sequences.size() || j >= sequences.size()){
		return;
	}
	sequences.move(i,j);
	dirty_=true;
	emit sequencesChanged(sequences.size(),sequences.size());
}

void Project::changeResidues(QString r,int pos)
{
	qDebug() << trace.header() << "Project::changeResidues()  " << r << " " << pos;
	sequences.at(pos)->residues=r;
	dirty_=true;
	emit sequencesChanged(sequences.size(),sequences.size());
}


void Project::newAlignment(QList <Sequence *> s)
{
	// Called after making an alignment
	qDebug() << trace.header() << "Project::newAlignment()";
	empty_=false;
	undoStack.push(new Operation(Operation::Alignment,sequences));
	sequences.clear();
	for (int i=0;i<s.count();i++){
		sequences.append(new Sequence(s.at(i)->label,s.at(i)->residues));
		//int rowLength = LABELWIDTH+FLAGSWIDTH+ s.at(i)->residues.length();
		//if (numCols()<rowLength) setNumCols(rowLength);
	}	
	nAlignments++;
	dirty_=true;
	emit sequencesChanged(sequences.size(),0);
}

void Project::setAlignment(QList <Sequence *> s){

	sequences.clear();
	empty_=false;
	for (int i=0;i<s.count();i++)
		sequences.append(new Sequence(s.at(i)->label,s.at(i)->residues));
	dirty_=true;
	emit sequencesChanged(sequences.size(),0);
}

//
//
//

bool Project::groupSelectedSequences(QColor gcol){
	// Require two or more sequences in the selection
	if (sequenceSelection->size() < 2)
		return false;
	
	// If the selection wholly contains one or more existing groups, then
	// the selection is merged into a single group
	QList<SequenceGroup *> selgroups;
	for (int g=0;g<groups_.size();g++){
		SequenceGroup *sg = groups_.at(g);
		bool contained = true;
		int selcnt=0;
		for (int s=0;s<sg->size();s++){
			if (!sequenceSelection->contains(sg->itemAt(s))){
				contained=false;
				// don't break because we need to test the others for overlap
			}
			else
				selcnt++;
		}
		// If only part of a group is selected then this is bad
		if (selcnt > 0 && selcnt != sg->size()){
			qDebug() << trace.header() << "Project::groupSelectedSequences() failed";
			return false;
		}
		
		if (contained){
			selgroups.append(sg);
			qDebug() << trace.header() << "Project::groupSelectedSequences() group selected for merging";  
		}
	}
	
	for (int g=0;g<selgroups.size();g++){
		SequenceGroup *sg = selgroups.at(g);
		groups_.removeOne(sg);
		delete sg;
	}
	
	// All sorted, so create the group

	SequenceGroup *sg = new SequenceGroup();
	sg->setTextColour(gcol);
	groups_.append(sg);
	for ( int s=0;s<sequenceSelection->size();s++){
		Sequence *seq = sequenceSelection->itemAt(s);
		sg->addSequence(seq);
	}
	qDebug() << trace.header() << "Project::groupSelectedSequences() new group ";
	dirty_=true;
	return true;
}

bool Project::ungroupSelectedSequences()
{
	// Any grouped sequence that is in the selection is removed from its group
	// If this leaves only one sequence in the group, this is OK
	for ( int s=0;s<sequenceSelection->size();s++){
		Sequence *seq = sequenceSelection->itemAt(s);
		if (seq->group){
			for (int g=0;g<groups_.size();g++){
				groups_.at(g)->removeSequence(seq); // this also removes the parent group form the sequence
			}
		}
	}
	dirty_=true;
	return true;
}

void Project::lockSelectedGroups(bool lock){
	// The selection must contain all the sequences in one or more groups
	
	// First, check for an ungrouped sequence because this is fatal
	for (int s=0;s<sequenceSelection->size();s++){
		if (sequenceSelection->itemAt(s)->group == NULL){
			qDebug() << trace.header() << "Project::lockSelectedGroups() ungrouped sequence in the selcetion";
			return;
		}
	}
	QList<SequenceGroup *> selgroups;
	for (int g=0;g<groups_.size();g++){
		SequenceGroup *sg = groups_.at(g);
		bool contained=true;
		for (int s=0;s<sg->size();s++){
			if (!sequenceSelection->contains(sg->itemAt(s))){
				contained=false;
				break;
			}
		}
		if (contained){
			selgroups.append(sg);
			qDebug() << trace.header() << "Project::lockSelectedGroups() group in selection"; 
		}	
	}
	
	for (int sg=0;sg<selgroups.size();sg++){
		selgroups.at(sg)->lock(lock);
	}
}

void Project::addGroupToSelection(SequenceGroup *selg)
{
	qDebug() << trace.header() << "Project::addGroupToSelection ";
	for (int g=0;g<groups_.size();g++){
		SequenceGroup *sg = groups_.at(g);
		if (selg==sg){
			for (int s=0;s<sg->size();s++)
				sequenceSelection->toggle(sg->itemAt(s));
			break;
		}
	}
	dirty_=true;
}

//
//
//

void Project::logOperation(Operation *op)
{
	undoStack.push(op);
}

void Project::undo()
{
	// Undo last editing command
// 	qDebug() << trace.header() << "SeqEdit::undoEdit()";
// 	int startRow,stopRow,startCol,stopCol,row,firstRow;
// 	int col;
// 	Operation *op;
// 	
// 	if (!undoStack.isEmpty()){
// 	
// 		op = undoStack.top(); // FIXME ported but not tested
// 		startRow=op->startRow;
// 		stopRow =op->stopRow;
// 		startCol=op->startCol;
// 		stopCol=op->stopCol;
// 		
// 		switch (op->mode){
// 		
//    		case Operation::Insertion:
//      		for (row=startRow;row<=stopRow;row++){
//       		deleteCells(row,startCol,stopCol);
// 					checkLength();
//       		// Update past the deletion point only FIXME
//       		//for (col=startCol;col<numCols();col++)
//         	//	updateCell(row,col);
//     	 	}
//     		
//      		break;
// 				
//    		case Operation::Deletion:
//     		
//     		for (row=startRow;row<=stopRow;row++){
//      			insertCells((op->editText).at(row-startRow),
//          		row,startCol-1); // subtract 1 because we get a post-insertion
// 					checkLength();
//        		for (col=startCol;col< numCols();col++)
//        			updateCell(row,col);
//     		}
//     		
//      		break;
// 				
// 			case Operation::Mark:
//     		for (row=startRow;row<=stopRow;row++)
//       		for (col=startCol;col<=stopCol;col++){
//         		setCellMark(row,col,FALSE);
//         		updateCell(row,col);
//       		}
// 				break;
// 			case Operation::Move:
// 				moveSequence(stopRow,startRow);
// 				//firstRow=stopRow; ?? FIXME
// 				//if (startRow<stopRow) firstRow=startRow;
// 				//for (row=firstRow;row<seq.count();row++)
// 				//	for (col=0;col<seq.at(row)->residues.length()+LABELWIDTH+FLAGSWIDTH;col++)
// 				//		updateCell(row,col); 
//     		break;
// 			case Operation::Alignment:
// 				setAlignment(op->seq);
// 				nAlignments--; // changeAlignment increments so decrement by 2
// 				break;
// 		} // end of case
//  		Operation * op = undoStack.pop(); delete op; // FIXME ported but not tested
// 		emit sequencesChanged();
// 	} // end of if
// 	else{
// 		// Make a rude sound
// 		printf("\a");
// 	}
}

void Project::redo()
{
	// TO DO
}

void Project::undoLastAlignment()
{
	// This is a special undo operation because it junks everything 
	qDebug() << trace.header() << "SeqEdit::undoLastAlignment";
	Operation *op;
	
	if (nAlignments > 0){
		// Find the most recent alignment on the undo stack
		// removing all other edit records
		// TO DO the redo stack
		op=undoStack.top();
		while (op->mode != Operation::Alignment){ // FIXME ported but not checked yet
			undoStack.pop();
			delete op;
			op = undoStack.top();
		}
		int oldSize = sequences.size();
		setAlignment(op->seq);
		nAlignments--;
		emit sequencesChanged(sequences.size(),oldSize);
	}
	dirty_=true;
}

//
//	Public slots:
//


void Project::newProject()
{
	new Project();
}

void Project::openProject()
{
}

bool Project::save()
{
	QString tmp;
	
	if (!named_){ // never saved so need a get a project name and path
		QString fileName = QFileDialog::getSaveFileName(mainWindow_, tr("Save Project"));
		if (fileName.isNull()) return false;
		QFileInfo fi(fileName);
		path_=fi.path();
		name_=fi.fileName();
		named_=true;
	}
	
	QDomDocument saveDoc;
	QDomElement root = saveDoc.createElement("tweakseq");
	saveDoc.appendChild(root);
	
	QFileInfo fi(path_,name_);
	QFile f(fi.filePath());
	f.open(IO_WriteOnly);
	QTextStream ts(&f);
	
	QDomElement el = saveDoc.createElement("version");
	root.appendChild(el);
	QDomText te = saveDoc.createTextNode(app->version());
	el.appendChild(te);
		
	for (int s=0;s<sequences.size();s++){
		Sequence *seq = sequences.at(s);
		
		QDomElement se = saveDoc.createElement("sequence");
		root.appendChild(se);
		
		XMLHelper::addElement(saveDoc,se,"name",seq->label);
		XMLHelper::addElement(saveDoc,se,"comment",seq->comment);		
		XMLHelper::addElement(saveDoc,se,"residues",seq->noFlags());
		XMLHelper::addElement(saveDoc,se,"source",seq->source);
		XMLHelper::addElement(saveDoc,se,"exclusions","unimplemented");
		
	}
	
	for (int g=0;g<groups_.size();g++){
		SequenceGroup *sg = groups_.at(g);
		QDomElement gel = saveDoc.createElement("group");
		root.appendChild(gel);
		XMLHelper::addElement(saveDoc,gel,"locked",(sg->locked()?"yes":"no"));
		QColor col = sg->textColour();
		QString str =  QString::number(col.red()) + QString(",") + QString::number(col.green()) + QString(",")+ QString::number(col.blue());
		XMLHelper::addElement(saveDoc,gel,"colour",str);
		
		QString seqs = "";
		for (int s=0;s<sg->size();s++){
			Sequence *seq = sg->itemAt(s);
			seqs += seq->label;
			if (s < sg->size()-1) seqs += ",";
		}
		XMLHelper::addElement(saveDoc,gel,"sequences",seqs);
	}
	
	saveDoc.save(ts,2);
	f.close();
	
	dirty_=false;
	
	return true;
}

void Project::load(QString &fname)
{
	qDebug() << trace.header() << "Project::load()" << fname;
	named_=true; 
	dirty_=false;
	empty_=false;
	QDomDocument doc;
	
	QFile file(fname);
	if ( !file.open( IO_ReadOnly ) )
		return ;
	QString err; int errlineno,errcolno;
	if ( !doc.setContent( &file,true,&err,&errlineno,&errcolno ) )
	{	
		qDebug() << trace.header() << "Project::load() error at line " << errlineno;
		file.close();
		return ;
	}
	QList<long> gids;
	
	// Get all the sequences
	QDomNodeList nl = doc.elementsByTagName("sequence");
	for (int i=0;i<nl.count();i++){
		QDomNode sNode = nl.item(i);
		
		QString sName,sComment,sResidues,sSrc;
		
		QDomElement elem = sNode.firstChildElement();
		while (!elem.isNull()){
			if (elem.tagName() == "name")
				sName = elem.text().trimmed();
			else if (elem.tagName() == "comment")
				sComment=elem.text().trimmed();
			else if (elem.tagName() == "residues")
				sResidues = elem.text().trimmed();
			else if (elem.tagName() == "source")
				sSrc = elem.text().trimmed();
			else if (elem.tagName() == "exclusions"){
			}
			elem=elem.nextSiblingElement();
		}
		addSequence(sName,sResidues,sComment,sSrc);
	}	
	
	// Get all the groups
	nl = doc.elementsByTagName("group");
	for (int i=0;i<nl.count();i++){
		QDomNode gNode = nl.item(i);
		QDomElement elem = gNode.firstChildElement();
		bool gLocked=false;
		QColor gColor;
		QStringList seqs;
		while (!elem.isNull()){
			if (elem.tagName() == "locked"){
				gLocked=XMLHelper::stringToBool(elem.text().trimmed());
			}
			else if (elem.tagName() == "colour"){
				QStringList tmp = elem.text().split(',');
				gColor.setRgb(tmp.at(0).toInt(),tmp.at(1).toInt(),tmp.at(2).toInt());
			}
			else if (elem.tagName() == "sequences"){
				seqs=elem.text().split(',');
			}
			elem=elem.nextSiblingElement();
		}
		SequenceGroup *sg = new SequenceGroup();
		sg->setTextColour(gColor);
		sg->lock(gLocked);
		for (int s=0;s<seqs.size();s++){
			for (int ss=0;ss<sequences.size();ss++){
				if (seqs.at(s) == sequences.at(ss)->label){
					sg->addSequence(sequences.at(ss));
					break;
				}
			}
		}
		groups_.append(sg);
	}
	
	file.close();
}

void Project::exportFASTA(QString fname)
{
	FASTAFile ff(fname);
	QStringList l,seqs,c;
	for (int s=0;s<sequences.size();s++){
		l.append(sequences.at(s)->label);
		seqs.append(sequences.at(s)->noFlags());
		c.append(sequences.at(s)->comment);
	}
	ff.write(l,seqs,c);
}

void Project::exportClustalW(QString fname)
{
	ClustalFile cf(fname);
	QStringList l,seqs,c;
	for (int s=0;s<sequences.size();s++){
		l.append(sequences.at(s)->label);
		seqs.append(sequences.at(s)->noFlags());
		c.append(sequences.at(s)->comment);
	}
	cf.write(l,seqs,c);
}

void Project::closeIt()
{
	static bool closing = false;
	if (!closing)
	{
		closing = true;
		delete this;
		closing = false;
	}
}

void Project::createMainWindow()
{
	mainWindow_ = new SeqEditMainWin(this);
	connect(mainWindow_, SIGNAL(byebye()), this,SLOT(mainWindowClosed()));
	connect(sequenceSelection,SIGNAL(changed()),mainWindow_,SLOT(sequenceSelectionChanged()));
	mainWindow_->show();
}

void Project::mainWindowClosed()
{
	mainWindow_=NULL;
	closeIt();
}

// 
// Private members
//

void Project::init()
{
	sequenceSelection = new SequenceSelection();
	named_=false;
	dirty_=false;
	name_="unnamed.tsq";
	empty_=true;
}

int Project::getSeqIndex(QString l)
{
	// Get the index of the sequence with label l
	// Returns -1 if no match
	int i=0;
	QString t=l.stripWhiteSpace();// TO DO why is this here ?
	while ((i<sequences.count()) && (getLabelAt(i) != t)) i++;
	if (i==sequences.count())
		return -1;
	else
		return i;
}



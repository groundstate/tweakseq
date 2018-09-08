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

#include "AddInsertionsCmd.h"
#include "AlignmentCmd.h"
#include "AlignmentTool.h"
#include "Application.h"
#include "ClustalFile.h"
#include "ClustalO.h"
#include "CutResiduesCmd.h"
#include "CutSequencesCmd.h"
#include "ExcludeResiduesCmd.h"
#include "FASTAFile.h"
#include "GroupCmd.h"
#include "ImportCmd.h"
#include "LockResiduesCmd.h"
#include "MAFFT.h"
#include "Muscle.h"
#include "PasteCmd.h"
#include "PDBFile.h"
#include "Project.h"
#include "RenameCmd.h"
#include "ResidueLockGroup.h"
#include "ResidueSelection.h"
#include "SearchResult.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SequenceSelection.h"
#include "SeqEditMainWin.h"
#include "UngroupCmd.h"
#include "UnlockResiduesCmd.h"
#include "XMLHelper.h"

extern Application *app;

//
// Public members
//	

Project::Project()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	init();
	connect(&sequences,SIGNAL(changed()),this,SLOT(sequencesChanged()));
}

Project::~Project()
{
	while (!searchResults_.isEmpty())
		delete searchResults_.takeFirst();	
	emit searchResultsCleared();
	
	delete sequenceSelection;
	delete residueSelection;
	// FIXME and the rest ..
	if (muscleTool_) delete muscleTool_;
	if (clustalOTool_) delete clustalOTool_;
	if (mafftTool_) delete mafftTool_;
}

void Project::setMainWindow(SeqEditMainWin *mainwin)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	mainWindow_=mainwin;
	//connect(mainWindow_, SIGNAL(byebye()), this,SLOT(mainWindowClosed()));
	connect(residueSelection,SIGNAL(changed()),mainWindow_,SLOT(residueSelectionChanged()));
	connect(sequenceSelection,SIGNAL(changed()),mainWindow_,SLOT(sequenceSelectionChanged()));
}

void Project::setName(QString &n)
{
	name_=n;
}

bool Project::named()
{
	return named_;
}

bool Project::empty(){
	return sequences.isEmpty();
}

void Project::setAligned(bool a)
{
	aligned_=a;
	if (aligned_){
		consensusSequence.setSequences(&sequences);
		consensusSequence.calculate();
	}
	else{
		consensusSequence.setValid(false);
	}
}

bool Project::importSequences(QStringList &files,QString &errmsg)
{
	FASTAFile ff;
	ClustalFile cf;
	PDBFile pf;
	Structure structure;
	
	for (int f=0;f<files.size();f++){
		QString fname = files.at(f);
		bool ok = false;
		QStringList seqnames,seqs,comments;
		
		if (ff.isValidFormat(fname)){
			ff.setName(fname);
			ok = ff.read(seqnames,seqs,comments);
		}
		else if (cf.isValidFormat(fname)){
			cf.setName(fname);
			ok = cf.read(seqnames,seqs,comments);
		}
		else if (pf.isValidFormat(fname)){
			pf.setName(fname);
			ok = pf.read(seqnames,seqs,comments,&structure);
		}
		else{
			errmsg = "Unable to identify " + fname;
			emit uiUpdatesEnabled(true);
			return false;
		}
		
		if (ok){
			clearSearchResults();
			
			// Check for duplicates
			qDebug() << trace.header() << "checking for duplicates";
			QStringList currSeqNames;
			QList<Sequence *> currseq = sequences.sequences();
			for (int i=0;i<currseq.size();++i)
				currSeqNames.append(currseq.at(i)->name); // FIXME? removed trimmed()
			currSeqNames = currSeqNames + seqnames;
			QStringList dups = findDuplicates(currSeqNames);
			if (dups.size() > 0){
				errmsg="There are duplicated sequences in the file being imported:\n";
				for (int i=0; i< dups.size()-1;i++)
					errmsg = errmsg + dups.at(i) + ",";
				errmsg=errmsg+dups.last() + "\nYou will have to fix this.";
				emit uiUpdatesEnabled(true);
				return false;
			}
			
			if (seqs.size() == 0){
				errmsg="No sequences were imported";
				emit uiUpdatesEnabled(true);
				return false;
			}
			
			emit uiUpdatesEnabled(false);
			QList<Sequence *> newSeqs;
			
			for (int i=0;i<seqnames.size();i++){
				Sequence * seq = new Sequence(seqnames.at(i),seqs.at(i),comments.at(i),fname,true);
				newSeqs.append(seq);
				if (!structure.isEmpty()){
					seq->structureFile=fname;
					seq->structure=structure;
				}
			}
		
			undoStack_.push(new ImportCmd(this,newSeqs,"sequence import"));
			
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "added " << newSeqs.size();
			
		}
		else{
			errmsg ="Error while trying to read " + fname;
			emit uiUpdatesEnabled(true);
			return false;
		}
	}
	emit uiUpdatesEnabled(true);
	return true;
}

QString Project::getResidues(int i,int maskFlags)
{
	
	// First sequence has id 0
	// REMOVE_FLAGS in this context means return only those
	// residues which are not to be excluded from the alignment
	
	// It is intended that this function be mainly used to construct
	// a string suitable for use by an external alignment program
	
	QString r;
	int j,k=0;
	
	qDebug() << trace.header(__PRETTY_FUNCTION__)  << i << " " << maskFlags;
	// Return NULL if the index is out of range
	if ( i > sequences.sequences().count()-1)
		return NULL;
	else{
		r=sequences.sequences().at(i)->residues;
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


QString Project::getLabelAt(int i)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << i;
	// Return NULL if the index is out of range
	if (i<0 || i > sequences.size()-1)
		return NULL;
	else
		// strip white space from the end of the string
		return (sequences.sequences().at(i)->name).trimmed(); 
}

void Project::setAlignment(const QList<Sequence *> &newSequences,const QList<SequenceGroup *> &newGroups)
{
	// Used bu AlignmentCmd undo() and redo()
	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	clearSearchResults();
	// Clear the selections because they will be meaningless post alignment
	emit uiUpdatesEnabled(false);
	residueSelection->clear();
	sequenceSelection->clear();
	
	sequences.sequences() = newSequences;
	sequences.forceCacheUpdate(); 
	sequenceGroups = newGroups;
	
	emit uiUpdatesEnabled(true);
	dirty_=true;
}

//
//
//

void Project::undo()
{	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	undoStack_.undo();
}

void Project::redo()
{
	undoStack_.redo();
}

void Project::excludeSelectedResidues(bool add)
{
	undoStack_.push(new ExcludeResiduesCmd(this,residueSelection->residueGroups(),add,
		(add?"exclude residues":"remove exclusions")));
	dirty_=true;
}

void Project::lockSelectedResidues(bool lock)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	if (lock)
		undoStack_.push(new LockResiduesCmd(this,residueSelection->residueGroups(),"lock residues"));
	else
		undoStack_.push(new UnlockResiduesCmd(this,residueSelection->residueGroups(),"unlock residues"));
	dirty_=true;
}

bool Project::canLockSelectedResidues()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	// If the selection contains a locked residue, then we can't create a lock
	// ie overlapping locks are not allowed
	if (residueSelection->isLocked()) return false;
	
	// For insertions before a lock group, there has to be a contiguous block of insertions
	// before the lock group so that there are insertions that can be deleted to keep the locked 
	// portion in place
	for (int r=0;r<residueSelection->size();r++){
		ResidueGroup *rg = residueSelection->itemAt(r);
		if (rg->start==0) return false; // can't be anything before the start
		if (!(rg->sequence->isInsertion(rg->start-1))) return false;
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << " the selection is lockable";
	return true;
}

bool Project::canUnlockSelectedResidues()
{
	return residueSelection->isLocked();
}


bool Project::cutSelectedResidues()
{
	clearSearchResults();
	undoStack_.push(new CutResiduesCmd(this,residueSelection->residueGroups(),"cut insertions"));
	dirty_=true;
	return true;
}

bool Project::cutSelectedSequences()
{
	clearSearchResults();
	undoStack_.push(new CutSequencesCmd(this,"cut sequences"));
	dirty_ = true;
	return true;
}

void Project::pasteClipboard(Sequence *insertAfter)
{
	clearSearchResults();
	undoStack_.push(new PasteCmd(this,insertAfter,"paste sequences"));
	dirty_=true;
}


bool Project::canGroupSelectedSequences()
{
	if (sequenceSelection->size() < 2)
		return false;
	
	for (int g=0;g<sequenceGroups.size();g++){
		SequenceGroup *sg = sequenceGroups.at(g);
		int selcnt=0;
		for (int s=0;s<sg->size();s++){
			if (!sequenceSelection->contains(sg->itemAt(s))){
				// don't break because we need to test the others for overlap
			}
			else
				selcnt++;
		}
		// If only part of a group is selected then this is bad
		if (selcnt > 0 && selcnt != sg->size()){
			return false;
		}
	}
	
	// If everything in the selection is already grouped, and there's just one group then it's not groupable
	int nGrouped=0;
	QList<SequenceGroup*> groups;
	for (int s=0;s<sequenceSelection->size();s++){
		SequenceGroup *sg = sequenceSelection->itemAt(s)->group;
		if (sg != NULL){ 
			nGrouped++;
			if (!groups.contains(sg))
				groups.append(sg);
		}
	}
	if (nGrouped == sequenceSelection->size() && groups.size()==1)
		return false;
	return true;
}

bool Project::groupSelectedSequences(QColor gcol){
	// Require two or more sequences in the selection
	if (sequenceSelection->size() < 2)
		return false;
	
	// If the selection wholly contains one or more existing groups, then
	// the selection is merged into a single group
	QList<SequenceGroup *> mergeGroups;
	for (int g=0;g<sequenceGroups.size();g++){
		SequenceGroup *sg = sequenceGroups.at(g);
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
			qDebug() << trace.header() << "failed";
			return false;
		}
		
		if (contained){
			mergeGroups.append(sg);
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "group selected for merging";  
		}
	}
	
	undoStack_.push(new GroupCmd(this,sequenceSelection->sequences(),mergeGroups,gcol,"group sequences"));
	dirty_=true;
	return true;
}

void  Project::ungroupSelectedSequences()
{
	undoStack_.push(new UngroupCmd(this,"ungroup sequences"));
	dirty_=true;
}

void Project::ungroupAllSequences()
{
	while (!sequenceGroups.isEmpty()){
		SequenceGroup *sg = sequenceGroups.takeLast(); 
		sg->clear(); // this also removes sequencefrom their group
	}
	dirty_=true;
}

bool Project::canToggleLock()
{
	for (int s=0;s<sequenceSelection->size();s++){
		if (sequenceSelection->itemAt(s)->group == NULL){
			return false;
		}
	}
	QList<SequenceGroup *> selgroups;
	for (int g=0;g<sequenceGroups.size();g++){
		SequenceGroup *sg = sequenceGroups.at(g);
		bool contained=true;
		for (int s=0;s<sg->size();s++){
			if (!sequenceSelection->contains(sg->itemAt(s))){
				contained=false;
				break;
			}
		}
		if (contained){
			selgroups.append(sg);
		}	
	}
	return (selgroups.size() != 0);
}

void Project::lockSelectedGroups(bool lock){
	// The selection must contain all the sequences in one or more groups
	
	// First, check for an ungrouped sequence because this is fatal
	for (int s=0;s<sequenceSelection->size();s++){
		if (sequenceSelection->itemAt(s)->group == NULL){
			qDebug() << trace.header(__PRETTY_FUNCTION__) << " ungrouped sequence in the selection";
			return;
		}
	}
	QList<SequenceGroup *> selgroups;
	for (int g=0;g<sequenceGroups.size();g++){
		SequenceGroup *sg = sequenceGroups.at(g);
		bool contained=true;
		for (int s=0;s<sg->size();s++){
			if (!sequenceSelection->contains(sg->itemAt(s))){
				contained=false;
				break;
			}
		}
		if (contained){
			selgroups.append(sg);
			qDebug() << trace.header(__PRETTY_FUNCTION__) << " group in selection"; 
		}	
	}
	
	for (int sg=0;sg<selgroups.size();sg++){
		selgroups.at(sg)->lock(lock);
	}
	dirty_=true;
}

void Project::addGroupToSelection(SequenceGroup *selg)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	for (int g=0;g<sequenceGroups.size();g++){
		SequenceGroup *sg = sequenceGroups.at(g);
		if (selg==sg){
			for (int s=0;s<sg->size();s++)
				sequenceSelection->toggle(sg->itemAt(s));
			break;
		}
	}
	dirty_=true;
}



void Project::hideNonSelectedGroupMembers()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// FIXME This duplicates what's in the UI 
	// Check the selection
	QList<SequenceGroup *> groups;
	for (int s=0;s<sequenceSelection->size();s++){
		SequenceGroup *sg = sequenceSelection->itemAt(s)->group;
		if (sg != NULL ){
			if (!groups.contains(sg))
				groups.append(sg);
		}
	}
	
	if (groups.size() != 1) return;
	
	SequenceGroup  *selGroup = groups.at(0);
	for (int s=0; s<selGroup->size();s++){
		Sequence *seq = selGroup->itemAt(s);
		if (!sequenceSelection->contains(seq)){
			seq->visible=false;
		}
	}
	dirty_=true;
}

void Project::unhideAllGroupMembers()
{
	QList<SequenceGroup *> groups;
	for (int s=0;s<sequenceSelection->size();s++){
		SequenceGroup *g = sequenceSelection->itemAt(s)->group;
		if (g != NULL){
			if (!groups.contains(g))
				groups.append(g);
		}
	}
	
	if (groups.size() != 1) return;
	
	SequenceGroup  *selGroup = groups.at(0);
	
	for (int s=0; s<selGroup->size();s++){
		Sequence *seq = selGroup->itemAt(s);
		seq->visible = true;
	}
	dirty_=true;
}

bool Project::renameSequence(Sequence *seq,QString &newName)
{
	if (sequences.isUniqueName(newName)){
		undoStack_.push(new RenameCmd(this,seq,newName,"rename sequence"));
		dirty_=true;
		return true;
	}
	return false;
}

bool Project::modifySequenceProperties(Sequence *seq,Sequence *newSeq)
{
	bool modified=false;
	modified = modified || (seq->name != newSeq->name);
	modified = modified || (seq->comment != newSeq->comment);
	modified = modified || (seq->structureFile != newSeq->structureFile);
	modified = modified || (seq->structure.selectedChain != newSeq->structure.selectedChain);
	if (modified){
		return true;
	}
	return false;
}

void Project::addInsertions(QList<Sequence*> &seqs,int startCol,int stopCol,bool postInsert)
{
	clearSearchResults();
	undoStack_.push(new AddInsertionsCmd(this,seqs,startCol,stopCol,postInsert,"insertions"));
	dirty_=true;
}


void Project::setAlignmentTool(const QString & atool)
{
	if (atool == "clustalo" && clustalOTool_)
		alignmentTool_=clustalOTool_;
	else if (atool == "MUSCLE" && muscleTool_)
		alignmentTool_=muscleTool_;
	else if (atool == "MAFFT" && mafftTool_)
		alignmentTool_=mafftTool_;
}

//
//
//



bool Project::save(QString &fpathname)
{
	QString tmp;
	
	if(!fpathname.isNull()){
		QFileInfo fi(fpathname);
		path_=fi.path();
		name_=fi.fileName();
		named_=true;
	}
	
	QDomDocument saveDoc;
	QDomElement root = saveDoc.createElement("tweakseq");
	saveDoc.appendChild(root);
	
	QFileInfo fi(path_,name_);
	QFile f(fi.filePath());
	f.open(QIODevice::WriteOnly);
	QTextStream ts(&f);
	
	QDomElement el = saveDoc.createElement("version");
	root.appendChild(el);
	QDomText te = saveDoc.createTextNode(app->version());
	el.appendChild(te);
	
	
	el = saveDoc.createElement("settings");
	root.appendChild(el);

	tmp="unknown";
	if (sequenceDataType_==SequenceFile::Proteins)
		tmp="proteins";
	else if (sequenceDataType_==SequenceFile::DNA)
		tmp="dna";
	
	XMLHelper::addElement(saveDoc,el,"sequencedata",tmp);
	XMLHelper::addElement(saveDoc,el,"aligned",XMLHelper::boolToString(aligned_));
	
	for (int s=0;s<sequences.size();s++){
		Sequence *seq = sequences.sequences().at(s);
		
		QDomElement se = saveDoc.createElement("sequence");
		root.appendChild(se);
		
		XMLHelper::addElement(saveDoc,se,"name",seq->name);
		XMLHelper::addElement(saveDoc,se,"comment",seq->comment);		
		XMLHelper::addElement(saveDoc,se,"residues",seq->filter());
		XMLHelper::addElement(saveDoc,se,"source",seq->source);
		if (!seq->visible)
			XMLHelper::addElement(saveDoc,se,"visible",XMLHelper::boolToString(seq->visible));
		if (seq->bookmarked)
			XMLHelper::addElement(saveDoc,se,"bookmarked",XMLHelper::boolToString(seq->bookmarked));
		if (seq->originalName != seq->name)
			XMLHelper::addElement(saveDoc,se,"originalname",seq->originalName);
		if (!seq->structureFile.isEmpty())
			XMLHelper::addElement(saveDoc,se,"structurefile",seq->structureFile);
		QList<int> x = seq->exclusions();
		QString xs="";
		for (int xi=0;xi<x.size()-1;xi+=2){
			xs=xs + QString::number(x.at(xi)) + "-" + QString::number(x.at(xi+1));
			if (xi < x.size()-2) xs += ",";
		}
		XMLHelper::addElement(saveDoc,se,"exclusions",xs);
		if (!seq->structure.isEmpty()){
			QDomElement stre = saveDoc.createElement("structure");
			se.appendChild(stre);
			XMLHelper::addElement(saveDoc,stre,"source",seq->structure.source);
			XMLHelper::addElement(saveDoc,stre,"comment",seq->structure.comment);
			XMLHelper::addElement(saveDoc,stre,"selectedchain",QString::number(seq->structure.selectedChain));
			for (int c=0;c<seq->structure.chains.size();c++){
				QDomElement che = saveDoc.createElement("chain");
				stre.appendChild(che);
				XMLHelper::addElement(saveDoc,che,"id",seq->structure.chainIDs.at(c));
				XMLHelper::addElement(saveDoc,che,"residues",seq->structure.chains.at(c));
			}
		}
	}
	
	for (int g=0;g<sequenceGroups.size();g++){
		SequenceGroup *sg = sequenceGroups.at(g);
		QDomElement gel = saveDoc.createElement("group");
		root.appendChild(gel);
		XMLHelper::addElement(saveDoc,gel,"locked",(sg->locked()?"yes":"no"));
		QColor col = sg->textColour();
		QString str =  QString::number(col.red()) + QString(",") + QString::number(col.green()) + QString(",")+ QString::number(col.blue());
		XMLHelper::addElement(saveDoc,gel,"colour",str);
		
		QString seqs = "";
		for (int s=0;s<sg->size();s++){
			Sequence *seq = sg->itemAt(s);
			seqs += seq->name;
			if (s < sg->size()-1) seqs += ",";
		}
		XMLHelper::addElement(saveDoc,gel,"sequences",seqs);
	}
	
	for (int r=0;r<residueLockGroups.size();r++){
		ResidueLockGroup *rg = residueLockGroups.at(r);
		QDomElement rgel = saveDoc.createElement("residuelockgroup");
		root.appendChild(rgel);
		XMLHelper::addElement(saveDoc,rgel,"position",QString::number(rg->position()));
		
		QString seqs = "";
		QList<Sequence *> &rgseqs = rg->sequences;
		for (int s=0;s<rgseqs.size();s++){
			seqs += rgseqs.at(s)->name;
			if (s < rgseqs.size()-1) seqs += ",";
		}
		XMLHelper::addElement(saveDoc,rgel,"sequences",seqs);
	}
	
	// QSettings is not used because we want per-project settings
	writeSettings(saveDoc,root);
	
	saveDoc.save(ts,2);
	f.close();
	
	dirty_=false;

	return true;
}

void Project::load(QString &fname)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) << fname;
	QFileInfo fi(fname);
	path_=fi.path();
	name_=fi.fileName();
	named_=true; 

	QDomDocument doc;
	
	QFile file(fname);
	if ( !file.open(QIODevice::ReadOnly ) )
		return ;
	QString err; int errlineno,errcolno;
	if ( !doc.setContent( &file,true,&err,&errlineno,&errcolno ) )
	{	
		qDebug() << trace.header() << "Project::load() error at line " << errlineno;
		file.close();
		return ;
	}
	
	mainWindow_->readSettings(doc); // do this first so no jarring geometry changes
	
	QDomNodeList nl = doc.elementsByTagName("settings");
	if (nl.count() == 1){
		QDomNode sNode = nl.item(0);
		QDomElement elem = sNode.firstChildElement();
		QString txt;
		while (!elem.isNull()){
			if (elem.tagName() == "sequencedata"){
				txt=elem.text().trimmed();
				if (txt == "proteins")
					sequenceDataType_=SequenceFile::Proteins;
				else if (txt == "dna")
					sequenceDataType_=SequenceFile::DNA;
			}
			else if (elem.tagName() == "aligned"){
				aligned_=XMLHelper::stringToBool(elem.text().trimmed());
			}
			elem=elem.nextSiblingElement();
		}
	}
	
	QList<long> gids;
	
	// Get all the sequences
	emit uiUpdatesEnabled(false);
	nl = doc.elementsByTagName("sequence");
	for (int i=0;i<nl.count();i++){
		QDomNode sNode = nl.item(i);
		
		QString sName,sComment,sResidues,sSrc,sOriginalName,sStructureFile;
		bool sVisible = true;
		bool sBookmarked = false;
		Structure structure;
		QDomElement elem = sNode.firstChildElement();
		QList<int> exclusions;
		while (!elem.isNull()){
			if (elem.tagName() == "name")
				sName = elem.text().trimmed();
			else if (elem.tagName() == "originalname")
				sOriginalName=elem.text().trimmed();
			else if (elem.tagName() == "comment")
				sComment=elem.text().trimmed();
			else if (elem.tagName() == "residues")
				sResidues = elem.text().trimmed();
			else if (elem.tagName() == "source")
				sSrc = elem.text().trimmed();
			else if (elem.tagName() == "structurefile")
				sStructureFile = elem.text().trimmed();
			else if (elem.tagName() == "visible")
				sVisible = XMLHelper::stringToBool(elem.text().trimmed());
			else if (elem.tagName() == "bookmarked")
				sBookmarked = XMLHelper::stringToBool(elem.text().trimmed());
			else if (elem.tagName() == "exclusions"){
				QStringList sl = elem.text().trimmed().split(',');
				for (int sli=0;sli<sl.size();sli++){
					QStringList spair = sl.at(sli).split('-');
					if (spair.size() == 2){ // this catches empty exclusion lists
						int start = spair.at(0).toInt();
						int stop  = spair.at(1).toInt();
						qDebug() << trace.header() << start << " " << stop;
						exclusions.append(start);exclusions.append(stop);
					}
				}
			}
			else if (elem.tagName() == "structure"){
				QDomElement selem = elem.firstChildElement();
				while (!selem.isNull()){
					if (selem.tagName() == "source")
						structure.source = selem.text().trimmed();
					else if (selem.tagName() == "comment")
						structure.comment=selem.text().trimmed();
					else if (selem.tagName() == "selectedchain")
						structure.selectedChain=selem.text().toInt();
					else if (selem.tagName() == "chain"){
						QDomElement chelem = selem.firstChildElement();
						while (!chelem.isNull()){
							if (chelem.tagName() == "id"){
								structure.chainIDs.append(chelem.text());
							}
							else if (chelem.tagName() == "residues"){
								structure.chains.append(chelem.text());
							}
							chelem=chelem.nextSiblingElement();
						}
					}
					selem=selem.nextSiblingElement();
				}
			}
			elem=elem.nextSiblingElement();
		}
		Sequence *seq = sequences.append(sName,sResidues,sComment,sSrc,sVisible);
		seq->bookmarked=sBookmarked;
		seq->structureFile=sStructureFile;
		seq->structure=structure;
		
		// optional fields
		if (!sOriginalName.isEmpty())
			seq->originalName=sOriginalName;
		for (int x=0;x<exclusions.size()-1;x+=2)
			seq->exclude(exclusions.at(x),exclusions.at(x+1),true);
				 
	}	
	emit uiUpdatesEnabled(true);
	
	// Get all the sequence groups
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
				if (seqs.at(s) == sequences.sequences().at(ss)->name){
					sg->addSequence(sequences.sequences().at(ss));
					break;
				}
			}
		}
		sequenceGroups.append(sg);
	}

	// Get all the locked residue groups
	nl = doc.elementsByTagName("residuelockgroup");
	for (int i=0;i<nl.count();i++){
		QDomNode gNode = nl.item(i);
		QDomElement elem = gNode.firstChildElement();
		QStringList seqs;
		int position =0;
		while (!elem.isNull()){
			if (elem.tagName() == "position")
				position = elem.text().toInt();
			else if (elem.tagName() == "sequences"){
				seqs=elem.text().split(',');
			}
			elem=elem.nextSiblingElement();
		}
		ResidueLockGroup *rlg = new ResidueLockGroup();
		rlg->setPosition(position);
		for (int s=0;s<seqs.size();s++){
			for (int ss=0;ss<sequences.size();ss++){
				if (seqs.at(s) == sequences.sequences().at(ss)->name){
					rlg->addSequence(sequences.sequences().at(ss));
					sequences.sequences().at(ss)->residueLockGroup=rlg;
					break;
				}
			}
		}
		residueLockGroups.append(rlg);
	}
	
	readAlignmentToolSettings(doc);
	
	file.close();
	dirty_=false;
	empty_=false;
	mainWindow_->postLoadTidy();
	
}

void Project::writeSettings(QDomDocument &doc,QDomElement &root)
{
	mainWindow_->writeSettings(doc,root);
	if (clustalOTool_)
		clustalOTool_->writeSettings(doc,root);
	if (muscleTool_)
		muscleTool_->writeSettings(doc,root);
	if (mafftTool_)
		mafftTool_->writeSettings(doc,root);;
}

void Project::readSettings(QDomDocument &doc)
{
	readAlignmentToolSettings(doc);
}


void Project::exportFASTA(QString fname,bool removeExclusions)
{

	FASTAFile ff(fname);
	QStringList l,seqs,c;
	for (int s=0;s<sequences.size();s++){
		l.append(sequences.sequences().at(s)->name);
		seqs.append(sequences.sequences().at(s)->filter(removeExclusions));
		c.append(sequences.sequences().at(s)->comment);
	}
	ff.write(l,seqs,c);
}

void Project::exportSelectionFASTA(QString fname,bool removeExclusions)
{
	FASTAFile ff(fname);
	QStringList l,seqs,c;
	
	for (int s=0;s<sequenceSelection->size();s++){
		l.append(sequenceSelection->itemAt(s)->name);
		seqs.append(sequenceSelection->itemAt(s)->filter(removeExclusions));
		c.append(sequenceSelection->itemAt(s)->comment);
	}
	ff.write(l,seqs,c);
}

void Project::exportClustalW(QString fname,bool removeExclusions)
{
	ClustalFile cf(fname);
	QStringList l,seqs,c;
	for (int s=0;s<sequences.size();s++){
		l.append(sequences.sequences().at(s)->name);
		seqs.append(sequences.sequences().at(s)->filter(removeExclusions));
		c.append(sequences.sequences().at(s)->comment);
	}
	cf.write(l,seqs,c);
}

void Project::readNewAlignment(QString fname,bool isFullAlignment){
	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	FASTAFile fin(fname); // FIXME FASTA output is hardcoded at present but may be optional eventually
	QStringList newlabels,newseqs,newcomments;
	fin.read(newlabels,newseqs,newcomments);
	
	// No need to emit uiUpdatesEnabled(), because we are not modifying Project data here
	
	QList<Sequence *>      oldSeqs   = sequences.sequences();
	QList<SequenceGroup *> oldGroups = sequenceGroups;
	qDebug() << trace.header(__PRETTY_FUNCTION__) << oldSeqs.size() << " " << oldGroups.size();
	Sequences newSequences;
	QList<SequenceGroup*> newGroups;
		
	if (isFullAlignment){
	
		aligned_=true;
		
		// Create the new sequences, in the order of the new alignment
		for (int snew=0;snew<newseqs.size();snew++){
			Sequence *oldSeq = sequences.getSequence(newlabels.at(snew));
			if (NULL != oldSeq){
				Sequence *newSeq = new Sequence(newlabels.at(snew),newseqs.at(snew),oldSeq->comment,oldSeq->source,oldSeq->visible,oldSeq->structureFile);
				// carry forward any extra information
				newSeq->originalName=oldSeq->originalName;
				newSeq->bookmarked=oldSeq->bookmarked;
				newSeq->structure=oldSeq->structure;
				newSequences.append(newSeq);
			}
			else{
				qDebug() << trace.header(__PRETTY_FUNCTION__) << "missed " << newlabels.at(snew); 
			}	
		}
	
		// Recreate the groups for the new sequences
		for (int g=0;g<oldGroups.size();g++){
			SequenceGroup *newGroup = new SequenceGroup();
			newGroup->setTextColour(oldGroups.at(g)->textColour());
			newGroup->lock(oldGroups.at(g)->locked());
			newGroups.append(newGroup);
			for (int s=0;s<oldGroups.at(g)->size();s++){
				Sequence *oldGroupedSeq=oldGroups.at(g)->itemAt(s);
				Sequence *newGroupedSeq = newSequences.getSequence(oldGroupedSeq->name);
				if (NULL!=newGroupedSeq){
					newGroup->addSequence(newGroupedSeq);
				}
				else{
					qDebug() << trace.header(__PRETTY_FUNCTION__) << "missed grouping " << oldGroupedSeq->name;
				}
			}
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "new group created with " << newGroup->size() << " members";
		}
		
		
	}
	else{
		
		aligned_=false;
		
		SequenceSelection *sel = sequenceSelection;
		
		// Make a copy of the old sequences
		for (int s=0;s<oldSeqs.size();s++){
			Sequence *oldSeq = oldSeqs.at(s);
			Sequence *newSeq = new Sequence(oldSeq->name,oldSeq->residues,oldSeq->comment,oldSeq->source,oldSeq->visible);
			newSeq->bookmarked=oldSeq->bookmarked;
			newSequences.append(newSeq);
		}
		
		// Make a copy of the old groups
		for (int g=0;g<oldGroups.size();g++){
			SequenceGroup *newGroup = new SequenceGroup();
			newGroup->setTextColour(oldGroups.at(g)->textColour());
			newGroup->lock(oldGroups.at(g)->locked());
			newGroups.append(newGroup);
			for (int s=0;s<oldGroups.at(g)->size();s++){
				Sequence *oldGroupedSeq=oldGroups.at(g)->itemAt(s);
				Sequence *newGroupedSeq = newSequences.getSequence(oldGroupedSeq->name);
				if (NULL!=newGroupedSeq){
					newGroup->addSequence(newGroupedSeq);
				}
				else{
					qDebug() << trace.header(__PRETTY_FUNCTION__) << "missed grouping " << oldGroupedSeq->name;
				}
			}
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "new group created with " << newGroup->size() << " members";
		}
		
		// Now update the selected sequences with their new alignment
		// and put them in the right position
		// The assumption is that the selection is contiguous
		// so find the index of the first occurring selected sequence in the old alignment
		int indexFirstSelSeq=-1;
		for (int s=0;s<oldSeqs.size();s++){
			if (sel->contains(oldSeqs.at(s))){
				indexFirstSelSeq =s;
				break;
			}
		}
	
		qDebug() << trace.header(__PRETTY_FUNCTION__) <<"first selected sequence index=" << indexFirstSelSeq;
		
		for (int l=0;l<newlabels.size();l++){
			
			Sequence *seq = newSequences.getSequence(newlabels.at(l));
			if (seq){
				seq->residues=newseqs.at(l);
				// move it to its new home
				int oldIndex = newSequences.getIndex(newlabels.at(l)); // note, after a sequence is moved, positions have all changed so use newSequences!
				newSequences.sequences().move(oldIndex,indexFirstSelSeq + l);
				qDebug() << trace.header(__PRETTY_FUNCTION__) << "move " << newlabels.at(l) << " " << oldIndex << " " << indexFirstSelSeq + l;
			}
			else{
				qDebug() << trace.header(__PRETTY_FUNCTION__) << "missed aligned sequence " << newlabels.at(l);
			}
		}
		
	}

	// Pushing onto the stack triggers redo(), so this will finish things off (call setAlignment(), in particular
	undoStack_.push(new AlignmentCmd(this,oldSeqs,oldGroups,newSequences.sequences(),newGroups,aligned_,"alignment"));
	
}

int  Project::search(const QString &needle)
{
	clearSearchResults();
	int len = needle.length();
	QRegExp rx(needle);
	for (int s=0;s<sequences.size();s++){
		int pos = 0;
		Sequence *seq = sequences.sequences().at(s);
		QString residues = seq->filter(false);
		while ((pos = rx.indexIn(residues, pos)) != -1) {
			searchResults_.append(new SearchResult(seq,pos,pos+len-1));
			pos += rx.matchedLength();
		}
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << " found " << searchResults_.size();
	setSearchResultFlags(true);
	return searchResults_.size();
	
}

void Project::setSearchResultFlags(bool apply)
{
	// The search results are in Sequence order so 
	// searching is easy
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	int sr=0,s=0;
	QList<Sequence *> &seqs = sequences.sequences();
	if (seqs.size() == 0) return;
	while (sr<searchResults_.size()){
		SearchResult *result = searchResults_.at(sr);
		if (result->sequence == seqs.at(s)){
			seqs.at(s)->highlight(result->start,result->stop,apply);
			sr++;
		}
		else{
			s++;
			if (s == seqs.size()) break; // makes sure that we break out of the while loop
		}
	}
}

//
//	Public slots:
//

void Project::enableUIupdates(bool enable)
{
	emit uiUpdatesEnabled(enable);
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
	connect(residueSelection,SIGNAL(changed()),mainWindow_,SLOT(residueSelectionChanged()));
	connect(sequenceSelection,SIGNAL(changed()),mainWindow_,SLOT(sequenceSelectionChanged()));
	mainWindow_->readSettings(app->defaultSettings());
	mainWindow_->show();
}

void Project::mainWindowClosed()
{
	mainWindow_=NULL;
	closeIt();
}

void Project::sequencesChanged()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	dirty_=true;
}

void Project::clearSearchResults()
{
	setSearchResultFlags(false);
	while (!searchResults_.isEmpty()){
		delete searchResults_.takeLast();
	}
	emit searchResultsCleared();
}

// 
// Private members
//

void Project::init()
{
	residueSelection = new ResidueSelection();
	sequenceSelection = new SequenceSelection();

	named_=false;
	dirty_=false;
	name_="unnamed.tsq";
	empty_=true;
	sequenceDataType_=SequenceFile::Unknown;
	aligned_=false;
	
	mafftTool_= NULL;
	if (app->alignmentToolAvailable("MAFFT"))
		mafftTool_ = new MAFFT();
	
	muscleTool_= NULL;
	if (app->alignmentToolAvailable("MUSCLE"))
		muscleTool_ = new Muscle();
	
	clustalOTool_ = NULL;
	if (app->alignmentToolAvailable("clustalo"))
		clustalOTool_ = new ClustalO();
	
	alignmentTool_= NULL;
	
	QDomDocument &doc = app->defaultSettings();
	readSettings(doc);
	
}

void Project::readAlignmentToolSettings(QDomDocument &doc)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__) ;
	
	if (muscleTool_){
		muscleTool_->readSettings(doc);
		if (muscleTool_->preferred())
			alignmentTool_=muscleTool_;
	}
	
	if (clustalOTool_){
		clustalOTool_->readSettings(doc);
		if (clustalOTool_->preferred())
			alignmentTool_=clustalOTool_;
	}
	
	if (mafftTool_){
		mafftTool_->readSettings(doc);
		if (mafftTool_->preferred())
			alignmentTool_=mafftTool_;
	}
}



QStringList Project::findDuplicates(QStringList &sl)
{
	QStringList ret;
	QSet<QString> visited;
	for (int i=0;i<sl.size();++i){
		const QString &s = sl.at(i);
		if (visited.contains(s)){
			ret.append(s);
			continue;
		}
		visited.insert(s);
	}
	ret.removeDuplicates();
	return ret;
}


int Project::getSeqIndex(QString l)
{
	// Get the index of the sequence with label l
	// Returns -1 if no match
	int i=0;
	QString t=l.trimmed();// TO DO why is this here ?
	while ((i<sequences.sequences().size()) && (getLabelAt(i) != t)) i++;
	if (i==sequences.sequences().size())
		return -1;
	else
		return i;
}

int Project::getGroupIndex(SequenceGroup *sg)
{
	for (int i=0;i<sequenceGroups.size();i++){
		if (sequenceGroups.at(i) == sg) return i;
	}
	return -1;
}

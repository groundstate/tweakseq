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

#include "AlignmentTool.h"
#include "Application.h"
#include "ClustalFile.h"
#include "ClustalO.h"
#include "FASTAFile.h"
#include "Muscle.h"
#include "Project.h"
#include "ResidueSelection.h"
#include "Sequence.h"
#include "SequenceGroup.h"
#include "SequenceSelection.h"
#include "SeqEditMainWin.h"
#include "UndoAlignmentCommand.h"
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
	delete sequenceSelection;
	delete residueSelection;
	// FIXME and the rest ..
	if (muscleTool_) delete muscleTool_;
	if (clustalOTool_) delete clustalOTool_;
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
		return (sequences.sequences().at(i)->label).trimmed(); 
}

void Project::setAlignment(const QList<Sequence *> &newSequences,const QList<SequenceGroup *> &newGroups)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	// Clear the selections because they will be meaningless post alignment
	emit loadingSequences(true);
	residueSelection->clear();
	sequenceSelection->clear();
	
	sequences.clear();
	
	while (!sequenceGroups.isEmpty())
		delete sequenceGroups.takeFirst();
	
	for (int g=0;g<newGroups.size();g++){
		SequenceGroup *sg = new SequenceGroup();
		*sg = *(newGroups.at(g));
		sg->clear();
		sequenceGroups.append(sg);
	}
	
	for (int s=0;s<newSequences.size();s++){
		Sequence *seq = new Sequence();
		*seq = *(newSequences.at(s));
		sequences.append(seq);
		if (newSequences.at(s)->group){
			int gi;
			for (gi=0;gi<newGroups.size();gi++){
				if (newGroups.at(gi) == newSequences.at(s)->group) break;
			}
			sequenceGroups.at(gi)->addSequence(seq);
		}
	}
	emit loadingSequences(false);
	dirty_=true;
}

//
//
//

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
		SequenceGroup * sg = sequenceSelection->itemAt(s)->group;
		if (sg){ 
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
	QList<SequenceGroup *> selgroups;
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
			selgroups.append(sg);
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "group selected for merging";  
		}
	}
	
	for (int g=0;g<selgroups.size();g++){
		SequenceGroup *sg = selgroups.at(g);
		sequenceGroups.removeOne(sg);
		delete sg;
	}
	
	// All sorted, so create the group

	SequenceGroup *sg = new SequenceGroup();
	sg->setTextColour(gcol);
	sequenceGroups.append(sg);
	for ( int s=0;s<sequenceSelection->size();s++){
		Sequence *seq = sequenceSelection->itemAt(s);
		sg->addSequence(seq);
	}
	qDebug() << trace.header(__PRETTY_FUNCTION__) << "new group ";
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
			SequenceGroup *sg = seq->group; // save this, cos removing it from the groups sets ptr to NULL
			seq->group->removeSequence(seq); // this also removes the parent group from the sequence
			// if we have now removed all of the visible sequences in the group, make the hidden sequences
			// visible again
			sg->enforceVisibility();
		}
	}
	// Remove any empty groups
	int g=0;
	while (g<sequenceGroups.size()){
		SequenceGroup *sg = sequenceGroups.at(g);
		if (sg->size() == 0){
			sequenceGroups.removeOne(sg);
			qDebug() << trace.header(__PRETTY_FUNCTION__)  << " removing empty group";
			delete sg;
		}
		else
			g++;
	}
	
	dirty_=true;
	return true;
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

bool Project::cutSelectedResidues()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	for (int rg=0;rg<residueSelection->size();rg++){
		ResidueGroup *resGroup = residueSelection->itemAt(rg);
		resGroup->sequence->remove(resGroup->start,resGroup->stop-resGroup->start+1);
	}
	residueSelection->clear();
	return true;
}

bool Project::cutSelectedSequences()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	QList<Sequence*> &seqs = sequences.sequences();
	int s=0;
	QList<Sequence *> cutSeqs;
	// Building the list of cut sequences this way orders the selection in
	// the correct order, recognizing that the selection may be in random order
	for (s=0;s<seqs.size();s++){
		if (sequenceSelection->contains(seqs.at(s)))
			cutSeqs.append(seqs.at(s));
	}
	
	// Now we have to check whether any groups have been selected and include any
	// non-visible items in the right order
	QList<SequenceGroup *> sgl =  sequenceSelection->uniqueGroups();
	for (int g = 0; g < sgl.size(); g++){
		SequenceGroup *sg = sgl.at(g);
		bool groupSelected=true;
		for (s=0; s< sg->size(); s++){
			Sequence *seq = sg->itemAt(s);
			if (seq->visible && !sequenceSelection->contains(seq)){
				groupSelected=false;
				break;
			}
		}
		if (groupSelected){
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "group cut : size = " << sg->size();
			// Create the ordered list of sequences in the group
			QList<Sequence *> groupSeqs;
			for (s=0;s<seqs.size();s++){
				if (sg->contains(seqs.at(s))){
					groupSeqs.append(seqs.at(s));
				}
			}
			// Now remove the visible group members from the list of cut sequences
			s=0;
			while (s<cutSeqs.size()){
				Sequence *seq = cutSeqs.at(s);
				if (sg->contains(seq))
					cutSeqs.takeAt(s);
				else
					s++;
			}
			for (s=0;s<groupSeqs.size();s++) // Don't try to insert in the 'right' place
				cutSeqs.append(groupSeqs.at(s));
		}
		else{
			qDebug() << trace.header(__PRETTY_FUNCTION__) << "group not cut";
		}
	}
	
	// Order the cut sequences (again)
	// This is easier than trying to insert in the right place
	
	QList<Sequence *> orderedCutSeqs;
	for (s=0;s<seqs.size();s++){
		Sequence *seq = seqs.at(s);
		if (cutSeqs.contains(seq))
			orderedCutSeqs.append(seq);
	}
	// Now that there is a complete list of cut sequences, remove them
	s=0;
	while (s<seqs.size()){
		if (orderedCutSeqs.contains(seqs.at(s)))
			seqs.takeAt(s);
		else
			s++;
	}
	app->clipboard().setSequences(orderedCutSeqs);
	return true;
}

void Project::hideNonSelectedGroupMembers()
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// FIXME This duplicates what's in the UI 
	// Check the selection
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
		if (!sequenceSelection->contains(seq)){
			seq->visible=false;
		}
	}
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
}
		
void Project::setAlignmentTool(const QString & atool)
{
	if (atool == "clustalo" && clustalOTool_)
		alignmentTool_=clustalOTool_;
	else if (atool == "MUSCLE" && muscleTool_)
		alignmentTool_=muscleTool_;
}

//
//
//

//
//	Public slots:
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
		
	for (int s=0;s<sequences.size();s++){
		Sequence *seq = sequences.sequences().at(s);
		
		QDomElement se = saveDoc.createElement("sequence");
		root.appendChild(se);
		
		XMLHelper::addElement(saveDoc,se,"name",seq->label);
		XMLHelper::addElement(saveDoc,se,"comment",seq->comment);		
		XMLHelper::addElement(saveDoc,se,"residues",seq->filter());
		XMLHelper::addElement(saveDoc,se,"source",seq->source);
		XMLHelper::addElement(saveDoc,se,"visible",(seq->visible?"yes":"no"));
		QList<int> x = seq->exclusions();
		QString xs="";
		for (int xi=0;xi<x.size()-1;xi+=2){
			xs=xs + QString::number(x.at(xi)) + "-" + QString::number(x.at(xi+1));
			if (xi < x.size()-2) xs += ",";
		}
		XMLHelper::addElement(saveDoc,se,"exclusions",xs);
		
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
			seqs += seq->label;
			if (s < sg->size()-1) seqs += ",";
		}
		XMLHelper::addElement(saveDoc,gel,"sequences",seqs);
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
	
	QList<long> gids;
	
	// Get all the sequences
	emit loadingSequences(true);
	QDomNodeList nl = doc.elementsByTagName("sequence");
	for (int i=0;i<nl.count();i++){
		QDomNode sNode = nl.item(i);
		
		QString sName,sComment,sResidues,sSrc;
		bool sVisible = true;
		
		QDomElement elem = sNode.firstChildElement();
		QList<int> exclusions;
		while (!elem.isNull()){
			if (elem.tagName() == "name")
				sName = elem.text().trimmed();
			else if (elem.tagName() == "comment")
				sComment=elem.text().trimmed();
			else if (elem.tagName() == "residues")
				sResidues = elem.text().trimmed();
			else if (elem.tagName() == "source")
				sSrc = elem.text().trimmed();
			else if (elem.tagName() == "visible")
				sVisible = XMLHelper::stringToBool(elem.text().trimmed());
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
			elem=elem.nextSiblingElement();
		}
		Sequence *seq = sequences.add(sName,sResidues,sComment,sSrc,sVisible);
		for (int x=0;x<exclusions.size()-1;x+=2)
			seq->exclude(exclusions.at(x),exclusions.at(x+1));
				 
	}	
	emit loadingSequences(false);
	
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
				if (seqs.at(s) == sequences.sequences().at(ss)->label){
					sg->addSequence(sequences.sequences().at(ss));
					break;
				}
			}
		}
		sequenceGroups.append(sg);
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
		muscleTool_->writeSettings(doc,root);;
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
		l.append(sequences.sequences().at(s)->label);
		seqs.append(sequences.sequences().at(s)->filter(removeExclusions));
		c.append(sequences.sequences().at(s)->comment);
	}
	ff.write(l,seqs,c);
}

void Project::exportSelectionFASTA(QString fname,bool removeExclusions)
{
	FASTAFile ff(fname);
	QStringList l,seqs,c;
	
	sequenceSelection->order();
	
	for (int s=0;s<sequenceSelection->size();s++){
		l.append(sequenceSelection->itemAt(s)->label);
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
		l.append(sequences.sequences().at(s)->label);
		seqs.append(sequences.sequences().at(s)->filter(removeExclusions));
		c.append(sequences.sequences().at(s)->comment);
	}
	cf.write(l,seqs,c);
}

void Project::readNewAlignment(QString fname,bool isFullAlignment){
	
	// Make a copy of the existing alignment and groups
	QList<Sequence *> oldSeqs;
	QList<SequenceGroup *> oldGroups;
	
	for (int g=0;g<sequenceGroups.size();g++){
		SequenceGroup *sg = new SequenceGroup();
		*sg = *sequenceGroups.at(g);
		sg->clear();
		oldGroups.append(sg);
	}
	
	for (int s=0;s<sequences.size();s++){
		Sequence *seq = new Sequence();
		*seq = *(sequences.sequences().at(s));
		oldSeqs.append(seq);
		if (sequences.sequences().at(s)->group){
			int gi = getGroupIndex(sequences.sequences().at(s)->group);
			oldGroups.at(gi)->addSequence(seq);
		}
	}
	
	FASTAFile fin(fname);
	QStringList newlabels,newseqs,newcomments;
	fin.read(newlabels,newseqs,newcomments);
	
	//qDebug() << trace.header() << newseqs.size() << " " << sequences.size();

	// Groupings are preserved
	
	// Preserve tree-order
	
	emit loadingSequences(true);
	
	if (isFullAlignment){
		// Sequences are moved to their new position in the alignment and updated
		for (int snew=0;snew<newseqs.size();snew++){
			int sold=0;
			while ((sold < sequences.size())){
				if (newlabels.at(snew) == (sequences.sequences().at(sold)->label.trimmed())) // remember, labels are padded at the end with spaces
					break;
				sold++;
			}
			if (sold < sequences.size()){
				sequences.move(sold,snew);
				sequences.sequences().at(snew)->residues = newseqs.at(snew);
			}
			else{
				qDebug() << trace.header(__PRETTY_FUNCTION__) << " missed " << newlabels.at(snew); 
			}
		}
	}
	else{
		// The saved selection is examined and each sequence is replaced by the corresponding sequence in the new alignment
		SequenceSelection *sel = sequenceSelection;
		Sequences &seqs = sequences;
		
		QList<Sequence *> newSequences;	
		// Make a list of pointers to the Sequences in the new alignment
		for (int s=0;s<newlabels.size();s++){
			int seqIndex = seqs.getIndex(newlabels.at(s));
			newSequences.append(seqs.sequences().at(seqIndex));
		}
		// Make a list of indices of the sequences in the selection
		QList<int> selIndices;
		for (int s=0;s<sel->size();s++){
			selIndices.append(seqs.getIndex(sel->itemAt(s)->label));
		}
		
		for (int s=0;s<selIndices.size();s++){
			seqs.sequences().replace(selIndices.at(s),newSequences.at(s));
			seqs.sequences().at(selIndices.at(s))->residues = newseqs.at(s); // update the residues
		}
		
	}

	undoStack().push(new UndoAlignmentCommand(this,oldSeqs,oldGroups,sequences.sequences(),sequenceGroups,"alignment"));
	
	// Tidy up time
	while (!oldSeqs.isEmpty())
		delete oldSeqs.takeFirst();
	while (!oldGroups.isEmpty())
		delete oldGroups.takeFirst();
	
	emit loadingSequences(false);
	
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
	dirty_=true;
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

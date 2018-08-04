// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2018  Merridee A. Wouters, Michael J. Wouters
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

#include <iostream>
#include <zlib.h>
#include <stdio.h>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "PDB.h"

using namespace std;

static char oneLetterCode(QString r);


char oneLetterCode(QString r){
	char c;

// FIXME this is used a lot so maybe a hash table/lookup ??

	if (r=="ALA")
		c='A';
 else if (r=="ARG")
   c='R';
 else if (r=="ASN")
   c='N';
 else if (r=="ASP")
   c='D';
 else if (r=="ASX")
   c='B';
 else if (r=="CYS")
   c='C';
 else if (r=="GLN")
   c='Q';
 else if (r=="GLU")
   c='E';
 else if (r=="GLX")
   c='Z';
 else if (r=="GLY")
   c='G';
 else if (r=="HIS")
   c='H';
 else if (r=="ILE")
   c='I';
 else if (r=="LEU")
   c='L';
 else if (r=="LYS")
   c='K';
 else if (r=="MET" || r=="MSE")
   c='M';
 else if (r=="PHE")
   c='F';
 else if (r=="PRO")
   c='P';
 else if (r=="SER")
   c='S';
 else if (r=="THR")
   c='T';  
 else if (r=="TRP")
   c='W';
 else if (r=="TYR")
   c='Y';
 else if (r=="UNK")
   c='?';
 else if (r=="VAL")
   c='V';
 else
	c='!';
 return c;
}

void PDBTitle::read(QTextStream *ts, QString *l){

	// *l is a line of text from the input stream
	// It can be null.
	// The intention is that the various read member functions
	// are called sequentially, the functions returning the
	// last line read

	qDebug() << trace.header(__PRETTY_FUNCTION__);
// 	
	*l=ts->readLine();
	
	// Columns 11-50 contain classification
	classification = (l->mid(10,40)).trimmed();
	// 51-59 date
	depDate = l->mid(50,9);
	// 63-66 IDcode
	IDcode = l->mid(62,4);
	IDcode=IDcode.trimmed();
	*l=ts->readLine(); 
	
	// OBSLTE
	
	while (l->indexOf("OBSLTE")==0){
		*l=ts->readLine();
	}	

	// TITLE
	while (l->indexOf("TITLE")==0){
		title.append((l->mid(10,70)).trimmed());
		*l=ts->readLine();
	}
	
	// CAVEAT
	while (l->indexOf("CAVEAT")==0){
		*l=ts->readLine();
	}
	
	// COMPND
	
	while (l->indexOf("COMPND")==0){
		*l=ts->readLine();
	}
	
	// SOURCE
	while (l->indexOf("SOURCE")==0){
		*l=ts->readLine();
	}
	
	// KEYWDS
	while (l->indexOf("KEYWDS")==0){
		*l=ts->readLine();
	}
	
	// EXPDTA
	while (l->indexOf("EXPDTA")==0){
		*l=ts->readLine();
	}
	
	// AUTHOR
	while (l->indexOf("AUTHOR")==0){
		*l=ts->readLine();
	}
	
	// REVDAT
	while (l->indexOf("REVDAT")==0){
		*l=ts->readLine();
	}
	
	// SPRSDE
	while (l->indexOf("SPRSDE")==0){
		*l=ts->readLine();
	}
	
	// JRNL
	while (l->indexOf("JRNL")==0){
		*l=ts->readLine();
	}
	
}

//
//
//
void PDBRemarks::read(QTextStream *ts,QString *l)
{
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	while (l->indexOf("REMARK")==0){
		*l=ts->readLine();
	}
}

//
//
//

PDBChain::PDBChain(QChar cid,long nr){
	ID=cid;
	nResidues=nr;
	residues="";
}



//
//
//
void PDBPrimStruct::read(QTextStream *ts, QString *l){

	PDBChain *chain;
	bool result;
	long nRead,nToRead,i;
	QString res;

	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
  //DBREF section
	QTextStream cout(stdout,QIODevice::WriteOnly);
	// cout << *l ;
		
	while (l->indexOf("DBREF")==0){
		*l=ts->readLine();
	}
	
	//SEQADV section
	
	while (l->indexOf("SEQADV")==0){
		*l=ts->readLine();
	}
	
	//SEQRES section
	if (l->indexOf("SEQRES")==0){
		nChains = 1 ;
		nRead = 0 ;
		// chainID in column 12
		// number of residues in columns 14-17
		QChar chainID = (l->mid(11,1)).at(0);
		if (chainID==' ') chainID='A';
		chain = new PDBChain(chainID, (l->mid(13,4).toLong(&result)));
		chains.append(chain);
		while (l->indexOf("SEQRES")==0){
			// Check the chainID in column 12
			chainID = (l->mid(11,1)).at(0);
			if (chainID==' ') chainID='A';
			if (chain->ID != chainID){		
				chain = new PDBChain(chainID, (l->mid(13,4).toLong(&result)));
				chains.append(chain);
				nRead=0;
				nChains++;
				// 
			}
			if ((chain->nResidues - nRead) >= 13)
				nToRead=13;
			else
				nToRead=chain->nResidues - nRead;
			for (i=1;i<=nToRead;i++){
				chain->residues +=
					oneLetterCode((l->mid(19+(i-1)*4,3)).trimmed());
				
				// cout << res << "\n";
			}
			nRead+=nToRead;
			*l=ts->readLine();
		}
	}
}

void PDBHeterogen::read(QTextStream *ts, QString *l){
	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	//HET
	while (l->indexOf("HET")==0){
		*l=ts->readLine();
	}
	
	//HETNAM
	while (l->indexOf("HETNAM")==0){
		*l=ts->readLine();
	}
	
	//HETSYN
	while (l->indexOf("HETSYN")==0){
		*l=ts->readLine();
	}
	
	//FORMUL
	while (l->indexOf("FORMUL")==0){
		*l=ts->readLine();
	}
	
}

void PDBSecStruct::read(QTextStream *ts, QString *l){

	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// HELIX
	while (l->indexOf("HELIX")==0){
		*l=ts->readLine();
	}
	
	// SHEET
	while (l->indexOf("SHEET")==0){
		*l=ts->readLine();
	}
	
	// TURN
	while (l->indexOf("TURN")==0){
		*l=ts->readLine();
	}

}

void PDBConnectivity::read(QTextStream *ts, QString *l){

	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// SSBOND
	while (l->indexOf("SSBOND")==0){
		*l=ts->readLine();
	}
	
	// LINK
	while (l->indexOf("LINK")==0){
		*l=ts->readLine();
	}
	
	// HYDBND
	while (l->indexOf("HYDBND")==0){
		*l=ts->readLine();
	}
		
	// SLTBRG
	while (l->indexOf("SLTBRG")==0){
		*l=ts->readLine();
	}
	
	// CISPEP
	while (l->indexOf("CISPEP")==0){
		*l=ts->readLine();
	}
	
}

void PDBFeatures::read(QTextStream *ts, QString *l){
	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// SITE
	
	while (l->indexOf("SITE")==0){
		*l=ts->readLine();
	}
	
}


void PDBCrystal::read(QTextStream *ts, QString *l){
	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// CRYST1
	while (l->indexOf("CRYST1")==0){
		*l=ts->readLine();
	}
	
}

void PDBCoordTransform::read(QTextStream *ts, QString *l){
	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	// ORIGX
	while (l->indexOf("ORIGX")==0){
		*l=ts->readLine();
	}
	
	// SCALE
	while (l->indexOf("SCALE")==0){
		*l=ts->readLine();
	}
	
	// MTRIX
	while (l->indexOf("MTRIX")==0){
		*l=ts->readLine();
	}
	
	// TVECT
	while (l->indexOf("TVECT")==0){
		*l=ts->readLine();
	}
	
}

PDBAtom::PDBAtom()
{
	// Does bugger all. really
	// TO DO these classes SUCK
}

PDBAtom::PDBAtom(PDBAtom *t)
{
	kind=t->kind; 
	name=t->name;
	altLoc=t->altLoc;
	resName=t->resName; 
	chainID=t->chainID;
	resSeq=t->resSeq;
	iCode=t->iCode;
	//r=t->r;
	occupancy=t->occupancy;
	tempFactor=t->tempFactor;
	segID=t->segID;
	element=t->element;
	charge=t->charge;
}

PDBAtom::PDBAtom(QString *l,long k)
{
	// FIXME HETATMs etc to be treated differently ?
	QString s;
	bool convFlag;
	
	kind = k;
	name = (l->mid(12,4)).trimmed();
	altLoc = (l->mid(16,1)).at(0);
	resName = oneLetterCode(l->mid(17,3));
	chainID = (l->mid(21,1)).at(0);
	if (chainID == ' ') chainID='1'; // when there's only one chain, this field is blank
	resSeq = (l->mid(22,4)).toLong(&convFlag);
	// iCode = ;
	//r.x = (l->mid(30,8)).toDouble(&convFlag);
	//r.y = (l->mid(38,8)).toDouble(&convFlag);
	//r.z = (l->mid(46,8)).toDouble(&convFlag);
	occupancy = (l->mid(54,6)).toDouble(&convFlag);

}

bool PDBAtom::isATM()
{
	return (kind == ISATOM);
}

//
//
//

PDBModel::PDBModel()
{
	modelID=1;
}

PDBModel::~PDBModel()
{
}
		
void PDBModel::read(QTextStream *ts, QString *l){
	// have ATOM and HETATM records in any order
	// NB: atoms and HETATMS have the same fields

	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	while ((l->indexOf("MODEL") != 0) && (l->indexOf("ENDMDL") != 0) &&
		(l->indexOf("CONECT") != 0) && (l->indexOf("MASTER") != 0)){
		if (l->indexOf("ATOM") == 0)
		{	
			PDBAtom *t = new PDBAtom(l,ISATOM);
			atoms.append(t);
		}
		else if (l->indexOf("HETATM")== 0)
		{
			PDBAtom *t = new PDBAtom(l,ISHETATOM);
			atoms.append(t);
		}
		else if (l->indexOf("SIGATM")==0){
		}
		else if (l->indexOf("ANISOU")==0){
		}
		else if (l->indexOf("SIGUIJ")==0){
		}
		else if (l->indexOf("TER")==0){
		}
		*l=ts->readLine();	
	}
	
}

QList<PDBAtom *> PDBModel::getAtomsByChain(QChar &chainID)
{
	QList<PDBAtom *> ret;
	
	for (int i=0;i<atoms.size();i++ ){
		PDBAtom *atom = atoms.at(i);
		if (atom->chainID == chainID && atom->isATM()){
			ret.append(atom);
		}
	}
	
	return ret;
}

//
//
//

PDBStructure::PDBStructure(){
	nModels=0;
}

PDBStructure::~PDBStructure(){
	qDebug() << trace.header(__PRETTY_FUNCTION__);
}

bool PDBStructure::read(QString fname,QStringList &mask){

	// The mask specifies which atoms/molecules are to be read in
	// If mask == NULL then all atoms/molecules are read in
	
	qDebug() << trace.header(__PRETTY_FUNCTION__);
	
	QString aline;
	QTextStream *ts;
	QString bigBuffer("");
	
	// If file is not compressed then ...
	QFileInfo finfo(fname);
	QFile f(fname);
	QString ext = finfo.suffix();
	ext = ext.toLower();
	if ( ext != "gz"){
		f.open(QIODevice::ReadOnly);
		ts = new QTextStream(&f);
	}  
	else if (ext == "gz") // uses zlib API
	{
		gzFile file;
		file = gzopen (fname.toUtf8().constData(), "rb");
		if(!file) {
			return false;
		}
		
		char buffer[1025];
		
		while(int readBytes =  gzread (file, buffer, 1024))
		{
			buffer[readBytes]=0; // null terminate !
			//printf("%s",buffer);
			bigBuffer.append(buffer);
		}
		gzclose(file); 
		ts = new QTextStream(&bigBuffer,QIODevice::ReadOnly); 
	}
 
	header.read(ts,&aline);
	remarks.read(ts,&aline);
	primStructure.read(ts,&aline);
	heterogen.read(ts,&aline);
	secStruct.read(ts,&aline);
	connectivity.read(ts,&aline);
	features.read(ts,&aline);	
	crystal.read(ts,&aline);
	coordTransform.read(ts,&aline);
	
	if (aline.indexOf("MODEL") != 0){
		// no MODEL token so only one model
		nModels =1;
		PDBModel *m = new PDBModel();
		m->read(ts,&aline);
		models.append(m);
	}
	else
	{
		while(aline.indexOf("MODEL") == 0){
			// read the MODEL	
			PDBModel *m = new PDBModel();
			bool convFlag;
			m->modelID=(aline.mid(10,4)).toInt(&convFlag);
			nModels++;
			aline=ts->readLine();
			m->read(ts,&aline);
			models.append(m);
			// consume the ENDMDL token and read the next line
			
			if (aline.indexOf("ENDMDL") ==0)
				aline=ts->readLine();
		}
	}

	f.close(); 
	delete ts;
	
	return true;
}

PDBModel *PDBStructure::getModel(int mid)
{

	
	for (int i=0;i< models.size(); i++ ){
		PDBModel *m = models.at(i);
		if (m->modelID==mid) return m;
	}
	return NULL;
}

QString PDBStructure::getChain(QChar &chainID)
{
	
	for (int i=0; i<primStructure.chains.size();i++ ){
		PDBChain *ch = primStructure.chains.at(i);
		if (ch->ID == chainID)
			return ch->residues;
	}
	return "";
}

QString	PDBStructure::getSequenceFromAtoms(int selModel,QChar selChain)
{
	PDBModel *mod = getModel(selModel);
	QList <PDBAtom *> chainAtoms = mod->getAtomsByChain(selChain);
		
	QString seq;
	long resSeq = 0;
	for (int i=0;i<chainAtoms.size();i++ ){
		PDBAtom *atom = chainAtoms.at(i);
		if (atom->resSeq != resSeq) {
			seq.append(atom->resName);
			resSeq=atom->resSeq;
		}
	}
	return seq;
}

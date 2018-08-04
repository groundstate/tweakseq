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

#ifndef __PDB_H_
#define __PDB_H_

#include <QList>
#include <QString>

//#include "vector3.h"

#define ISATOM 0
#define ISHETATOM 1

class QTextStream;

class PDBTitle{

	public:
		QString classification,depDate,IDcode; // HEADER field
		QString title;
		void read(QTextStream *, QString *);
};

class PDBRemarks{
	public:
		void read(QTextStream *,QString *);
};

class PDBChain{

	public:
		PDBChain(QChar,long );
		QChar ID;
		long nResidues;
		QString residues;
};

class PDBPrimStruct{

	public:
		long nChains;
		QList <PDBChain *> chains;
		void read(QTextStream *, QString *);
};

class  PDBHeterogen{
	public:
		void read(QTextStream *, QString *);
};

class PDBSecStruct{
	public:
		void read(QTextStream *, QString *);
};

class PDBConnectivity{
	public:
		void read(QTextStream *, QString *);
};

class PDBFeatures{
	public:
		void read(QTextStream *, QString *);
};

class PDBCrystal{
	public:
		void read(QTextStream *, QString *);
};

class PDBCoordTransform{
	public:
		void read(QTextStream *, QString *);
};


class PDBAtom
{
	public:
	
		long kind; // ATOM/HETATM - non-PDB field
		QString name;
		QChar altLoc;
		QChar resName; // 1 letter code
		QChar chainID;
		long resSeq;
		QChar iCode;
		//TVector3 r;
		double occupancy;
		double tempFactor;
		QString segID;
		QString element;
		QString charge;
	
		PDBAtom();
		PDBAtom(PDBAtom *);
		PDBAtom(QString *,long);
	
		bool isATM();
	
	// Eventually SIGUID, ANISOU and SIGUIJ should be here
	// since these pertain to each atom
};class TVector3;

// Slight departure here from the PBD docs grouping here ...
// No coordinates section but a list of models ...

class PDBModel
{
	public:
	
		PDBModel();
		~PDBModel();
		
		void read(QTextStream *,QString *);
		QList<PDBAtom *> getAtomsByChain(QChar &);
		
		int  modelID;
		long nChains;
		
		QList <PDBAtom *> atoms;
};


class PDBStructure{
	
	public:
	
		PDBTitle header;
		PDBRemarks remarks;
		PDBPrimStruct primStructure;
		PDBHeterogen heterogen;
		PDBSecStruct secStruct;
		PDBConnectivity connectivity;
		PDBFeatures features;	
		PDBCrystal crystal;
		PDBCoordTransform coordTransform;
		long nModels;
		QList <PDBModel *> models;
		
		PDBStructure();
		~PDBStructure();
		bool read(QString,QStringList &);
		
		PDBModel *getModel(int);
		QString getChain(QChar &);
		QString	getSequenceFromAtoms(int ,QChar);
		
	private:
		
};

#endif

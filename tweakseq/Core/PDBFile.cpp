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

#include <QFileInfo>

#include "PDB.h"
#include "PDBFile.h"

PDBFile::PDBFile(QString n):SequenceFile(n)
{
	QStringList ext;
	ext << "*.ent" << "*.ent.gz";
	setExtensions(ext,SequenceFile::Proteins);
}

PDBFile::~PDBFile()
{
}

bool PDBFile::isValidFormat(QString &fname)
{
	QFileInfo fi(fname);
	QString ext = "*."+fi.suffix();
	return (extensions(SequenceFile::Proteins).contains(ext,Qt::CaseInsensitive));
}

bool PDBFile::read(QStringList &seqnames,QStringList &seqs,QStringList &comments,Structure *s)
{
	// Structure s should be allocated externally
	
	PDBStructure pdbs;
	QStringList mask;
	pdbs.read(name(),mask);
	PDBPrimStruct ps = pdbs.primStructure;
	int nChains = pdbs.primStructure.nChains;
	qDebug() << trace.header(__PRETTY_FUNCTION__) << name() << "num chains = " << nChains;
	QFileInfo finfo(name());
	PDBChain *chain = pdbs.primStructure.chains.at(0);
	seqnames.append(finfo.baseName());
	seqs.append(chain->residues);
	comments.append(pdbs.header.classification + "\n"+pdbs.header.title);
	s->comment.append(pdbs.header.classification + "\n"+pdbs.header.title);
	s->source=name();
	for (int i=0;i<nChains;i++){
		PDBChain *chain = pdbs.primStructure.chains.at(i);
		s->chains.append(chain->residues);
		s->chainIDs.append(chain->ID);
	}
	return true;
}

bool PDBFile::write(QStringList &,QStringList &,QStringList &)
{
	// This does nothing
	return true;
}
		

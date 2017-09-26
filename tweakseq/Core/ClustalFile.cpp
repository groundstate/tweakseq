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

#include <QtDebug>
#include "DebuggingInfo.h"

#include <QFile>
#include <QStringList>

#include "ClustalFile.h"

// Base class for supported sequence alignment formats

//
//	Public members
//		

ClustalFile::ClustalFile(QString n):SequenceFile(n)
{
}

ClustalFile::~ClustalFile()
{
}

bool ClustalFile::read(QStringList &seqnames, QStringList &seqs,QStringList &comments)
{
	QString s;
	
	qDebug() << trace.header() << "ClustalFile::read() " << name();
	
	setError("");
	
	QFile f(name());
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() << trace.header() << "ClustalFile::read() couldn't open file";
		setError("Couldn't open file");
		return false;
	}
	QTextStream ts (&f);
	
	// Eat all lines until we get CLUSTAL (which should be the first line)
	while (!ts.atEnd()){
		s = ts.readLine();
		qDebug() << trace.header() << "line:" << s;
		if (s.contains("CLUSTAL")){
			// CLUSTALW is either "CLUSTALW" or "CLUSTAL W"
			// CLUSTAL O is
			// FIXME
			qDebug() << trace.header() << "ClustalFile::read() identified";
			break;
		}
	}
	
	if (ts.atEnd()){
		qDebug() << trace.header() << "ClustalFile::read() not CLUSTAL format";
		setError("Not CLUSTAL format");
		return false;
	}
	
	int nblk=0;
	int seqcnt=0;
	QRegExp ws = QRegExp("\\s+");

	while (!ts.atEnd()){
		s = ts.readLine().trimmed();
		if (s.isEmpty()){ // new block follows
			if (nblk==0)
				nblk=seqcnt;
			seqcnt=0;
			continue;
		}
		// There's something in the line
		// If the line contains '.','*',or ':' then it's the conservation information and it is skipped
		if (s.contains('.') || s.contains('*') || s.contains(':'))
			continue;
		// Must be a sequence
		// Consists of name, sequence and optionally some other stuff, all whitespace separated
		QStringList strl = s.split(ws);
		if (nblk==0){ // first
			seqnames.append(strl.at(0));
			seqs.append(strl.at(1));
		}
		else{
			seqs.replace(seqcnt,seqs.at(seqcnt)+strl.at(1));
		}
		//qDebug() << trace.header() << strl.length() << strl.at(0);
		
		seqcnt++;
	}
	
	// The sequences are read into a temporary QStringList
	for (int i=0;i<seqnames.size();i++){
		qDebug() << trace.header() << seqnames.at(i) << " " << seqs.at(i);
	}

	return true;
}

bool ClustalFile::write(QStringList &,QStringList &,QStringList &)
{
	return true;
}
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

#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QStringList>

#include "Application.h"
#include "ClustalFile.h"

extern Application *app;

// Base class for supported sequence alignment formats

//
//	Public members
//		

ClustalFile::ClustalFile(QString n):SequenceFile(n)
{
	QStringList ext;
	ext << "*.aln";
	setExtensions(ext,SequenceFile::Proteins);
	setExtensions(ext,SequenceFile::DNA);
}

ClustalFile::~ClustalFile()
{
}

bool ClustalFile::isValidFormat(QString & fname)
{
	// FIXME for the moment just use the extension to identify the file
	QFileInfo fi(fname);
	QString ext = "*."+fi.suffix();
	return (extensions(SequenceFile::Proteins).contains(ext,Qt::CaseInsensitive) ||
					extensions(SequenceFile::DNA).contains(ext,Qt::CaseInsensitive));
}

bool ClustalFile::read(QStringList &seqnames, QStringList &seqs,QStringList &comments,Structure *)
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
		qDebug() << trace.header(__PRETTY_FUNCTION__) << "not CLUSTAL format";
		setError("Not CLUSTAL format");
		return false;
	}
	
	int nblk=0;
	int seqcnt=0;
	QRegExp ws = QRegExp("\\s+");

	// FIXME no guess at data type
	
	//bool dataTypeIdentified=false;
	//int dataType=SequenceFile::DNA;
	//QRegExp regex("[EFIPQZ]");
	
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
			comments.append(strl.at(0)); // no comment - just use name
			seqs.append(strl.at(1));
		}
		else{
			seqs.replace(seqcnt,seqs.at(seqcnt)+strl.at(1));
		}
		
		// Try to identify the sequence data
		// Protein specific codes are E,F,I,P,Q,Z
		//if (!dataTypeIdentified){
		//	if (strl.at(1).contains(regex)){
		//		dataType=SequenceFile::Proteins;
		//		dataTypeIdentified=true;
		//		qDebug() << trace.header(__PRETTY_FUNCTION__) << "data type is PROTEIN";
		//	}
		//}
		//qDebug() << trace.header() << strl.length() << strl.at(0);
		
		seqcnt++;
	}
	
	// The sequences are read into a temporary QStringList
	for (int i=0;i<seqnames.size();i++){
		qDebug() << trace.header() << seqnames.at(i) << " " << seqs.at(i);
	}

	return true;
}

bool ClustalFile::write(QStringList &l,QStringList &s,QStringList &)
{
	
	setError("");
	
	QFile f(name());
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text)){
		qDebug() << trace.header() << "ClustalFile::write() couldn't open file";
		setError("Couldn't open file");
		return false;
	}
	
	// Prettify - find the longest label so that this field can be made a constant width
	int maxlablen =0;
	for (int li=0;li<l.size();li++){
		if (l.at(li).size() > maxlablen)
			maxlablen = l.at(li).size();
	}
	maxlablen += 3;
	
	// Determine the number of output blocks from the longest sequence
	int maxseqlen=0;
	for (int si=0;si<s.size();si++){
		if (s.at(si).size() > maxseqlen)
			maxseqlen = s.at(si).size();
	}
	
	int nblks = rint(maxseqlen/60);
	if (nblks*60 < maxseqlen) nblks++;
		
	QTextStream ts (&f);
	
	ts << "CLUSTALW created by tweakseq " << app->version() << endl;
	ts << endl;
	ts << endl;
	
	for (int b=1;b<=nblks;b++){
		QString conservationDegree = QString(60,'*');
		for (int si=0;si<s.size();si++){
			ts.setFieldAlignment(QTextStream::AlignLeft);
			ts << qSetFieldWidth(maxlablen) << l.at(si);
			ts.reset();
			ts << s.at(si).mid((b-1)*60,60) << endl;
		}
		QString p = QString(maxlablen,' ');
		ts << p << conservationDegree << endl;
		ts << endl;
	}
	
	f.close();
	
	return true;
}
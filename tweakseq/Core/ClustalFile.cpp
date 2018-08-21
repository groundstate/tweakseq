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
	
	QString cd = conservationDegree(s,maxseqlen);
	for (int b=1;b<=nblks;b++){
		
		for (int si=0;si<s.size();si++){
			ts.setFieldAlignment(QTextStream::AlignLeft);
			ts << qSetFieldWidth(maxlablen) << l.at(si);
			ts.reset();
			ts << s.at(si).mid((b-1)*60,60) << endl;
		}
		QString p = QString(maxlablen,' ');
		ts << p << cd.mid((b-1)*60,60) << endl;
		ts << endl;
	}
	
	f.close();
	
	return true;
}

QString ClustalFile::conservationDegree(QStringList &seqs,int maxLength)
{
	QString cd(maxLength,' ');
	// clustal-omega-1.2.4/src/squid/clustal.c
	int counts[11];
	for (int si=0;si<maxLength;si++){
		
		// Conserved residues
		QChar r;
		bool conserved=true;
		for (int sq=0;sq<seqs.size();sq++){
			if (si < seqs.at(sq).length()){
				if (r.isNull())
					r=seqs.at(sq).at(si); // initialize
				if (seqs.at(sq).at(si) != r){
					conserved=false;
					break;
				}
			}
		}
		if (conserved){
			cd.replace(si,1,'*');
			continue; // no more to do
		}
		
		// 'Strong conservation'
		for (int w=0;w<9;w++)
			counts[w]=0;
		
		for (int sq=0;sq<seqs.size();sq++){
			if (si < seqs.at(sq).length()){
				char r = seqs.at(sq).at(si).toLatin1(); 
				switch (r)
				{
					case 'S': counts[0]++; break;
					case 'T': counts[0]++; break;
					case 'A': counts[0]++; break;
					case 'N': counts[1]++; counts[2]++; counts[3]++;break;
					case 'E': counts[1]++; counts[3]++; break;
					case 'Q': counts[1]++; counts[2]++; counts[3]++; counts[4]++; break;
					case 'K': counts[1]++; counts[2]++; counts[4]++; break;
					case 'D': counts[3]++; break;
					case 'R': counts[4]++; break;
					case 'H': counts[2]++; counts[4]++; counts[7]++; break;
					case 'M': counts[5]++; counts[6]++; break;
					case 'I': counts[5]++; counts[6]++; break;
					case 'L': counts[5]++; counts[6]++; break;
					case 'V': counts[5]++; break;
					case 'F': counts[6]++; counts[8]++; break;
					case 'Y': counts[7]++; counts[8]++; break;
					case 'W': counts[8]++; break;
				}
			}
		} // for 
		conserved=false;
		for (int w=0;w<9;w++){
			if (counts[w]==seqs.size()){
				conserved = true;
				break;
			}
		}
		if (conserved){
			cd.replace(si,1,':');
			continue;
		}
		
		// 'Weak' conservation
		for (int w=0;w<11;w++)
			counts[w]=0;
		
		for (int sq=0;sq<seqs.size();sq++){
			if (si < seqs.at(sq).length()){
				char r = seqs.at(sq).at(si).toLatin1(); 
				switch (r){
					case 'C': counts[0]++; break;
					case 'S': counts[0]++; counts[2]++; counts[3]++; counts[4]++; counts[5]++; counts[6]++; break;
					case 'A': counts[0]++; counts[1]++; counts[2]++; counts[4]++; break;
					case 'T': counts[1]++; counts[3]++; counts[4]++; break;
					case 'V': counts[1]++; counts[9]++; break;
					case 'G': counts[2]++; counts[5]++; break; 
					case 'N': counts[3]++; counts[5]++; counts[6]++; counts[7]++; counts[8]++; break;
					case 'K': counts[3]++; counts[6]++; counts[7]++; counts[8]++; break;
					case 'D': counts[5]++; counts[6]++; counts[7]++; break;
					case 'E': counts[6]++; counts[7]++; counts[8]++; break;
					case 'Q': counts[6]++; counts[7]++; counts[8]++; break;
					case 'H': counts[7]++; counts[8]++; counts[10]++; break;
					case 'R': counts[8]++; break;
					case 'F': counts[9]++; counts[10]++; break;
					case 'L': counts[9]++; break;
					case 'I': counts[9]++; break;
					case 'M': counts[9]++; break;
					case 'Y': counts[10]++; break;
				}
			}
		} // for 
		conserved=false;
		for (int w=0;w<11;w++){
			if (counts[w]==seqs.size()){
				conserved = true;
				break;
			}
		}
		if (conserved){
			cd.replace(si,1,'.');
			continue;
		}
		
	}
	return cd;
}

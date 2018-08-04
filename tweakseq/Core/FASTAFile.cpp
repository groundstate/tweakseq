//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
/// Copyright (c) 2000-2017  Merridee A. Wouters, Michael J. Wouters
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
#include <QStringList>
#include <QFileInfo>
#include <QStringList>

#include "FASTAFile.h"

// Base class for supported sequence alignment formats

//
//	Public members
//		

FASTAFile::FASTAFile(QString n):SequenceFile(n)
{
	QStringList ext;
	ext << "*.faa" << "*.mpfa" << "*.fasta" << "*.fa" << "*.fas" << "*.seq" << "*.fsa";
	setExtensions(ext,SequenceFile::Proteins);
	QStringList ext2;
	ext2 << "*.fna" << "*.ffn"  << "*.frn" << "*.fasta" << "*.fa" << "*.fas" << "*.seq" << "*.fsa";
	setExtensions(ext2,SequenceFile::DNA);
}

FASTAFile::~FASTAFile()
{
}

bool FASTAFile::isValidFormat(QString & fname)
{
	QFileInfo fi(fname);
	QString ext = "*."+fi.suffix();
	return (extensions(SequenceFile::Proteins).contains(ext,Qt::CaseInsensitive) ||
					extensions(SequenceFile::DNA).contains(ext,Qt::CaseInsensitive));
}

#define SEEKING_COMMENT 0
#define READING_COMMENT 1
#define READING_SEQUENCE 2

bool FASTAFile::read(QStringList &seqnames, QStringList &seqs,QStringList &comments,Structure *)
{
	QString s;
	
	qDebug() << trace.header() << "FASTAFile::read() " << name();
	
	// Guess the sequence data type from the extension
	int seqData=SequenceFile::DNA;
	QFileInfo fi(name());
	QString suffix = fi.suffix().toLower();
	if ( suffix == "faa" || suffix == "mpfa" ){
		seqData=SequenceFile::Proteins;
	}
	else if (suffix == "fasta"){
		seqData=SequenceFile::Unknown;
	}
	
	setError("");
	
	QFile f(name());
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() << trace.header() << "FASTAFile::read() couldn't open file";
		setError("Couldn't open file");
		return false;
	}
	QTextStream ts (&f);
	
	int parseState=SEEKING_COMMENT;
	QString l,seq;
	int seqcnt=0;
	while (!ts.atEnd()){
		s = ts.readLine().trimmed();
		//qDebug() << trace.header() << "line:" << s;
		if (s.size() == 0) continue; // skip empty line
		char firstChar = s.at(0).toLatin1(); // guaranteed to be valid 
		switch (parseState)
		{
			case SEEKING_COMMENT:
				if (firstChar == ';' || firstChar == '>'){
					parseState = READING_COMMENT;
					comments.append(s);
					parseComment(s,l);
					seqnames.append(l);
					seqcnt++;
				}
				break;
			case READING_COMMENT:
				if (firstChar == ';') // FIXME skip other comments for the moment 
					continue;
				else{
					parseState = READING_SEQUENCE;
					seqs.append(s);
				}
				break;
			case READING_SEQUENCE:
				if (firstChar == ';' || firstChar == '>'){
					parseState = READING_COMMENT;
					comments.append(s);
					parseComment(s,l);
					seqnames.append(l);
					seqcnt++;
				}
				else{
					seqs.replace(seqcnt-1,seqs.at(seqcnt-1)+s);
				}
				break;
		}
		
	}

	qDebug() << trace.header() << "read " << seqcnt << " sequences";
	//for (int i=0;i<seqnames.size();i++){
	//	qDebug() << trace.header() << seqnames.at(i) << " " << seqs.at(i).size();
	//}
	setDataType(seqData);
	return true;
}

bool FASTAFile::write(QStringList &l,QStringList &s,QStringList &c)
{
	setError("");
	
	QFile f(name());
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text)){
		qDebug() << trace.header() << "FASTAFile::write() couldn't open file";
		setError("Couldn't open file");
		return false;
	}
	QTextStream ts (&f);
	
	for (int i=0;i<l.size();i++){
		ts << c.at(i) << endl;
		int nlines = rint(s.at(i).size()/80);
		if (nlines*80 < s.at(i).size()) nlines++;
		for (int j=0;j<nlines;j++){
			ts << s.at(i).mid(j*80,80) << endl; // ok, mid() will return last bit of the string
		}
	}
	f.close();
	return true;
}

//
// Private members
//

void FASTAFile::parseComment(QString &s,QString &l)
{
	//qDebug() << trace.header() << "FASTAFile::parseComment " << s;
	// Provisionally use the comment to the first space
	int index =s.indexOf(QChar(' '),1);
	if (index == -1)
		l = s.mid(1,-1); // use the lot
	else
		l = s.mid(1,index-1);
	//qDebug() << trace.header() << "FASTAFile::parseComment " << l;
}
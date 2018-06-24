//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2018 Michael J. Wouters, Merridee A. Wouters
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

#include "Consensus.h"
#include "Sequence.h"
#include "Sequences.h"
 
static int BLOSUM62[23][23] = 
{
	{  4,-1,-2,-2, 0,-1,-1, 0,-2,-1,-1,-1,-1,-2,-1, 1, 0,-3,-2, 0,-2,-1, 0}, //  0  A 
	{ -1, 5, 0,-2,-3, 1, 0,-2, 0,-3,-2, 2,-1,-3,-2,-1,-1,-3,-2,-3,-1, 0,-1}, //  1  R
	{ -2, 0, 6, 1,-3, 0, 0, 0, 1,-3,-3, 0,-2,-3,-2, 1, 0,-4,-2,-3, 3, 0,-1}, //  2  N 
	{ -2,-2, 1, 6,-3, 0, 2,-1,-1,-3,-4,-1,-3,-3,-1, 0,-1,-4,-3,-3, 4, 1,-1}, //  3  D 
	{  0,-3,-3,-3, 9,-3,-4,-3,-3,-1,-1,-3,-1,-2,-3,-1,-1,-2,-2,-1,-3,-3,-2}, //  4  C 
	{ -1, 1, 0, 0,-3, 5, 2,-2, 0,-3,-2, 1, 0,-3,-1, 0,-1,-2,-1,-2, 0, 3,-1}, //  5  Q 
	{ -1, 0, 0, 2,-4, 2, 5,-2, 0,-3,-3, 1,-2,-3,-1, 0,-1,-3,-2,-2, 1, 4,-1}, //  6  E 
	{  0,-2, 0,-1,-3,-2,-2, 6,-2,-4,-4,-2,-3,-3,-2, 0,-2,-2,-3,-3,-1,-2,-1}, //  7  G 
	{ -2, 0, 1,-1,-3, 0, 0,-2, 8,-3,-3,-1,-2,-1,-2,-1,-2,-2, 2,-3, 0, 0,-1}, //  8  H 
	{ -1,-3,-3,-3,-1,-3,-3,-4,-3, 4, 2,-3, 1, 0,-3,-2,-1,-3,-1, 3,-3,-3,-1}, //  9  I 
	{ -1,-2,-3,-4,-1,-2,-3,-4,-3, 2, 4,-2, 2, 0,-3,-2,-1,-2,-1, 1,-4,-3,-1}, // 10  L 
	{ -1, 2, 0,-1,-3, 1, 1,-2,-1,-3,-2, 5,-1,-3,-1, 0,-1,-3,-2,-2, 0, 1,-1}, // 11  K 
	{ -1,-1,-2,-3,-1, 0,-2,-3,-2, 1, 2,-1, 5, 0,-2,-1,-1,-1,-1, 1,-3,-1,-1}, // 12  M 
	{ -2,-3,-3,-3,-2,-3,-3,-3,-1, 0, 0,-3, 0, 6,-4,-2,-2, 1, 3,-1,-3,-3,-1}, // 13  F 
	{ -1,-2,-2,-1,-3,-1,-1,-2,-2,-3,-3,-1,-2,-4, 7,-1,-1,-4,-3,-2,-2,-1,-2}, // 14  P 
	{  1,-1, 1, 0,-1, 0, 0, 0,-1,-2,-2, 0,-1,-2,-1, 4, 1,-3,-2,-2, 0, 0, 0}, // 15  S 
	{  0,-1, 0,-1,-1,-1,-1,-2,-2,-1,-1,-1,-1,-2,-1, 1, 5,-2,-2, 0,-1,-1, 0}, // 16  T 
	{ -3,-3,-4,-4,-2,-2,-3,-2,-2,-3,-2,-3,-1 ,1,-4,-3,-2,11, 2,-3,-4,-3,-2}, // 17  W 
	{ -2,-2,-2,-3,-2,-1,-2,-3, 2,-1,-1,-2,-1, 3,-3,-2,-2, 2, 7,-1,-3,-2,-1}, // 18  Y 
	{  0,-3,-3,-3,-1,-2,-2,-3,-3, 3, 1,-2, 1,-1,-2,-2, 0,-3,-1, 4,-3,-2,-1}, // 19  V 
	{ -2,-1, 3, 4,-3, 0, 1,-1, 0,-3,-4, 0,-3,-3,-2, 0,-1,-4,-3,-3, 4, 1,-1}, // 20  B 
	{ -1, 0, 0, 1,-3, 3, 4,-2, 0,-3,-3, 1,-1,-3,-1, 0,-1,-3,-2,-2, 1, 4,-1}, // 21  Z 
	{  0,-1,-1,-1,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2, 0, 0,-2,-1,-1,-1,-1,-1}  // 22  X 
};

static int BLOSUM62map[26]=
{
	0,  20,   4,   3,   6,
 13,   7,   8,   9,  22,
 11,  10,  12,   2,  22,
 14,   5,   1,  15,  16,
 22,  19,  17,  22,  18,
 21
};

Consensus::Consensus(Sequences *s)
{
	sequences_=s;
	// calculate the default plurality
	QList<Sequence *> &ss = sequences_->sequences();
	plurality_=0.0;
	for (int s=0;s<ss.size();s++){
		double weight=1.0; // FIXME
		plurality_ += weight;
	}
	plurality_ /= 2.0;
}

Consensus::~Consensus()
{
}

void Consensus::calculate()
{
	// Assuming here that aligned sequences are all the same length
	QList<Sequence *> &ss = sequences_->sequences();
	int slen = ss.at(0)->residues.length();
	int nseq = ss.size();
	double *scores = new double[nseq];
	double matches;
	
	// Convert the residues to indices into the scoring matrix 
	char **rindex = new char*[nseq]; // * 8 bits is enough ...
	for (int i=0;i<nseq;i++)
		rindex[i] = new char[slen];
	
	for (int s=0;s<nseq;s++){
		Sequence *seq = ss.at(s);
		for (int c=0;c<slen;c++){
			QChar pChar = (seq->residues[c].unicode() & 0xff);
			int idx = pChar.toLatin1()-65;
			if (idx < 0 || idx > 25)
				rindex[s][c]=99;
			else
				rindex[s][c]=BLOSUM62map[idx];
		}
	}
	
	for (int c=0;c<slen;c++){ // for each column in the alignment
		double hiScore=0.0;
		int riHiScore=0;
		double riMatches=0; //weighted, so need double
		for (int ri=0;ri<nseq;ri++){ // for each residue in the column
			scores[ri]=0.0;
			matches=0.0;
			for (int rj=0;rj<nseq;rj++){
				if (ri==rj) continue;
				double weight = 1.0;
				int resi=rindex[ri][c];
				int resj=rindex[rj][c];
				if (resi == 99 && resj == 99){
					scores[ri]+= weight;
					if (weight > 0)
						matches += weight;
				}
				else if (resi == 99 || resj == 99)
					scores[ri]+=-4*weight;
				else{
					double tmp = BLOSUM62[resi][resj]*weight;
					scores[ri]+= tmp;
					if (tmp > 0)
						matches += weight;
				}
						
			}
			if (ri==0){
				hiScore = scores[ri];
				riHiScore=ri;
				riMatches=matches;
			}
			else if (scores[ri] > hiScore){
				hiScore = scores[ri];
				riHiScore=ri;
				riMatches=matches;
			}
		}
		//qDebug() << c << " " << riHiScore << " " << hiScore << " " << riMatches << 
		//	QChar(ss.at(riHiScore)->residues[c].unicode() & 0xff);
		if (riMatches >= plurality_)
			consensusSequence_[c]=QChar(ss.at(riHiScore)->residues[c].unicode() & 0xff);
		else
			consensusSequence_[c]=QChar('!');
	}
	
	delete[] scores;
	
	for (int i=0;i<nseq;i++)
		delete[] rindex[i];
	delete[] rindex;
	//qDebug() << consensusSequence_;
}

void Consensus::setSequences(Sequences *s)
{
	sequences_=s;
}


QString Consensus::scoringMatrixName()
{
	return "BLOSUM62";
}

void Consensus::setPlurality(double p)
{
	plurality_=p;
}

QString Consensus::sequence()
{
	return "";
}


//
//

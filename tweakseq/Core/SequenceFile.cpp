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


#include "SequenceFile.h"

// Base class for supported sequence alignment formats

//
//	Public members
//		

SequenceFile::SequenceFile(QString n)
{
	n_=n;
	formatName_="unknown";
	formatVersion_="unknown";
	err_="";
	dataType_=SequenceFile::Unknown;
}

SequenceFile::~SequenceFile()
{
}

QStringList & SequenceFile::extensions(int dataFilter)
{
	if (dataFilter == SequenceFile::Proteins)
		return proteinExtensions_;
	else 
		return DNAextensions_;
}

//
//

void SequenceFile::setExtensions(QStringList &e,int dataFilter)
{
	if (dataFilter == SequenceFile::Proteins)
		proteinExtensions_=e;
	else
		DNAextensions_ =e;
}

bool SequenceFile::read(QStringList &,QStringList &,QStringList &,Structure *)
{
	err_="";
	return true;
}

bool SequenceFile::write(QStringList &,QStringList &,QStringList &)
{
	err_="";
	return true;
}
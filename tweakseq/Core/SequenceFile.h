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


#ifndef __SEQUENCE_FILE_H_
#define __SEQUENCE_FILE_H_

#include <QString>
#include <QStringList>

#include "Structure.h"

// Base class for supported sequence alignment formats
class SequenceFile
{
	public:
		
		SequenceFile(QString n= QString());
		~SequenceFile();
		
		enum DataType {Proteins, DNA, PDB,Unknown};
		
		virtual bool isValidFormat(QString &){return true;}
		
		QString name(){return n_;};
		void setName(QString n){n_=n;}
		
		QString formatName(){return formatName_;}
		QString formatVersion(){return formatVersion_;}
		QString error(){return err_;}
		
		int dataType(){return dataType_;}
		
		QStringList & extensions(int dataFilter);
		
		virtual bool read(QStringList &,QStringList &,QStringList &,Structure *s= NULL);
		virtual bool write(QStringList &,QStringList &,QStringList &);
	
	protected:
		
		void setFormatName(QString n){formatName_=n;}
		void setFormatVersion(QString n){formatVersion_=n;}
		void setError(QString e){err_=e;}
		void setExtensions(QStringList &e,int dataFilter);
		void setDataType(int dType){dataType_=dType;}
		
	private:
		
		QString n_;
		
		QString formatName_;
		QString formatVersion_;
		
		QStringList proteinExtensions_;
		QStringList DNAextensions_;
		
		QString err_;
		
		int dataType_;
};

#endif

//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017  Michael J. Wouters, Merridee A. Wouters
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

#ifndef __CLUSTALO_H_
#define __CLUSTALO_H_

#include "AlignmentTool.h"

class BoolProperty;
class FileProperty;
class IntProperty;
class PropertiesDialog;

class ClustalO: public AlignmentTool
{
	public:
		
		ClustalO();
		~ClustalO();
		
		virtual void makeCommand(QString &, QString &, QString &, QStringList &);
		virtual void writeSettings(QDomDocument &,QDomElement &);
		virtual void readSettings(QDomDocument &);
		
		virtual PropertiesDialog * propertiesDialog(QWidget *);
		
	private:
	
		void init();
		void getVersion();
		
		FileProperty *execPath_;
		// Sequence input
		BoolProperty *dealign_;
		// Clustering
		FileProperty *distMatrixIn_,*distMatrixOut_,*guideTreeIn_,*guideTreeOut_,*clusteringOut_,*posteriorOut_;
		BoolProperty *pileup_,*full_,*full_iter_,*useKimura_,*percentID_;
		IntProperty  *clusterSize_,*transitivity_;
		
};

#endif
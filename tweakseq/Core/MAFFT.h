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

#ifndef __MAFFT_H_
#define __MAFFT_H_

#include "AlignmentTool.h"

class QAction;
class QMenu;

class BoolProperty;
class DoubleProperty;
class IntProperty;
class PropertiesDialog;

class MAFFT: public AlignmentTool
{
	Q_OBJECT
	
	public:
		
		MAFFT();
		~MAFFT();
		
		virtual void makeCommand(QString &, QString &, QString &, QStringList &);
		virtual void makeDefaultCommand(QString &, QString &, QString &, QStringList &);
		virtual void writeSettings(QDomDocument &,QDomElement &);
		virtual void readSettings(QDomDocument &);
		
		virtual PropertiesDialog * propertiesDialog(QWidget *);
		virtual QMenu *createCustomMenu();
	
		enum Strategies {Auto,L_INSI_i,G_INSI_i,E_INSI_i,
										 FFT_NS_i_2,DEFAULT_FFT_NS_2,FFT_NS_1,
										 NW_NS_i,NW_NS_2,NW_NS_PartTree_1,
										 Custom};
	private slots:
		
		void setStrategy(QAction *);
		
	private:
	
		void init();
		void getVersion();
		void setPropertiesFromStrategy(int);
		
		QMenu *customMenu_;
		QList<QAction *> actions_;
		
		int idealThreadCount_;
		int strategy_;
		
		IntProperty *numThreads_;
		
		BoolProperty *localPair_,*globalPair_;
		IntProperty  *maxIterate_;
		DoubleProperty  *ep_;
		BoolProperty *genafpair_;
		IntProperty  *retree_;
		BoolProperty *nofft_;
		BoolProperty *parttree_;
		BoolProperty *auto_;
		
		QList<Property *> strategyProperties_;
		
};

#endif
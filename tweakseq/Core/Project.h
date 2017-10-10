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


#ifndef __PROJECT_H_
#define __PROJECT_H_

#include <QDir>
#include <QObject>
#include <QString>

class SequenceSelection;
class SeqEditMainWin;

class Project:public QObject
{
	Q_OBJECT
	
	public:
		
		Project();
		Project(QString &);
		
		~Project();
		
		SequenceSelection * sequenceSelection;
		
		QString name(){return name_;}
		void setName(QString &);
		
	public slots:
	
		void newProject();
		void openProject();
		void save();
		void read(QString &);
	
		void closeIt();
		
		void createMainWindow();
		void mainWindowClosed();
		
	private:
		
		void init();
		
		// Widgets we keep track of
		SeqEditMainWin *mainWindow_;
		
		QString fname_;
		QString name_;
		QDir path_;
};

#endif
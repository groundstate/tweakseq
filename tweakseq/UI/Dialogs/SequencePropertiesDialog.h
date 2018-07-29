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

#ifndef __SEQUENCE_PROPERTIES_DLG_H_
#define __SEQUENCE_PROPERTIES_DLG_H_

#include <QDialog>

class QDialogButtonBox;
class QLineEdit;

class Project;
class Sequence;

class SequencePropertiesDialog:public QDialog
{
	Q_OBJECT
	
	public:
		
		SequencePropertiesDialog(Project *, Sequence *,QWidget* parent = 0, Qt::WindowFlags f = 0 );
		~SequencePropertiesDialog();
	
		QString structureFile();
		
	public slots:
		
		virtual void accept();
		
	private slots:
	
		void browseStructures();
		
	private:
		
		QLineEdit *nameEditor_,*structureEditor_;
		QDialogButtonBox *buttonBox_;
		
		Sequence *seq_;
		Project  *project_;
		QString name_;
};

#endif

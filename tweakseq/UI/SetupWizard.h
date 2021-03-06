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

#ifndef __SETUP_WIZARD_H_
#define __SETUP_WIZARD_H_

#include <QWizard> 

class QCheckBox;
class QComboBox;
class QLineEdit;

class SetupWizard: public QWizard
{
	
	Q_OBJECT
	
	public:
	
		SetupWizard(QWidget *parent =0,Qt::WindowFlags flags = 0);
		~SetupWizard();
		
		void addMessage(QString &);
		void clustalOConfig(bool &,QString &);
		void muscleConfig(bool &,QString &);
		void mafftConfig(bool &,QString &);
		QString preferredTool();
		
		virtual bool validateCurrentPage();
		
	private slots:
		
		  void browseClustalO();
			void browseMUSCLE();
			void browseMAFFT();
	private:
		
		QWizardPage * createIntroPage();
		QWizardPage * createAlignmentToolPage();
		
		QCheckBox *clustaloCB_,*muscleCB_,*mafftCB_;
		QLineEdit *clustaloLE_,*muscleLE_,*mafftLE_;
		
		QComboBox *preferredTool_;
		
		QString msg_;
		
		int idAlignmentTool_;
		
};

#endif
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

#ifndef _PROPERTIES_DIALOG_H_
#define _PROPERTIES_DIALOG_H_

#include <QDialog>
#include <QStack>

class QTabWidget;
class QVBoxLayout;
class QColor;
class QGridLayout;
class QBoxLayout;

//class ColourInput ;
class DoubleProperty;
class DoublePropertyInput;
class EnumIntPropertyInput;
//class DateTimeSaveInput ;
class FileProperty;
class FilePropertyInput ;
class FloatProperty;
class FloatRangePropertyInput;
class FloatPropertyInput ;
class IntProperty;
class IntPropertyInput ;
class BoolProperty;
class OptionPropertyInput ;
class StringProperty;
class StringPropertyInput ;

class Propertied;

class PropertiesDialog: public QDialog
{
	Q_OBJECT
	
	public:
		
		enum ButtonClick{OKClicked,ApplyClicked,CancelClicked};
		
		PropertiesDialog(QWidget*,Propertied *,QString);
		~PropertiesDialog();
		
		void addTab(QString);

		void beginGridLayout();
		void endGridLayout();

		QWidget *currContainerWidget();
		
		void addText(QString);
		void addPixmap(QString,QString);
		void addPixmap(QString,const char **);

		//ColourInput * addColorPicker(const char *,float *,float *,float *);
		//ColourInput * addColorPicker(const char *,QColor *);
		
		OptionPropertyInput * addOptionInput(QString,BoolProperty *);
		FloatPropertyInput * addFloatInput(QString,FloatProperty *,bool showSlider=false);
		DoublePropertyInput * addDoubleInput(QString,DoubleProperty *,bool showSlider=false);
		IntPropertyInput * addIntInput(QString,IntProperty *,bool showSlider=false);
		FloatRangePropertyInput * addFloatRangeInput(QString,FloatProperty *,
			QString,FloatProperty *);
		EnumIntPropertyInput* addEnumIntInput(QString,IntProperty *);
		EnumIntPropertyInput* addEnumIntInput(QString,IntProperty *,QStringList &,int,int);

		//DateTimeSaveInput * addDateTimeInput(const char *,int *,int *,int *,int *,int *,int *);
		StringPropertyInput * addStringInput(QString,StringProperty *);
		FilePropertyInput * addFileInput(QString,FileProperty *,int,const char *,const char *);

		void addCustomWidget(QWidget *);
		//void addTickedList(const char *,const char **,int);
		
		void setActiveTab(QString);
		void addPadding();
		void addSeparator(QString label= QString());
		
	signals:
		
		void apply(int); 
		void createdDialog(PropertiesDialog *);

	protected:
		
		void setChildren();
		void restoreChildren();

	protected slots:
	
		virtual void help();
		virtual void ok();
		virtual void applyB();
		virtual void cancel();
		
	private:
	
		QTabWidget *tw_;
	
		//ComboBoxInput *currComboBoxInput_;
		QStack<QWidget *> containerWidgets_;
		QStack<QBoxLayout *> layouts_;

		QGridLayout *gridLayout_;

		Propertied *propertied_;

};

#endif

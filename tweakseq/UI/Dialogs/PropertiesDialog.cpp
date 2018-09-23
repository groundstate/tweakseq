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

#include <cmath>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QPixmap>
#include <QPushButton>
#include <QTabWidget>

//#include "Channel.h"
//#include "ColourInput.h"
//#include "ComboBoxInput.h"
//#include "DateTimeSaveInput.h"

#include "EnumIntPropertyInput.h"
#include "DoubleProperty.h"
#include "DoublePropertyInput.h"
#include "IntProperty.h"
#include "IntPropertyInput.h"
#include "FileProperty.h"
#include "FilePropertyInput.h"
#include "FloatProperty.h"
#include "FloatPropertyInput.h"
#include "FloatRangePropertyInput.h"
#include "BoolProperty.h"
#include "OptionPropertyInput.h"
#include "Propertied.h"
#include "PropertiesDialog.h"
#include "StringProperty.h"
#include "StringPropertyInput.h"

static QPixmap *dppm=0;

PropertiesDialog::PropertiesDialog(QWidget *parent,
	Propertied *propertied,
	QString caption):QDialog(parent)
{
	
	propertied_=propertied;

	setWindowTitle(caption);
	
	QVBoxLayout *topvb = new QVBoxLayout(this);
	//topvb->setSpacing(8);
	tw_=new QTabWidget();
	topvb->addWidget(tw_);
	
	//pushb = new QPushButton("Help",this);
	//hb->addWidget(pushb);
	//connect(pushb,SIGNAL(clicked()),this,SLOT(help()));
	
	buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	topvb->addWidget(buttonBox_);
	
	connect(buttonBox_, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox_, SIGNAL(rejected()), this, SLOT(reject()));
	
	//currComboBoxInput_=NULL; FIXME
	containerWidgets_.push(NULL);
	layouts_.push(NULL);
	gridLayout_=NULL;

	if (!(dppm))
		dppm = new QPixmap(":/images/dotpoint.xpm");
	
}

PropertiesDialog::~PropertiesDialog()
{
	propertied_->lock(false);

	//qDebug() << trace.header() << "PropertiesDialog::~PropertiesDialog()";
}


QWidget *PropertiesDialog::currContainerWidget()
{
	return containerWidgets_.top();
}
		
void PropertiesDialog::addTab(QString label)
{
	// FIXME maybe the scroll area needs to be named
	QScrollArea *sv= new QScrollArea(this);
	// If you don't do this then the 
	sv->setWidgetResizable(true);
	tw_->addTab(sv,label);
	QWidget *w = new QWidget(sv);
	w->setObjectName(label); // required so by setActiveTab()
	sv->setWidget(w);
	// add some stretch to the last layout
	if (layouts_.top()) layouts_.top()->addStretch(1);
	QVBoxLayout *vb =new QVBoxLayout(w);
	vb->setContentsMargins(8,8,8,8);
	vb->setSpacing(2);
	if (layouts_.top())
		layouts_.pop();
	layouts_.push(vb);
	if (containerWidgets_.top()) // keep the NULL 
		containerWidgets_.pop();
	containerWidgets_.push(w);
	w->show();
	
}

void PropertiesDialog::beginGridLayout()
{
}

void PropertiesDialog::endGridLayout()
{
}


void PropertiesDialog::addText(QString txt)
{
	if (containerWidgets_.top())
	{
		QVBoxLayout *vb = new QVBoxLayout();
		vb->setContentsMargins(0,0,0,0);
		layouts_.top()->addLayout(vb);

		QLabel *l = new QLabel(containerWidgets_.top());
		l->setText(txt);
		vb->addWidget(l,1);
	}
}

void PropertiesDialog::addPixmap(QString label, QString fileName)
{
	if (containerWidgets_.top())
	{
		QVBoxLayout *vb = new QVBoxLayout();
		vb->setContentsMargins(0,0,0,0);
		layouts_.top()->addLayout(vb);

		QLabel *l = new QLabel(containerWidgets_.top());
		l->setPixmap(QPixmap(fileName));
		vb->addWidget(l,1);

		l = new QLabel(label,containerWidgets_.top());
		vb->addWidget(l,0);
	}
}

void PropertiesDialog::addPixmap(QString label,const char **pixmapName)
{
	if (containerWidgets_.top())
	{
		QVBoxLayout *vb = new QVBoxLayout();
		vb->setContentsMargins(0,0,0,0);
		layouts_.top()->addLayout(vb);

		//qDebug() << trace.header() << "About to die ..";
		QLabel *l = new QLabel(containerWidgets_.top());
		l->setPixmap(QPixmap(pixmapName));
		vb->addWidget(l,1);
		//qDebug() << trace.header() << "No I didn't ..";

		l = new QLabel(label,containerWidgets_.top());
		vb->addWidget(l,0);
	}
}

// ColourInput* PropertiesDialog::addColorPicker(const char *label ,
// 	float *r,float *g,float *b)
// {
// 	if (containerWidgets_.top())
// 	{
// 		QHBoxLayout *hb = new QHBoxLayout();
// 		hb->setContentsMargins(0,0,0,0);
// 		layouts_.top()->addLayout(hb);
// 		
// 		QLabel *l = new QLabel(label,containerWidgets_.top());
// 		hb->addWidget(l,1);
// 		
// 		ColourInput *ci = new ColourInput(containerWidgets_.top(),r,g,b,label);
// 		hb->addWidget(ci,1);
// 		return ci;
// 	}
// 	else
// 	{
// 		cerr << "PropertiesDialog::addColorPicker() No active tab yet ! "
// 				 << endl;
// 	}
// 	return NULL;
// }

// ColourInput* PropertiesDialog::addColorPicker(const char *label ,QColor *c)
// {
// 	if (containerWidgets_.top())
// 	{
// 		QHBoxLayout *hb = new QHBoxLayout();
// 		layouts_.top()->addLayout(hb);
// 		
// 		QLabel *l = new QLabel(label,containerWidgets_.top());
// 		hb->addWidget(l,1);
// 		
// 		ColourInput *ci = new ColourInput(containerWidgets_.top(),c,label);
// 		hb->addWidget(ci,1);
// 		return ci;
// 	}
// 	else
// 	{
// 		cerr << "PropertiesDialog::addColorPicker() No active tab yet ! "
// 				 << endl;
// 	}
// 	return NULL;
// }


OptionPropertyInput* PropertiesDialog::addOptionInput(QString label,BoolProperty *val)
{
	if (containerWidgets_.top())
	{
	
		QHBoxLayout *hb = new QHBoxLayout();
		layouts_.top()->addLayout(hb);
		hb->setContentsMargins(0,0,0,0);

		OptionPropertyInput *oi= new OptionPropertyInput(containerWidgets_.top(),val,label);
		hb->addWidget(oi,0);
		
		QLabel *l = new QLabel(label,containerWidgets_.top());
		hb->addWidget(l,1);
		
		return oi;
	}
	else
	{
		//qWarning() <<  warning.header() << "PropertiesDialog::addOption() No active tab yet ! ";
		return NULL;
	}
	return NULL;
}

FloatPropertyInput * PropertiesDialog::addFloatInput(
	QString label,FloatProperty *val,bool showSlider)
{
	if (containerWidgets_.top())
	{
		QHBoxLayout *hb = new QHBoxLayout();
		layouts_.top()->addLayout(hb);
		hb->setContentsMargins(0,0,0,0);

		QLabel *l = new QLabel(label,containerWidgets_.top());
		hb->addWidget(l,1);
		
		FloatPropertyInput *fi = new FloatPropertyInput(containerWidgets_.top(),val,label,showSlider);
		hb->addWidget(fi,1);
		
		return fi;
	}
	else
	{
		//qWarning() <<  warning.header() << "PropertiesDialog::addFloatInput No active tab yet ! ";
		return NULL;
	}
	return NULL;
}

DoublePropertyInput * PropertiesDialog::addDoubleInput(
	QString label,DoubleProperty *val,bool showSlider)
{
	if (containerWidgets_.top())
	{
		QHBoxLayout *hb = new QHBoxLayout();
		layouts_.top()->addLayout(hb);
		hb->setContentsMargins(0,0,0,0);

		QLabel *l = new QLabel(label,containerWidgets_.top());
		hb->addWidget(l,1);
		
		DoublePropertyInput *fi = new DoublePropertyInput(containerWidgets_.top(),val,label,showSlider);
		hb->addWidget(fi,1);
		
		return fi;
	}
	else
	{
		//qWarning() <<  warning.header() << "PropertiesDialog::addFloatInput No active tab yet ! ";
		return NULL;
	}
	return NULL;
}

IntPropertyInput * PropertiesDialog::addIntInput(
	QString label,IntProperty *val,bool showSlider)
{
	if (containerWidgets_.top())
	{
		QHBoxLayout *hb = new QHBoxLayout();
		layouts_.top()->addLayout(hb);
		hb->setContentsMargins(0,0,0,0);

		QLabel *l = new QLabel(label,containerWidgets_.top());
		hb->addWidget(l,1);
		
		IntPropertyInput *ii = new IntPropertyInput(containerWidgets_.top(),val,label,showSlider);
		hb->addWidget(ii,1);
		
		return ii;
	}
	else
	{
		//qWarning() <<  warning.header() << "PropertiesDialog::addIntInput No active tab yet ! ";
		return NULL;
	}
	return NULL;
}

// SimpleChannelEditor * PropertiesDialog::addChannelInput(const char *,Channel &c)
// {
// 	if (containerWidgets_.top())
// 	{
// 		SimpleChannelEditor *ce =
// 		 new SimpleChannelEditor(containerWidgets_.top(),c,c.name().c_str());
// 		connect(ce,SIGNAL(createdDialog(PropertiesDialog *)),this,SIGNAL(createdDialog(PropertiesDialog *)));
// 		layouts_.top()->addWidget(ce);
// 		return ce;
// 	}
// 	return NULL;
// }

FloatRangePropertyInput * PropertiesDialog::addFloatRangeInput(
	QString label1,FloatProperty *val1,
	QString label2,FloatProperty *val2)
{
	if (containerWidgets_.top())
	{
		FloatRangePropertyInput *fr= new FloatRangePropertyInput(containerWidgets_.top(),
			val1,label1,val2,label2);
		layouts_.top()->addWidget(fr);
		return fr;
	}
	return NULL;
}

EnumIntPropertyInput* PropertiesDialog::addEnumIntInput(QString label,IntProperty *val)
{
	if (containerWidgets_.top())
	{
		QHBoxLayout *hb = new QHBoxLayout();
		layouts_.top()->addLayout(hb);
		hb->setContentsMargins(0,0,0,0);

		QLabel *l = new QLabel(label,containerWidgets_.top());
		hb->addWidget(l,1);
		
		EnumIntPropertyInput *ei = new EnumIntPropertyInput(containerWidgets_.top(),val,label);
		hb->addWidget(ei,1);
		
		//currComboBoxInput_=cb;
		return ei;
	}
	return NULL;
}

EnumIntPropertyInput* PropertiesDialog::addEnumIntInput(QString label,IntProperty *val,
	QStringList &icons,int w,int h)
{
	if (containerWidgets_.top())
	{
		QHBoxLayout *hb = new QHBoxLayout();
		layouts_.top()->addLayout(hb);
		hb->setContentsMargins(0,0,0,0);

		QLabel *l = new QLabel(label,containerWidgets_.top());
		hb->addWidget(l,1);
		
		EnumIntPropertyInput *ei = new EnumIntPropertyInput(containerWidgets_.top(),val,label,icons,w,h);
		hb->addWidget(ei,1);
		
		//currComboBoxInput_=cb;
		return ei;
	}
	return NULL;
}

// ComboBoxInput* PropertiesDialog::addComboBox(const char *label,
// 		const char **items,int *currItem)
// {
// 	if (containerWidgets_.top())
// 	{
// 		QHBoxLayout *hb = new QHBoxLayout();
// 		layouts_.top()->addLayout(hb);
// 		hb->setContentsMargins(0,0,0,0);
// 
// 		QLabel *l = new QLabel(label,containerWidgets_.top());
// 		hb->addWidget(l,1);
// 		
// 		ComboBoxInput *cb = new ComboBoxInput(containerWidgets_.top(),items,currItem,label);
// 		hb->addWidget(cb,1);
// 		
// 		currComboBoxInput_=cb;
// 		return cb;
// 	}
// 	return NULL;
// }

		
// DateTimeSaveInput * PropertiesDialog::addDateTimeInput(const char * label,
// 	int *year,int *month,int *day,int *hour,int *min,int *sec)
// {
// 	if (containerWidgets_.top())
// 	{
// 		DateTimeSaveInput *dtsi=new DateTimeSaveInput(
// 			containerWidgets_.top(),label,year,month,day,hour,min,sec);
// 		layouts_.top()->addWidget(dtsi);
// 		return dtsi;
// 	}
// 	return NULL;
// }
	
// void PropertiesDialog::addTickedList(const char *,const char **,int)
// {
// 	if (containerWidgets_.top())
// 	{
// 		//TickedListBox *lb = new TickedListBox(currWidget_,items,
// 	}
// }

// ShaderGroupInput * PropertiesDialog::addShaderGroupInput(const char *label,
// 			Project *project,NodeSP &node,vector<Shader *> &shaders)
// {
// 	if (containerWidgets_.top())
// 	{
// 		ShaderGroupInput *sgi = new ShaderGroupInput(containerWidgets_.top(),
// 			project,node,shaders,label);
// 		layouts_.top()->addWidget(sgi);
// 		return sgi;
// 	}
// 	return NULL;
// }

			
StringPropertyInput * PropertiesDialog::addStringInput(QString label,StringProperty *val)
{
	if (containerWidgets_.top())
	{
		QHBoxLayout *hb = new QHBoxLayout();
		layouts_.top()->addLayout(hb);
		hb->setContentsMargins(0,0,0,0);

		QLabel *l = new QLabel(label,containerWidgets_.top());
		hb->addWidget(l,1);
		
		StringPropertyInput *si=new StringPropertyInput(containerWidgets_.top(),val,label);
		hb->addWidget(si,1);
		si->show();
		
		return si;
	}
	return NULL;
}

FilePropertyInput * PropertiesDialog::addFileInput(
	QString label,FileProperty *filep,
	int inputKind,const char *dirName,const char *filter	)
{
	if (containerWidgets_.top())
	{
		FilePropertyInput *fi=new FilePropertyInput(containerWidgets_.top(),filep,label,inputKind,dirName,filter);
		layouts_.top()->addWidget(fi);
		return fi;
	}
	return NULL;
}

void PropertiesDialog::addCustomWidget(QWidget *w)
{
	if (containerWidgets_.top())
	{
		layouts_.top()->addWidget(w);
		//SaveWidget *sw = dynamic_cast<SaveWidget *>(w); // FIXME
		//if (sw)
		//	connect(sw,SIGNAL(createdDialog(PropertiesDialog *)),this,SIGNAL(createdDialog(PropertiesDialog *)));
	}
}

void PropertiesDialog::setActiveTab(QString label)
{
	// Allows us to add new widgets after a base dialog
	// has been created
	QWidget *w;
	if ((w = findChild<QWidget *>(label)))
	{
		if (containerWidgets_.top()) containerWidgets_.pop();
			containerWidgets_.push(w);
			// layouts_.top() = w->layout();
	}
	else{
		//qWarning() <<  warning.header() << "PropertiesDialog::setActiveTab() failed to find it";
	}
}



void PropertiesDialog::addPadding()
{
	if (layouts_.top()) layouts_.top()->addStretch(1);
}

void PropertiesDialog::addSeparator(QString label)
{
	if (containerWidgets_.top())
	{
		if (!label.isNull())
		{
			QHBoxLayout *hb = new QHBoxLayout();
			hb->setContentsMargins(0,0,0,0);
			hb->setSpacing(8);
			layouts_.top()->addLayout(hb);
			QLabel *l = new QLabel("",containerWidgets_.top());
			hb->addWidget(l,0);
			l->setPixmap(*dppm);
			l = new QLabel(label,containerWidgets_.top());
			hb->addWidget(l,1);
		}
		else
		{
		}
	}
	
}
		
//
// Protected
//

void PropertiesDialog::setChildren()
{
	
	QList<PropertyInput *> pl= this->findChildren<PropertyInput *>();
	for ( int i=0;i<pl.count();i++)
		pl[i]->set();

}

void PropertiesDialog::restoreChildren()
{
	
	QList<PropertyInput *> pl= this->findChildren<PropertyInput *>();
	for ( int i=0;i<pl.count();i++)
		pl[i]->restore();
	
	emit apply(true);
}
			
//
// Protected slots
//

void PropertiesDialog::help()
{
	// app->HelpBrowser("Channel Properties");
}

void PropertiesDialog::accept()
{

	// Input widgets are all children of a QWidget 
	setChildren();
	propertied_->propertiesDialogEvent(OKClicked);
	emit apply(OKClicked);
	
	QDialog::accept();
}

void PropertiesDialog::apply()
{
	setChildren();
	propertied_->propertiesDialogEvent(ApplyClicked);
	emit apply(ApplyClicked);
}

void PropertiesDialog::reject()
{
	// Undo any changes
	restoreChildren();
	propertied_->propertiesDialogEvent(CancelClicked);
	emit apply(CancelClicked);
	
	QDialog::reject();
	
}


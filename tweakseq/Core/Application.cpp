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

#include <QtDebug>
#include "DebuggingInfo.h"

#include <iostream>

#include <QDir>
#include <QMessageBox>
#include <QStringList>

#include "AboutDialog.h"
#include "Application.h"
#include "ClustalO.h"
#include "Muscle.h"
#include "Project.h"
#include "SetupWizard.h"

Application::Application(int &argc, char **argv):QApplication(argc,argv)
{
	app = this;
	
	init();
	connect(this,SIGNAL(lastWindowClosed()),this,SLOT(quit())); // FIXME mainwindows are parent to project so this may not work
	connect(this,SIGNAL(aboutToQuit()),this,SLOT(cleanup()));
}

bool Application::configure()
{
	// This does first time setup of the application if necessary
	
	// Create a directory for the app if it doesn't exist etc
	QString path = QDir::home().absolutePath();
	appDirPath_ = path + "/.tweakseq";
	QString msg="";
	QDir appDir(appDirPath_);
	if (!(appDir.exists())){
		appDir.setPath(path);
		appDir.mkdir(".tweakseq");
		msg = "The directory " + appDirPath_ + " was created";
	}
	
	// Load default settings 
	applicationSettingsFile_ = appDirPath_+"/defaults.xml";
	QFile defs(applicationSettingsFile_);
	if (defs.exists()){
		readSettings();
		return true;
	}
	
	// If we get here, then we have to run the wizard
	SetupWizard sw;
	sw.addMessage(msg);
	
	int ret = sw.exec();
	// 
	QString preferredTool;
	QString clustalO,muscle;
	
	if (ret == QDialog::Accepted){
		preferredTool = sw.preferredTool();
		sw.clustalOConfig(clustaloConfigured_,clustalO);
		sw.muscleConfig(muscleConfigured_,muscle);
	}
	else{
		QMessageBox::warning(NULL, tr("tweakseq"),
																tr("Configuration was canceled. The application will now exit."));
		return false;
	}
	
	// Create a default settings file
	QDomDocument saveDoc;
	QDomElement root = saveDoc.createElement("tweakseq");
	saveDoc.appendChild(root);
	
	QFileInfo fi(applicationSettingsFile_);
	QFile f(fi.filePath());
	f.open(IO_WriteOnly);
	QTextStream ts(&f);
	
	QDomElement el = saveDoc.createElement("version");
	root.appendChild(el);
	QDomText te = saveDoc.createTextNode(app->version());
	el.appendChild(te);
	
	if (clustaloConfigured_){
		ClustalO atool;
		atool.setPreferred(preferredTool == "clustalo");
		atool.setExecutable(clustalO);
		atool.writeSettings(saveDoc,root);
	}
	
	if (muscleConfigured_){
		Muscle atool;
		atool.setPreferred(preferredTool == "MUSCLE");
		atool.setExecutable(muscle);
		atool.writeSettings(saveDoc,root);
	}
	
	saveDoc.save(ts,2);
	f.close();
	
	// and read it again 
	readSettings();
	
	return true;
}

Project * Application::createProject()
{
	qDebug() << trace.header() << "Application::createProject()";
	Project *p = new Project();
	openProjects_.append(p);
	return p;
}

void Application::saveDefaultSettings(Project *project)
{
	// Saves  the settings in the Project as application defaults
	
	QDomDocument saveDoc;
	QDomElement root = saveDoc.createElement("tweakseq");
	saveDoc.appendChild(root);
	
	QFileInfo fi(applicationSettingsFile_);
	QFile f(fi.filePath());
	f.open(IO_WriteOnly);
	QTextStream ts(&f);
	
	QDomElement el = saveDoc.createElement("version");
	root.appendChild(el);
	QDomText te = saveDoc.createTextNode(app->version());
	el.appendChild(te);
	
	project->writeSettings(saveDoc,root);

	saveDoc.save(ts,2);
	f.close();
	
}
	
void Application::showHelp(QString)
{
	QMessageBox msgBox;
	msgBox.setText("Bwahahahaha\n\n\nNo.");
	msgBox.exec();
}

void Application::showAboutDialog(QWidget *parent)
{
	// Like help, we only want one instance of this for the application
	if (!aboutDlg)
		aboutDlg = new AboutDialog(parent);
	aboutDlg->show();
  aboutDlg->raise();
  aboutDlg->activateWindow();
}

QString Application::applicationTmpPath()
{
	return appDirPath_;
}

bool Application::alignmentToolAvailable(const QString &toolName){
	if (toolName == "clustalo")
		return clustaloConfigured_;
	if (toolName == "MUSCLE")
		return muscleConfigured_;
}

//	
//	Public  slots
//

//	
//	Private slots
//	

void Application::helpClosed()
{
}

void Application::cleanup()
{
	//saveDefaultSettings();
}

void Application::projectClosed(Project *p)
{
	qDebug() << trace.header() << "Application::projectClosed()";
	// Remove from the list
	openProjects_.removeOne(p);
	// FIXME If all projects closed then byebye
	if (openProjects_.isEmpty()){
		qDebug() << trace.header() << "Application::projectClosed() all closed";
	}
}

//		
//	Private members
//
	
void Application::init()
{
	aboutDlg = NULL;
	defaultSettings_ = new QDomDocument();
	clustaloConfigured_=muscleConfigured_=false;
}

void Application::readSettings()
{
	QFile defs(applicationSettingsFile_);
	if (defs.exists()){
		qDebug() << trace.header() << "Application::readSettings() reading " << applicationSettingsFile_;
		if ( !defs.open( IO_ReadOnly ) )
			return;
		QString err; int errlineno,errcolno;
		if ( !defaultSettings_->setContent(&defs,true,&err,&errlineno,&errcolno ) ){	
			qDebug() << trace.header() << " Application::init() error at line " << errlineno;
			defs.close();
			return;
		}
		
		// Now pick out some stuff
		QDomNodeList nl = defaultSettings_->elementsByTagName("alignment_tool");
		for (int i=0;i<nl.count();i++){
			QDomNode gNode = nl.item(i);
			QDomElement elem = gNode.firstChildElement();
			while (!elem.isNull()){
				if (elem.tagName() == "name"){
					if (elem.text()=="clustalo")
						clustaloConfigured_=true;
					else if (elem.text()=="MUSCLE")
						muscleConfigured_=true;
				}
				elem=elem.nextSiblingElement();
			}
		}
		
	}
}

void Application::writeSettings()
{
}

		
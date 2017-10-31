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
#include "Project.h"

Application::Application(int &argc, char **argv):QApplication(argc,argv)
{
	app = this;
	
	setup(); // run first time setup
	
	init();
	connect(this,SIGNAL(lastWindowClosed()),this,SLOT(quit())); // FIXME mainwindows are parent to project so this may not work
	connect(this,SIGNAL(aboutToQuit()),this,SLOT(cleanup()));
}

void Application::setup()
{
	// FIXME this does first time setup of the application
	// create a directory for the app if it doesn't exist etc
	QString path = QDir::home().absolutePath();
	appDirPath_ = path + "/.tweakseq";
	QDir appDir(appDirPath_);
	if (!(appDir.exists())){
		appDir.setPath(path);
		appDir.mkdir(".tweakseq");
	}
	// Create default preferences file
}

Project * Application::createProject()
{
	qDebug() << trace.header() << "Application::createProject()";
	Project *p = new Project();
	openProjects_.append(p);
	return p;
}

void Application::saveDefaultSettings()
{
	QString prefFile=appDirPath_+"/defaults.xml";
	//defaultSettings.saveIt(prefFile);
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

QString Application::applicationSettingsPath()
{
	return appDirPath_;
}

QString Application::applicationTmpPath()
{
	return appDirPath_;
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
	saveDefaultSettings();
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

	// load default settings 
	QString path = appDirPath_+"/defaults.xml";
	QFile defs(path);
	if (defs.exists()){
		//defaultSettings.load(path);
	}
	else{ // make a default settings document, save it and load it
		
	}
}

void Application::readSettings()
{
}

void Application::writeSettings()
{
}

		
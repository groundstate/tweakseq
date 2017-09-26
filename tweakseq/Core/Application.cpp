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

#include <iostream>

#include <QDir>
#include <QStringList>

#include "AboutDialog.h"
#include "Application.h"

Application::Application(int &argc, char **argv):QApplication(argc,argv)
{
	app = this;
	init();
	
	// Testing purposes only
	previousProjects.append("/home/mjw/da/src/untitled.da");
	previousProjects.append("/home/mjw/da/src/test.da");
	
	connect(this,SIGNAL(lastWindowClosed()),this,SLOT(quit()));
	connect(this,SIGNAL(aboutToQuit()),this,SLOT(cleanup()));
}

void Application::saveDefaultSettings()
{
	QString prefFile=appDirPath_+"/defaults.xml";
	//defaultSettings.saveIt(prefFile);
}
	
void Application::showHelp(const char *)
{
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

QString Application::ClustalWPath()
{
	QString path("./");
	//defaultSettings.getString("settings/clustalw/path",path);
	return path;
}


				
//	
//	public  slots
//

//	
//	private slots
//	

void Application::helpClosed()
{
}

void Application::cleanup()
{
	saveDefaultSettings();
}

//		
//	private members
//
	
void Application::init()
{
	aboutDlg = NULL;
	
	// create a directory for the app if it doesn't exist etc
	QString path = QDir::home().absolutePath();
	appDirPath_ = path + "/.da";
	QDir appDir(appDirPath_);
	if (!(appDir.exists()))
	{
		appDir.setPath(path);
		appDir.mkdir(".da");
	}
	
	// load default settings 
	path = appDirPath_+"/defaults.xml";
	QFile defs(path);
	if (defs.exists())
	{
		//defaultSettings.load(path);
	}
	else // make a default settings document, save it and load it
	{
		
	}
}

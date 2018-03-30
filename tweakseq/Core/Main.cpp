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

#include <QtDebug>
#include "DebuggingInfo.h"

#include <unistd.h>

#include <fstream>

#include "Application.h"
#include "DebuggingInfo.h"
#include "Project.h"

Application *app;

DebuggingInfo trace("TRACE");
DebuggingInfo warning("WARNING");
DebuggingInfo fixme("FIXME");
DebuggingInfo benchmark("BENCHMARK");

static bool traceOn=false;
static bool warningOn=false;
static bool benchmarkOn=false;

void myMessageOutput(QtMsgType type, const char *msg)
{
	switch (type) {
		case QtDebugMsg:
				if (traceOn)   fprintf(stderr,"%s\n",msg);
				break;
		case QtWarningMsg:
				if (warningOn) fprintf(stderr,"%s\n",msg);
				break;
		case QtCriticalMsg:
				fprintf(stderr,"%s\n",msg);
				break;
		case QtFatalMsg:
				fprintf(stderr,"%s\n",msg);
				abort();
	}
}

int main(int argc, char **argv){
	
	char c;
	trace.showTimestamp(false);
	trace.showFunctionName(true);
	
	//trace.showThread(true);

	while ((c=getopt(argc,argv,"tbfow")) != EOF)
  {
		switch (c)
		{
			case 't':traceOn=true;break;
			case 'b':benchmarkOn=true;break;
			case 'f':break;
			case 'w':warningOn=true;break;
			case 'o': // debugging to file
			break;
		}
	}
	qInstallMsgHandler(myMessageOutput);
	
	qDebug() << trace.header(__PRETTY_FUNCTION__)  << "application starting ...";
	
	Application a(argc, argv);
	if (a.configure()){
		Project *prj = a.createProject();
		prj->createMainWindow();
		if (optind == argc-1){
			QString fname = argv[optind];
			prj->load(fname);
		}
		return a.exec();
	}
	
	return EXIT_SUCCESS;
	
}

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

#ifndef __DEBUGGING_INFO_HEADER_H_
#define __DEBUGGING_INFO_HEADER_H_

#include <QString>

#include <sys/time.h>
#include <pthread.h>

class DebuggingInfo
{
	public:
		
		DebuggingInfo(QString name)
		{
			n=name;
			ton=false;
			tidon=false;
		}
		
		void showTimestamp(bool on){ton=on;}
		void showThread(bool on){tidon=on;}
		
		QString header()
		{
			QString ret(n);
			if (ton)
			{
				struct timeval tv;
				gettimeofday(&tv,NULL);
				QString tod;
				tod.sprintf("::%d.%06d",(int) tv.tv_sec,(int) tv.tv_usec);
				ret.append(tod);
			}
			if (tidon)
			{
				QString tid = QString("::")+QString::number(pthread_self());
				ret.append(tid);
			}
			
			return ret;
		};

	private:
		
		QString n;
		bool ton,tidon;
};

extern DebuggingInfo trace;
extern DebuggingInfo warning;
extern DebuggingInfo fixme;

#endif
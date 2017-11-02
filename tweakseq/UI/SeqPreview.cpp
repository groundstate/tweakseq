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



#include <QPainter>

#include "SeqPreview.h"

#define FLAGSWIDTH 4 // FIXME this is a bodge
#define LABELWIDTH 16

#define HPADDING 1.3
#define VPADDING 1.3

SeqPreview::SeqPreview()
{
	setMinimumSize(800,50);
	setStyleSheet("background-color:black;");
	
  int w;
	int h;
	// FIXME may have misunderstood here: 
	QFontInfo fi(font());
	QFont f = font();
	if (!fi.exactMatch()){
		f= QFont(fi.family(),fi.pointSize(),fi.weight());
	}
	QFontMetrics fm(f);
	w = fm.width('W');
	h = fm.width('W');
	cellWidth_ = (int) (w*HPADDING);                        
	cellHeight_= (int) (h*VPADDING);
	
}

SeqPreview::~SeqPreview()
{
}


void SeqPreview::setSequences(QStringList &labels,QStringList &residues)
{
	labels_ = labels;
	residues_ = residues;
	int maxlen=0;
	for (int s=0;s<residues.size();s++){
		if (residues.at(s).size() > maxlen)
			maxlen = residues.at(s).size();
	}
	setFixedSize((FLAGSWIDTH+LABELWIDTH + maxlen)*cellWidth_,residues.size()*cellHeight_);
}

void SeqPreview::setPreviewFont(const QFont &f)
{
  int w;
	int h;
	setFont(f);
	QFontInfo fi(f);
	QFont ftmp = f;
	if (!fi.exactMatch()){
		ftmp= QFont(fi.family(),fi.pointSize(),fi.weight());
	}
	QFontMetrics fm(ftmp);
	w = fm.width('W');
	h = fm.width('W');
	cellWidth_ = (int) (w*HPADDING);                        
	cellHeight_= (int) (h*VPADDING);
}

//
// Protected
//

void SeqPreview::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setPen(QColor(255,0,0));
	int x,y;
	QColor txtColor;
	
	for (int row=0;row<residues_.size();row++){
		y = row*cellHeight_;
		
		txtColor.setRgb(255,255,255);
		p.setPen(txtColor);
		QString l = labels_.at(row);
		for (int col=0;col<l.size();col++){
			QChar ch = l.at(col);
			x=(col+FLAGSWIDTH)*cellWidth_;
			p.drawText( x, y, cellWidth_, cellHeight_, Qt::AlignCenter, ch);
		}
		
		QString r = residues_.at(row);
		for (int col=0;col<r.size();col++){
			QChar ch = r.at(col);
			
			switch (ch.toAscii()){
				case 'D': case 'E': case 'S': case 'T':// red 
					txtColor.setRgb(255,0,0);
					break; 
				case 'R': case 'K': case 'H': // sky blue
					txtColor.setRgb(135,206,235);
					break;
				case 'Q': case 'N': // purple
					txtColor.setRgb(255,0,255);
					break; 
				case 'M': case 'C': // yellow
					txtColor.setRgb(255,255,0);
					break; 
				case 'A': case 'I': case 'L' : case 'V' : case 'G': case '-': 
				case '!'://white
					txtColor.setRgb(255,255,255);
					break; 
				case 'Y': case 'F': case 'W': // orange
					txtColor.setRgb(254,172,0);
					break; 
				case 'P': // green
					txtColor.setRgb(0,255,0);
					break;
			};
			
			p.setPen(txtColor);
			
			x=(col+FLAGSWIDTH+LABELWIDTH)*cellWidth_;
			p.drawText( x, y, cellWidth_, cellHeight_, Qt::AlignCenter, ch);
		}
	}
}

		

	


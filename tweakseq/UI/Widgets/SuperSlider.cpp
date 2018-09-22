//
// tweakseq - provides an editor for and interface to various sequence alignment tools
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2018  Merridee A. Wouters, Michael J. Wouters
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
#include <cfloat>

#include "SuperSlider.h"


SuperSlider::SuperSlider(QWidget *parent,double val,
	double min,double max):
		QSlider(Qt::Horizontal)
{
	
	min_=min;
	max_=max;
	if (min > max )
	{
		max_ = min;
		min_ = max;
	}
	currValue_=val;
	
	setTracking(true);
	setSliderfLimits();
	setValue(sliderPosition(val));
	setMinimumWidth(80);
	connect(this,SIGNAL(sliderMoved(int)),this,SLOT(handleValueChange(int)));
}

SuperSlider::~SuperSlider()
{
}
	
void SuperSlider::setfValue(double v)
{
	if (v==currValue_) return; // breaks infinite loops
	if (sliderPosition(v) == value()) return;
	currValue_=v;
	setSliderfLimits(); // these may need to be reworked
	setValue(sliderPosition(v));
}

//		
// Private slots
//
		
void SuperSlider::handleValueChange(int i)
{
	currValue_= currMin_ + i*(currMax_-currMin_)/(maximum()-minimum());
	if (currValue_ > currMax_) currValue_ = currMax_;
	if (currValue_ < currMin_) currValue_ = currMin_;
	emit fValueChanged(currValue_);
}

//
// Private members
//

void  SuperSlider::setSliderfLimits()
{
	// Attempts to set sensible limits
	// for the slider based on (min,max), the resolution
	// of the slider and the current/initial value
	
	// There are really only two cases here: a well-defined,finite
	// range eg colour component [0,1], an angle [0 to 360] 
	// and ranges like -inf,+inf 0,+inf for eg translation
	
	double r = max_ - min_;
	
	currMin_ = min_;
	currMax_ = max_;
		
	if (r > 0.99 * DBL_MAX)
	{
		if (currValue_ != 0.0)
		{
			double m = trunc(log10(fabs(currValue_)));
			if (m<=0) 
				m=2;
			else
				m++;
			currMax_ = pow(10.0,(double) m);
			if (currMax_ > max_) currMax_ = max_;
			if (currMax_ < 0) currMin_ = currMax_*100.0;
			if (currMax_ > 0) currMin_ = currMax_/100.0;
		}
		else // currValue_ == 0 so set range to 100
		{
			currMin_ = min_;
			currMax_ = max_+100.0;
				
			if (min_< -50.0)
			{
				currMin_=-50.0;
				currMax_= currMin_+100.0;
			}	
		}
		
		
	}
	
	
}

int SuperSlider::sliderPosition(double v)
{
	return (int) ((maximum()-minimum())*(v- currMin_)/(currMax_ - currMin_));
}

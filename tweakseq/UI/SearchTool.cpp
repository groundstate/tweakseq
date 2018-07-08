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

#include <QtDebug>
#include "DebuggingInfo.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

#include "Project.h"
#include "SearchResult.h"
#include "SearchTool.h"
#include "Sequence.h"
#include "Sequences.h"

//
// Public members
//

SearchTool::SearchTool(QWidget *parent):QWidget(parent)
{
	currSearchResult_ = 0;
	
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
	layout->setContentsMargins(0,0,0,0);
	QLabel *label = new QLabel("Search",this);
	
	layout->addWidget(label);
	
	searchBox_ = new QComboBox(this);
	searchBox_->setEditable(true);
	searchBox_->setMinimumWidth(200);
	connect(searchBox_,SIGNAL(activated(const QString &)),this,SLOT(cleanSearchString(const QString &)));
	layout->addWidget(searchBox_);
	
	nextSearchResult_ = new QPushButton("Next",this);
	nextSearchResult_->setIcon(QIcon(":/images/go-down-search.png"));
	nextSearchResult_->setEnabled(false);
	connect(nextSearchResult_,SIGNAL(clicked(bool)),this,SLOT(nextSearchResult(bool)));
	layout->addWidget(nextSearchResult_);
	
	prevSearchResult_ = new QPushButton("Previous",this);
	prevSearchResult_->setIcon(QIcon(":/images/go-up-search.png"));
	prevSearchResult_->setEnabled(false);
	connect(prevSearchResult_,SIGNAL(clicked(bool)),this,SLOT(previousSearchResult(bool)));
	layout->addWidget(prevSearchResult_);

	

}

void SearchTool::setSearchResults(QList< SearchResult* >&results)
{
	searchResults_=results;
	if (searchResults_.size() > 0){
		currSearchResult_=0;
		nextSearchResult_->setEnabled(true);
		prevSearchResult_->setEnabled(true);
	}
}

void SearchTool::setSearchText(QString &txt)
{
	searchBox_->setCurrentText(txt);
	emit search(txt);
}

void SearchTool::clearSearch()
{
	searchResults_.clear();
	nextSearchResult_->setEnabled(false);
	prevSearchResult_->setEnabled(false);
}

void SearchTool::nextSearchResult(bool)
{
	currSearchResult_++;
	if (currSearchResult_ >= searchResults_.size())
		currSearchResult_=0;
	emit goToSearchResult(currSearchResult_);
}

void SearchTool::previousSearchResult(bool)
{
	currSearchResult_--;
	if (currSearchResult_ < 0)
		currSearchResult_= searchResults_.size()-1;
	emit goToSearchResult(currSearchResult_);
}

void SearchTool::cleanSearchString(const QString &searchString)
{
	QString str = searchString;
	str = str.trimmed();
	emit search(str);
}


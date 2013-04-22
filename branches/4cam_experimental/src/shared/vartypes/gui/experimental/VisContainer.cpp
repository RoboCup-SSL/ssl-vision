//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    VisContainer.cpp
  \brief   C++ Implementation: VisContainer
  \author  Stefan Zickler, (C) 2008
*/
#include "VisContainer.h"

VisContainer::VisContainer()
{
	widget=new QWidget(viewport());
	layout=new QVBoxLayout();
	//layout->addWidget(new ValueVis(0));	
	//layout->addWidget(new ValueVis(0));
	
	layout->setMargin(4);
	layout->setSpacing(2);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	widget->setLayout(layout);
	
	
	
	//widget->setMinimumWidth(200);
	//widget->setMinimumHeight(200);
  setWidget(widget);
   
}

VisContainer::~VisContainer()
{
}

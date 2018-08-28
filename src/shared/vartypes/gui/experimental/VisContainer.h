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
  \file    VisContainer.h
  \brief   C++ Interface: VisContainer
  \author  Stefan Zickler, (C) 2008
*/
#ifndef VISCONTAINER_H_
#define VISCONTAINER_H_
#include <QScrollArea>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollBar>
#include "ValueVis.h"
class VisContainer : public QScrollArea
{
protected:
	QWidget * widget;
	
		
public:
	VisContainer();
	QVBoxLayout * layout;
	virtual ~VisContainer();
};

#endif /*VISCONTAINER_H_*/

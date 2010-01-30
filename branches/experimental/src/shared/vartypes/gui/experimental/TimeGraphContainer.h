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
  \file    TimeGraphContainer.h
  \brief   C++ Interface: TimeGraphContainer
  \author  Stefan Zickler, (C) 2008
*/
#ifndef TIMEGRAPHCONTAINER_H_
#define TIMEGRAPHCONTAINER_H_
#include <QVBoxLayout>
#include <QWidget>
#include "TimeControlWidget.h"
#include "GraphScene.h"
#include <QGraphicsView>

class TimeGraphContainer : public QWidget
{
Q_OBJECT
protected:
	QList<QGraphicsView *> list;
	QVBoxLayout * outer_layout;
	QVBoxLayout * inner_layout;
	TimeControlWidget * time_widget;
public:
	void addGraphScene(GraphScene * widget);
	TimeControl * getTimeControl() {
		return time_widget->getTimeControl();
	}
	TimeGraphContainer();
	virtual ~TimeGraphContainer();
	void redraw();
	virtual void paintEvent ( QPaintEvent * event ); 
};

#endif /*TIMEGRAPHCONTAINER_H_*/

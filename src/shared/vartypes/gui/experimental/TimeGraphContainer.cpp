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
  \file    TimeGraphContainer.cpp
  \brief   C++ Implementation: TimeGraphContainer
  \author  Stefan Zickler, (C) 2008
*/
#include "TimeGraphContainer.h"

TimeGraphContainer::TimeGraphContainer()
{
	time_widget = new TimeControlWidget();
	outer_layout = new QVBoxLayout();
	inner_layout = new QVBoxLayout();
	outer_layout->addLayout(inner_layout);
	outer_layout->addWidget(time_widget);
	
	inner_layout->setMargin(0);
	inner_layout->setSpacing(0);
	
	outer_layout->setMargin(0);
	outer_layout->setSpacing(0);
	setLayout(outer_layout);
}

TimeGraphContainer::~TimeGraphContainer()
{
}


void TimeGraphContainer::addGraphScene(GraphScene * scene) {
	scene->setTimeControl(time_widget->getTimeControl());

	QGraphicsView * v = new QGraphicsView();
	v->scale(1.0,-1.0); 
	v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	list.append(v);
	v->setScene(scene); 
	inner_layout->addWidget(v);
	
}

void TimeGraphContainer::paintEvent ( QPaintEvent * event ) {
	redraw();
	QWidget::paintEvent(event);
	
	
}

void TimeGraphContainer::redraw() {
	//redraw it all:
	QGraphicsView * v;
	GraphScene * gs;
	for (int i=0;i<list.size();i++) {
		v=list.at(i);
		gs=(GraphScene *)v->scene();
		gs->updateArea();
		//v->fitInView ( 0,-10,3,20);
		
		v->fitInView ( (double)time_widget->getTimeControl()->getVisibleRange()->getMin() ,
									 (double)gs->sceneRect().y(),
									 (double)time_widget->getTimeControl()->getVisibleRange()->getLength(),
									  (double)gs->sceneRect().height(),
									  //20,
									  Qt::IgnoreAspectRatio );
									  
		qDebug("fit %f, %f, %f, %f\n",(double)time_widget->getTimeControl()->getVisibleRange()->getMin() ,
									 (double)gs->sceneRect().y(),
									 (double)time_widget->getTimeControl()->getVisibleRange()->getLength(),
									  (double)gs->sceneRect().height());
	}
}

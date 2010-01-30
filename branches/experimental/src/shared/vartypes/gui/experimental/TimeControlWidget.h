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
  \file    TimeControlWidget.h
  \brief   C++ Interface: TimeControlWidget
  \author  Stefan Zickler, (C) 2008
*/

#ifndef TIMECONTORLWIDGET_H_
#define TIMECONTORLWIDGET_H_
#include "TimeControl.h"
#include "ui_timewidget.h"
#include <QWidget>

class TimeControlWidget : public QWidget, public Ui_TimeWidget
{
	Q_OBJECT
protected:
	QVBoxLayout l;
	TimeControl * control;
	TimeRange range_full;
	TimeRange range_visible;
	TimePointer time_current;
	

public:
	
	TimeControl * getTimeControl() {
		return control;
	}	
	void redraw();
	
	TimeControlWidget(TimeControl * c=0);
	virtual ~TimeControlWidget();
};

#endif /*TIMEWIDGET_H_*/

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
  \file    TimeControlWidget.cpp
  \brief   C++ Implementation: TimeControlWidget
  \author  Stefan Zickler, (C) 2008
*/
#include "TimeControlWidget.h"

TimeControlWidget::TimeControlWidget(TimeControl * c)
{
	
	setupUi((QWidget *)this);
	if (c==0) c = new TimeControl();
	control=c;
	redraw();
}

TimeControlWidget::~TimeControlWidget()
{
}

void TimeControlWidget::redraw() {
	
	scroller->setMinimum((int)range_full.getMin());
	scroller->setMaximum((int)(range_full.getMax()+1.0));
	scroller->setPageStep(range_visible.getLength());
	scroller->setSliderPosition(range_visible.getMin());
	label1->setText("Total [ " +
	QString::number(range_full.getMin()) +
	" , " +
	QString::number(range_full.getMax()) +
	" ] (Range: " +
	QString::number(range_full.getLength()) +
	")"
	);

	label2->setText("Selected [ " +
	QString::number(range_visible.getMin()) +
	" , " +
	QString::number(range_visible.getMax()) +
	" ] (Range: " +
	QString::number(range_visible.getLength()) +
	")"
	);


}

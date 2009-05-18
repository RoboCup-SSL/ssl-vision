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
  \file    jog_dial.cpp
  \brief   C++ Implementation: JogDial
  \author  Roman Shtylman, 2009
*/
//========================================================================
#include "jog_dial.h"

#include <QMouseEvent>
#include <math.h>
#include <QPainter>


JogDial::JogDial(QWidget* parent) :
	QWidget(parent)
{
	_offset = 0;
  QWidget::setMinimumSize(40,10);
	_spacing = 10.0f;
	_viewSpan = 180.0f;
}

void JogDial::paintEvent(QPaintEvent* pe)
{
	QPainter p(this);
	mutex.lock();
	const float s = _offset - _spacing * (int)(_offset/_spacing);
	
	for (float angle = s - _viewSpan/2.0; angle <= s + _viewSpan/2.0 ; angle += _spacing)
	{
		int pos = (int)(sin(angle * M_PI/180.0f) * width()/2.0 + width()/2.0);
		
		p.drawLine(pos, 0, pos, height());
	}
	
  mutex.unlock(); 
	p.drawLine(0, 0, width(), 0);
	p.drawLine(0, height()-1, width(), height()-1);
}

void JogDial::mousePressEvent(QMouseEvent* me)
{
 mutex.lock();
 _x = me->x();
 mutex.unlock();
}

void JogDial::mouseMoveEvent(QMouseEvent* me)
{
  mutex.lock();
	int x = me->x();
	
	int diff = x - _x;
	_offset = diff * _viewSpan/width();
	
  mutex.unlock();
	update();
	valueChanged(offset());
}

void JogDial::mouseReleaseEvent(QMouseEvent* me)
{
  mutex.lock();
	_offset = 0;
  mutex.unlock(); 
	update();
	valueChanged(offset());
}

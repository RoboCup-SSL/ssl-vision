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
  \file    colorpicker.h
  \brief   C++ Interface: ColorPicker
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#ifndef COLORPICKER_H_
#define COLORPICKER_H_

#include "colors.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>

/*!
  \class   Swatch
  \brief   A widget for displaying a single color selection
  \author  Stefan Zickler, (C) 2008
*/
class Swatch : public QWidget {
  public:
    QColor color;

    Swatch() {
      color=Qt::white;
      setMinimumSize ( QSize ( 16, 16 ) );
      setMaximumSize ( QSize ( 16, 16 ) );
      setBaseSize ( QSize ( 16, 16 ) );
    }
    virtual ~Swatch() {

    }
    void setColor ( rgb c ) {
      color=QColor ( c.r,c.g,c.b );
    }
    void paintEvent ( QPaintEvent * event ) {
      ( void ) event;
      QPainter p ( this );
      p.setBrush ( color );
      p.drawRect ( 0,0,width(),height() );
    }
};

/*!
  \class   ColorPicker
  \brief   A widget for picking and displaying a single color selection
  \author  Stefan Zickler, (C) 2008
*/
class ColorPicker : public QWidget {
  public:
    Swatch * swatch;
    QLabel * label;
    ColorPicker();
    virtual ~ColorPicker();
    void setColor ( rgb color );
};

#endif /*COLORPICKER_H_*/

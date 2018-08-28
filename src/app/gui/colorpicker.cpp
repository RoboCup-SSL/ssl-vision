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
  \file    colorpicker.cpp
  \brief   C++ Implementation: ColorPicker
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#include "colorpicker.h"

ColorPicker::ColorPicker() {
  QHBoxLayout *hboxLayout;
  hboxLayout = new QHBoxLayout ( this );
  hboxLayout->setSpacing ( 4 );
  hboxLayout->setMargin ( 2 );

  swatch = new Swatch();
  label = new QLabel();

  hboxLayout->addWidget ( swatch );
  hboxLayout->addWidget ( label );
  label->setVisible ( true );
  swatch->setVisible ( true );
  setLayout ( hboxLayout );
  /*QFont font = label->font();
  font.setPointSize(7);
  label->setFont(font);*/
  setColor ( rgb ( 128,128,128 ) );

}

ColorPicker::~ColorPicker() {
}

void ColorPicker::setColor ( rgb color ) {
  swatch->setColor ( color );
  swatch->update();
  label->setText ( QString::number ( color.r ) + " " + QString::number ( color.g ) + " " + QString::number ( color.b ) );
  label->update();
}

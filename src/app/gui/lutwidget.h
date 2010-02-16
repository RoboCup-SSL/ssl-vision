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
  \file    lutwidget.h
  \brief   C++ Interface: LUTWidget
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef LUTWIDGET_H
#define LUTWIDGET_H
#include "glLUTwidget.h"
#include "lut3d.h"
#include <QWidget>
#include <QListWidget>
#include <QToolBar>

/*!
  \file    lutwidget.h
  \brief   An fully-fledged editor for 3D Color LUTs of type LUT3D
  \author  Stefan Zickler, (C) 2008
           This editor acts as a wrapper around the 3D GLLUTWidget.
*/
class LUTWidget : public QWidget {
Q_OBJECT
protected:
    QBoxLayout  * vbox;
    QBoxLayout  * hbox;
    QListWidget * list;
    QLabel      * label;
    GLLUTWidget * gllut;
    QToolBar * toolbar;
    void updateList(LUT3D * lut);
    LUTChannelMode _mode;
protected slots:
    void selectChannel(int c);
public:
    GLLUTWidget * getGLLUTWidget();
    void samplePixel(const yuv & color);
    void add_del_Pixel(yuv color, bool add, bool continuing_undo);
    void sampleImage(const RawImage & img);
    void focusInEvent ( QFocusEvent * event );
    LUTWidget(LUT3D * lut, LUTChannelMode mode);
    ~LUTWidget();
};

#endif

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
  \file    neurowidget.h
  \brief   C++ Implementation: NeuroWidget
  \author  J. A. Gurzoni Jr, (C) 2012
*/
//========================================================================

#ifndef NEUROWIDGET_H
#define NEUROWIDGET_H
#include "glLUTwidget.h"
#include "lut3d.h"
#include <QWidget>
#include <QListWidget>
#include <QToolBar>
#include <QtGui>
#include <string>
#include "ui_neurowidget.h"

/**!
  \file    lutwidget.h
  \brief   An fully-fledged editor for 3D Color LUTs of type LUT3D
  \author  Stefan Zickler, (C) 2008
           This editor acts as a wrapper around the 3D GLLUTWidget.
*/
class NeuroWidget : public QWidget, private Ui_NeuroWidget {
Q_OBJECT
protected:

    //QPushButton* btn_train;
    //QPushButton* btn_loadnn;
    //QPushButton* btn_clear;
    QBoxLayout  * vbox;
    QBoxLayout  * hbox;
    QListWidget * list;
    //QLabel * lbl_sample;
    //QLabel      * label;
    GLLUTWidget * gllut;
    QToolBar * toolbar;
    void updateList(LUT3D * lut);
    LUTChannelMode _mode;
    int currentChannel;
protected slots:
    void selectChannel(int c);
    void is_clicked_btn_loadnn();
    void is_clicked_btn_train();\
    void is_clicked_btn_clear();
public:
    bool pending_loadnn;
    bool pending_train;
    bool pending_clear;
    int samples_captured;
public:
    int getStateChannel();
    GLLUTWidget * getGLLUTWidget();
    void samplePixel(const yuv & color);
    void add_del_Pixel(yuv color, bool add, bool continuing_undo);
    void sampleImage(const RawImage & img);
    void focusInEvent ( QFocusEvent * event );
    void setStatusLabel (const QString &str_input);
    void setSamplesLabel (int samples);
    NeuroWidget(LUT3D * lut, LUTChannelMode mode);
    ~NeuroWidget();
};

#endif

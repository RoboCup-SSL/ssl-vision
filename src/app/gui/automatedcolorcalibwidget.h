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
  \file    automatedcolorcalibwidget.h
  \brief   C++ Interface: AutomatedColorCalibWidget
  \author  Mark Geiger, (C) 2017
*/
//========================================================================

#ifndef AUTOMATEDCOLORCALIBWIDGET_H
#define AUTOMATEDCOLORCALIBWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QListWidget>
#include <lut3d.h>
#include "glLUTwidget.h"

class AutomatedColorCalibWidget : public QWidget {
Q_OBJECT

private:
    QLabel * status_label;
    QListWidget * list;
    void updateList(const LUT3D * lut);

public:
    explicit AutomatedColorCalibWidget(const LUT3D * lut);

    ~AutomatedColorCalibWidget() override = default;

    void focusInEvent(QFocusEvent *event) override;
    void set_status(const QString &status);

    int currentChannel = -1;
    bool pending_reset = false;
    bool pending_reset_lut = false;
    bool pending_update = false;

public slots:

    void selectChannel(int c);
    void reset();
    void resetLut();
    void initialize();
    void update();
};

#endif

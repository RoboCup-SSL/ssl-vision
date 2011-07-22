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
  \file    CentralWindow.h
  \brief   C++ Interface: CentralWindow
  \author  Ulfert Nehmiz, 2009
*/
//========================================================================

#ifndef CENTRALWINDOW_H
#define CENTRALWINDOW_H

#include <QMainWindow>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QDockWidget>
#include <QtGui/QWidget>
#include <QtGui/QSlider>
#include <QtGui/QLabel>
#include <QtGui/QLCDNumber>
#include "GraphicsPrimitives.h"
#include "ClientThreading.h"

class CentralWindow : public QMainWindow
{
    Q_OBJECT

public:
    CentralWindow();
    ~CentralWindow();

    QWidget *centralwidget;
    QFrame *Spielfeld;
    QGridLayout *gridLayout_7;
    QGridLayout *gridLayout;
    SoccerView* soccerView;
    ViewUpdateThread* thread;

    QMutex* drawMutex;

    //LogPlayerControl
    QDockWidget *logControl;
    QWidget *logControlWidget;
    QSlider *horizontalSlider;
    QPushButton *log_backward;
    QPushButton *log_pause;
    QPushButton *log_forward;
    QPushButton *log_play;
    QPushButton *log_slower;
    QLabel *log_speed;
    QPushButton *log_faster;
    QPushButton *log_frame_back;
    QLCDNumber *log_frameNumber;
    QPushButton *log_frame_forward;
    QLCDNumber *log_totalFrames;

public slots:
    void initializeSlider(int, int, int, int, int);

signals:

};

#endif //CENTRALWINDOW_H

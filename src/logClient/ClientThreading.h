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
  \file    ClientThreading.h
  \brief   C++ Interface: ViewUpdateThread
  \author  Joydeep Biswas, Stefan Zickler (C) 2009
  \edit    Ulfert Nehmiz (LogPlayer included) 2009
*/
//========================================================================

#ifndef VIEWUPDATETHREAD_H
#define VIEWUPDATETHREAD_H

#include <QThread>
#include <QVector>
#include <QPointF>
#include "GraphicsPrimitives.h"
#include "robocup_ssl_client.h"
#include "timer.h"
#include "LogControl.h"

class ViewUpdateThread : public QThread
{
    Q_OBJECT

  private:
    bool shutdownView;
    RoboCupSSLClient client;
    SSL_WrapperPacket packet;
    SoccerView *soccerView;
    int execute();

    //Logplayer
    Refbox_Log logs;
    bool play;
    int start_play_record();
    void end_play_record();
    QString fileName;

  protected:
    QMutex* drawMutex;

  public:
    ViewUpdateThread() {}
    ViewUpdateThread ( SoccerView *_soccer, QMutex* _drawMutex );
    ~ViewUpdateThread() {}
    void run();
    void Terminate();

    //Logplayer
    LogControl* log_control;

  //LogPlayer data
  public slots:
    void playLogfilePressed();

  signals:
    //Slider and QLCDNumber
    void update_frame(int);
    //QLCDNumber
    void log_size(int);
    //Slider
    void initializeSlider(int,int,int,int,int);
    //LogControl
    void showLogControl(bool);
    //change Button text
    void change_play_button(QString);

};

#endif // VIEWUPDATETHREAD_H

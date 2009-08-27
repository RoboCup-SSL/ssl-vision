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
  \file    LogControl.h
  \brief   C++ Interface: LogControl
  \author  Ulfert Nehmiz, 2009
*/
//========================================================================

#ifndef LOGCONTROL_H
#define LOGCONTROL_H

#include <QObject>
#include <iostream>

class LogControl : public QObject
{
    Q_OBJECT

public:
    LogControl();
    ~LogControl();

    void reset(int);
    int get_current_frame();
    int get_next_frame();
    int get_prop_next_frame();
    double get_play_speed();

public slots:
    void log_forward();
    void log_backward();
    void log_play();
    void log_pause();
    void log_faster();
    void log_slower();
    void log_frame_forward();
    void log_frame_back();
    void goto_frame(int);

signals:
    void update_speed(QString);

private:
    int current_frame;
    int next_frame;
    int log_length;
    double play_speed;
    double play_speed_save;
    void update_play_speed();

};

#endif //LOGCONTROL_H

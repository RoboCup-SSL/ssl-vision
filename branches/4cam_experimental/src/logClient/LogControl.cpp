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
  \file    LogControl.cpp
  \brief   C++ Implementation: LogControl
  \author  Ulfert Nehmiz, 2009
*/
//========================================================================

#include "LogControl.h"
#include <math.h>
#include <sstream>

LogControl::LogControl()
{
    reset(0);
}

LogControl::~LogControl()
{
    ;
}

void LogControl::reset(int size)
{
    current_frame = 0;
    next_frame = 0;
    log_length = size;
    play_speed = 1.;
    play_speed_save = 1.;
    update_play_speed();
}

int LogControl::get_current_frame()
{

    if(current_frame >= log_length)
        return 0;

    return current_frame;
}

int LogControl::get_next_frame()
{
    if(play_speed == 0.)
        return current_frame;

    current_frame = next_frame;
    if(play_speed < 0.)
        next_frame--;
    else
        next_frame++;

    if(current_frame >= log_length || current_frame < 0)
        return -1;

    return current_frame;
}

int LogControl::get_prop_next_frame()
{
    int prop_next_frame = current_frame;
    if(play_speed < 0.)
        prop_next_frame--;
    else if(play_speed > 0.)
        prop_next_frame++;

    if(prop_next_frame < 0 || prop_next_frame >= log_length)
        return -1;

    return prop_next_frame;
}

double LogControl::get_play_speed()
{
    return play_speed;
}

// Slots
void LogControl::log_forward()
{
    play_speed = 5.;
    update_play_speed();
}

void LogControl::log_backward()
{
    play_speed = -5.;
    update_play_speed();
}

void LogControl::log_play()
{
    if(play_speed != 0.)
        play_speed = play_speed_save;
    else
        play_speed = 1.;
    update_play_speed();
}

void LogControl::log_pause()
{
    play_speed_save = play_speed;
    play_speed = 0.;
    update_play_speed();
}

void LogControl::log_faster()
{
    play_speed *= 2.;
    update_play_speed();
}

void LogControl::log_slower()
{
    play_speed /= 2.;
    update_play_speed();
}

void LogControl::log_frame_back()
{
    current_frame--;
}

void LogControl::log_frame_forward()
{
    current_frame++;
}

void LogControl::goto_frame(int f)
{
    current_frame = f;
    next_frame = f+1;
}

void LogControl::update_play_speed()
{
    double speed_tmp = floor(play_speed * 100.0 + 0.5) / 100.0;
    std::ostringstream o;
    o << speed_tmp;
    QString speed = QString::number(play_speed, 'g', 4);

    speed.append("x");
    emit update_speed(speed);
}

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
  \file    videostats.h
  \brief   A class for storing video display statistics.
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef VIDEOSTATS_H_
#define VIDEOSTATS_H_
#include "capturestats.h"

/*!
  \class   VideoStats
  \brief   A class for storing video display statistics.
  \author  Stefan Zickler, (C) 2008
*/
class VideoStats {
  public:
    double fps_draw;
    double fps_loop;
    double time_running;
    long frame_count;
    CaptureStats capture_stats;

    VideoStats() {
      fps_draw=0.0;
      fps_loop=0.0;
      time_running=0.0;
      frame_count=0;

    };
};
#endif /*VIDEOSTATS_H_*/

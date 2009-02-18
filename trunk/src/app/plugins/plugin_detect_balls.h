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
  \file    plugin_detect_balls.h
  \brief   C++ Interface: plugin_detect_balls
  \author  Author Name, 2009
*/
//========================================================================
#ifndef PLUGIN_DETECT_BALLS_H
#define PLUGIN_DETECT_BALLS_H

#include <visionplugin.h>
#include "lut3d.h"
/**
	@author Author Name
*/
class PluginDetectBalls : public VisionPlugin
{
protected:
  LUT3D * _lut;
  VarList * _settings;

  VarInt    * _max_balls;
  VarString * _color_label;
  VarList   * _filter_general;
    VarDouble * _ball_max_speed;
    VarInt    * _ball_min_width;
    VarInt    * _ball_max_width;
    VarInt    * _ball_min_height;
    VarInt    * _ball_max_height;
    VarInt    * _ball_min_area;
    VarInt    * _ball_max_area;
  VarList   * _filter_too_near_robot;
    VarBool   * _ball_too_near_robot_enabled;
    VarDouble * _ball_too_near_robot_dist;
  VarList   * _filter_histogram;
    VarBool   * _ball_histogram_enabled;
  VarList   * _filter_geometry;
    VarBool   * _ball_on_field_filter;
    VarBool   * _ball_in_goal_filter;

public:
    PluginDetectBalls(FrameBuffer * _buffer, LUT3D * lut);

    ~PluginDetectBalls();

    virtual VarList * getSettings();
    virtual string getName();
};

#endif

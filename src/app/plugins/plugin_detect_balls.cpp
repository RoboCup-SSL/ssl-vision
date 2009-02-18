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
  \file    plugin_detect_balls.cpp
  \brief   C++ Implementation: plugin_detect_balls
  \author  Author Name, 2009
*/
//========================================================================
#include "plugin_detect_balls.h"

PluginDetectBalls::PluginDetectBalls(FrameBuffer * _buffer, LUT3D * lut)
 : VisionPlugin(_buffer)
{
  _lut=lut;

  _settings=new VarList("Ball Detection");

  _settings->addChild(_max_balls = new VarInt("Max Ball Count",1));
  _settings->addChild(_color_label = new VarString("Ball Color","Orange"));

  _settings->addChild(_filter_general = new VarList("Ball Properties"));
    _filter_general->addChild(_ball_max_speed = new VarDouble("Max Speed (m/s)", 10.0));
    _filter_general->addChild(_ball_min_width = new VarInt("Min Width (pixels)", 3));
    _filter_general->addChild(_ball_max_width = new VarInt("Max Width (pixels)", 30));
    _filter_general->addChild(_ball_min_height = new VarInt("Min Height (pixels)", 3));
    _filter_general->addChild(_ball_max_height = new VarInt("Max Height (pixels)", 30));
    _filter_general->addChild(_ball_min_area = new VarInt("Min Area (pixels)", 9));
    _filter_general->addChild(_ball_max_area = new VarInt("Max Area (pixels)", 100));    

  _settings->addChild(_filter_too_near_robot = new VarList("Near Robot Filter"));
    _filter_too_near_robot->addChild(_ball_too_near_robot_enabled = new VarBool("Enable Filter",true));
    _filter_too_near_robot->addChild(_ball_too_near_robot_dist = new VarDouble("Distance (m)",0.5));

  _settings->addChild(_filter_histogram = new VarList("Histogram Filter"));
    _filter_histogram->addChild(_ball_histogram_enabled = new VarBool("Enable Filter",true));

  _settings->addChild(_filter_geometry = new VarList("Geometry Filters"));
    _filter_geometry->addChild(_ball_on_field_filter = new VarBool("Ball-In-Field Filter",true));  
    _filter_geometry->addChild(_ball_in_goal_filter = new VarBool("Ball-In-Goal Filter",true));

}


PluginDetectBalls::~PluginDetectBalls()
{
}


VarList * PluginDetectBalls::getSettings() {
  return _settings;
}

string PluginDetectBalls::getName() {
  return "DetectBalls";
}


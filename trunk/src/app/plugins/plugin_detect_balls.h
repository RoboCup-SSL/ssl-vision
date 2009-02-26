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
#include "cmvision_region.h"
#include "messages_robocup_ssl_detection.pb.h"
#include "camera_calibration.h"
#include "field_filter.h"
#include "cmvision_histogram.h"
#include "vis_util.h"
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
    VarDouble * _ball_z_height;
    VarDouble * _ball_max_speed;
    VarInt    * _ball_min_width;
    VarInt    * _ball_max_width;
    VarInt    * _ball_min_height;
    VarInt    * _ball_max_height;
    VarInt    * _ball_min_area;
    VarInt    * _ball_max_area;
    
  VarList * _filter_gauss;
    VarBool * _ball_gauss_enabled;
    VarInt  * _ball_gauss_min;
    VarInt  * _ball_gauss_max;
    VarDouble * _ball_gauss_var;
    
  VarList   * _filter_too_near_robot;
    VarBool   * _ball_too_near_robot_enabled;
    VarDouble * _ball_too_near_robot_dist;
  VarList   * _filter_histogram;
    VarBool   * _ball_histogram_enabled;
    VarDouble * _ball_histogram_min_greenness;
    VarDouble * _ball_histogram_max_markeryness;
  VarList   * _filter_geometry;
    VarBool   * _ball_on_field_filter;
    VarBool   * _ball_in_goal_filter;

  int color_id_orange;
  int color_id_pink;
  int color_id_yellow;
  int color_id_field;

  CMVision::Histogram * histogram;

  CMVision::RegionFilter filter;

  const CameraParameters& camera_parameters;
  const RoboCupField& field;

  FieldFilter field_filter;

  bool checkHistogram(const Image<raw8> * image, const CMVision::Region * reg, double min_greenness=0.5, double max_markeryness=2.0);

public:
    PluginDetectBalls(FrameBuffer * _buffer, LUT3D * lut, const CameraParameters& camera_params, const RoboCupField& field);

    ~PluginDetectBalls();

    virtual ProcessResult process(FrameData * data, RenderOptions * options);
    virtual VarList * getSettings();
    virtual string getName();
};

#endif

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
#include "VarNotifier.h"
#include "lut3d.h"
/**
	@author Author Name
*/
class PluginDetectBalls;

class PluginDetectBallsSettings {
friend class PluginDetectBalls; 
protected:
  VarList * _settings;

  VarInt    * _max_balls;
  VarString * _color_label;
  VarList   * _filter_general;
    VarDouble * _ball_z_height;
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
    VarDouble * _ball_gauss_stddev;
    
  VarList   * _filter_too_near_robot;
    VarBool   * _ball_too_near_robot_enabled;
    VarDouble * _ball_too_near_robot_dist;
  VarList   * _filter_histogram;
    VarBool   * _ball_histogram_enabled;
    VarDouble * _ball_histogram_min_greenness;
    VarDouble * _ball_histogram_max_markeryness;
  VarList   * _filter_geometry;
    VarBool   * _ball_on_field_filter;
    VarDouble * _ball_on_field_filter_threshold;
    VarBool   * _ball_in_goal_filter;

public:
  PluginDetectBallsSettings() {

  _settings=new VarList("Ball Detection");

  _settings->addChild(_max_balls = new VarInt("Max Ball Count",10));
  _settings->addChild(_color_label = new VarString("Ball Color","Orange"));

  _settings->addChild(_filter_general = new VarList("Ball Properties"));
    _filter_general->addChild(_ball_z_height = new VarDouble("Ball Z-Height", 30.0));
    _filter_general->addChild(_ball_min_width = new VarInt("Min Width (pixels)", 3));
    _filter_general->addChild(_ball_max_width = new VarInt("Max Width (pixels)", 30));
    _filter_general->addChild(_ball_min_height = new VarInt("Min Height (pixels)", 3));
    _filter_general->addChild(_ball_max_height = new VarInt("Max Height (pixels)", 30));
    _filter_general->addChild(_ball_min_area = new VarInt("Min Area (sq-pixels)", 9));
    _filter_general->addChild(_ball_max_area = new VarInt("Max Area (sq-pixels)", 1000));    

  _settings->addChild(_filter_gauss = new VarList("Gaussian Size Filter"));
    _filter_gauss->addChild(_ball_gauss_enabled = new VarBool("Enable Filter",true));
    _filter_gauss->addChild(_ball_gauss_min = new VarInt("Expected Min Area (sq-pixels)", 30));
    _filter_gauss->addChild(_ball_gauss_max = new VarInt("Expected Max Area (sq-pixels)", 40));
    _filter_gauss->addChild(_ball_gauss_stddev = new VarDouble("Expected Area StdDev (sq-pixels)", 10.0));

  _settings->addChild(_filter_too_near_robot = new VarList("Near Robot Filter"));
    _filter_too_near_robot->addChild(_ball_too_near_robot_enabled = new VarBool("Enable Filter",true));
    _filter_too_near_robot->addChild(_ball_too_near_robot_dist = new VarDouble("Distance (mm)",70));

  _settings->addChild(_filter_histogram = new VarList("Histogram Filter"));
    _filter_histogram->addChild(_ball_histogram_enabled = new VarBool("Enable Filter",true));
    _filter_histogram->addChild(_ball_histogram_min_greenness = new VarDouble("Min Greenness",0.5));
    _filter_histogram->addChild(_ball_histogram_max_markeryness = new VarDouble("Max Markeryness",2.0));

  _settings->addChild(_filter_geometry = new VarList("Geometry Filters"));
    _filter_geometry->addChild(_ball_on_field_filter = new VarBool("Ball-In-Field Filter",true));
    _filter_geometry->addChild(_ball_on_field_filter_threshold = new VarDouble("Ball-In-Field Extra Space (mm)",30.0));
    _filter_geometry->addChild(_ball_in_goal_filter = new VarBool("Ball-In-Goal Filter",true));

  }
  VarList * getSettings() {
    return _settings;
  }

};

class PluginDetectBalls : public VisionPlugin
{
protected:
  //-----------------------------
  //local copies of the vartypes tree for better performance
  //these are updated automatically if a change is reported by vartypes
  VarNotifier vnotify;
  bool filter_ball_in_field;
  double filter_ball_on_field_filter_threshold;
  bool filter_ball_in_goal;
  bool filter_ball_histogram;
  double min_greenness;
  double max_markeryness;
  bool filter_gauss;
  int exp_area_min;
  int exp_area_max;
  double exp_area_var;
  double z_height;
  bool near_robot_filter;
  double near_robot_dist_sq;
  int max_balls;
  //-----------------------------
  
  
  
  
  
  LUT3D * _lut;
  PluginDetectBallsSettings * _settings; 
  bool _have_local_settings;
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
    PluginDetectBalls(FrameBuffer * _buffer, LUT3D * lut, const CameraParameters& camera_params, const RoboCupField& field, PluginDetectBallsSettings * _settings=0);

    ~PluginDetectBalls();

    virtual ProcessResult process(FrameData * data, RenderOptions * options);
    virtual VarList * getSettings();
    virtual string getName();
};

#endif

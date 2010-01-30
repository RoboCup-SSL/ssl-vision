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
  \file    plugin_detect_robots.h
  \brief   C++ Interface: plugin_detect_robots
  \author  Author Name, 2009
*/
//========================================================================
#ifndef PLUGIN_DETECT_ROBOTS_H
#define PLUGIN_DETECT_ROBOTS_H

#include <visionplugin.h>
#include "cmvision_region.h"
#include "messages_robocup_ssl_detection.pb.h"
#include "camera_calibration.h"
#include "field_filter.h"
#include "cmvision_histogram.h"
#include "cmpattern_teamdetector.h"
#include "cmpattern_team.h"
#include "vis_util.h"
#include "lut3d.h"
#include "VarNotifier.h"
/**
	@author Author Name
*/
class PluginDetectRobots : public VisionPlugin
{
protected:
  VarNotifier _notifier;
  LUT3D * _lut;
  VarList * _settings;

  VarString * _color_label;
  //TeamDetector * 
  int color_id_yellow;
  int color_id_blue;
  int color_id_clear;
  int color_id_ball;
  int color_id_black;
  int color_id_field;
  

  CMVision::RegionTree reg_tree;

  CMPattern::TeamSelector * global_team_selector_blue;
  CMPattern::TeamSelector * global_team_selector_yellow;

  CMPattern::TeamDetector * team_detector_blue;
  CMPattern::TeamDetector * team_detector_yellow;

  const CameraParameters& camera_parameters;
  const RoboCupField& field;

  void buildRegionTree(CMVision::ColorRegionList * colorlist);

protected slots:
    void teamDataChange();
public:
    PluginDetectRobots(FrameBuffer * _buffer, LUT3D * lut, const CameraParameters& camera_params, const RoboCupField& field, CMPattern::TeamSelector * _global_team_selector_blue, CMPattern::TeamSelector * _global_team_selector_yellow);

    ~PluginDetectRobots();

    virtual ProcessResult process(FrameData * data, RenderOptions * options);
    virtual VarList * getSettings();
    virtual string getName();
};

#endif

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
  \file    stack_robocup_ssl.h
  \brief   C++ Interface: stack_robocup_ssl
  \author  Author Name, 2008
*/
//========================================================================
#ifndef STACK_ROBOCUP_SSL_H
#define STACK_ROBOCUP_SSL_H

#include "visionstack.h"
#include "lut3d.h"
#include "camera_calibration.h"
#include "field.h"
#include "plugin_dvr.h"
#include "plugin_colorcalib.h"
#include "plugin_cameracalib.h"
#include "plugin_greyscale.h"
#ifdef APRILTAG
#include "plugin_apriltag.h"
#endif
#include "plugin_visualize.h"
#include "plugin_colorthreshold.h"
#include "plugin_runlength_encode.h"
#include "plugin_find_blobs.h"
#include "plugin_detect_balls.h"
#include "plugin_detect_robots.h"
#include "plugin_sslnetworkoutput.h"
#include "plugin_publishgeometry.h"
#include "plugin_legacysslnetworkoutput.h"
#include "plugin_legacypublishgeometry.h"
#include "plugin_auto_color_calibration.h"
#include "plugin_dvr.h"
#include "cmpattern_teamdetector.h"
#include "robocup_ssl_server.h"

using namespace std;

/*!
  \class   StackRoboCupSSL
  \brief   The single camera vision stack implementation used for the RoboCup SSL
  \author  Stefan Zickler, (C) 2008
           multiple of these stacks are run in parallel using the MultiStackRoboCupSSL
*/
class StackRoboCupSSL : public VisionStack {
  protected:
  int _camera_id;
  YUVLUT * lut_yuv;
  string _cam_settings_filename;
  CameraParameters* camera_parameters;
  RoboCupField * global_field;
  PluginDetectBallsSettings * global_ball_settings;
  CMPattern::TeamDetectorSettings * global_team_settings;
  CMPattern::TeamSelector * global_team_selector_blue;
  CMPattern::TeamSelector * global_team_selector_yellow;
  // UDP Server for Double-Sized field, new protobuf format.
  RoboCupSSLServer * _ds_udp_server_new;
  // UDP Server for Double-Sized field, old protobuf format.
  RoboCupSSLServer * _ds_udp_server_old;
  public:
  StackRoboCupSSL(RenderOptions* _opts,
                  FrameBuffer* _fb,
                  int camera_id,
                  RoboCupField* _global_field,
                  PluginDetectBallsSettings* _global_ball_settings,
                  PluginPublishGeometry* _global_plugin_publish_geometry,
                  PluginLegacyPublishGeometry* _legacy_plugin_publish_geometry,
                  CMPattern::TeamDetectorSettings* _global_team_settings,
                  CMPattern::TeamSelector* _global_team_selector_blue,
                  CMPattern::TeamSelector* _global_team_selector_yellow,
                  RoboCupSSLServer* ds_udp_server_new,
                  RoboCupSSLServer* ds_udp_server_old,
                  string cam_settings_filename);
  virtual string getSettingsFileName();
  virtual ~StackRoboCupSSL();
};


#endif

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
  \file    stack_robocup_ssl.cpp
  \brief   C++ Implementation: stack_robocup_ssl
  \author  Author Name, 2008
*/
//========================================================================
#include "stack_robocup_ssl.h"

StackRoboCupSSL::StackRoboCupSSL(
    RenderOptions * _opts,
    FrameBuffer * _fb,
    int camera_id,
    RoboCupField * _global_field,
    PluginDetectBallsSettings * _global_ball_settings,
    PluginPublishGeometry * _global_plugin_publish_geometry,
    PluginLegacyPublishGeometry * _legacy_plugin_publish_geometry,
    CMPattern::TeamDetectorSettings* _global_team_settings,
    CMPattern::TeamSelector * _global_team_selector_blue,
    CMPattern::TeamSelector * _global_team_selector_yellow,
    RoboCupSSLServer * ds_udp_server_new,
    RoboCupSSLServer * ds_udp_server_old,
    string cam_settings_filename) :
    VisionStack(_opts),
    _camera_id(camera_id),
    _cam_settings_filename(cam_settings_filename),
    global_field(_global_field),
    global_ball_settings(_global_ball_settings),
    global_team_settings(_global_team_settings),
    global_team_selector_blue(_global_team_selector_blue),
    global_team_selector_yellow(_global_team_selector_yellow),
    _ds_udp_server_new(ds_udp_server_new),
    _ds_udp_server_old(ds_udp_server_old) {
  (void)_fb;
  lut_yuv = new YUVLUT(4,6,6,cam_settings_filename + "-lut-yuv.xml");
  lut_yuv->loadRoboCupChannels(LUTChannelMode_Numeric);
  lut_yuv->addDerivedLUT(new RGBLUT(5,5,5,""));

  camera_parameters = new CameraParameters(_camera_id, global_field);

  _global_plugin_publish_geometry->addCameraParameters(camera_parameters);
  _legacy_plugin_publish_geometry->addCameraParameters(camera_parameters);

  stack.push_back(new PluginDVR(_fb));

  auto *pluginColorCalibration = new PluginColorCalibration(_fb, lut_yuv, LUTChannelMode_Numeric);
  stack.push_back(pluginColorCalibration);
  settings->addChild(lut_yuv->getSettings());

  stack.push_back(new PluginCameraCalibration(_fb,*camera_parameters, *global_field));

  stack.push_back(new PluginColorThreshold(_fb,lut_yuv));

  stack.push_back(new PluginGreyscale(_fb));

  stack.push_back(new PluginRunlengthEncode(_fb));

  stack.push_back(new PluginFindBlobs(_fb,lut_yuv));

  stack.push_back(new PluginDetectRobots(_fb,lut_yuv,*camera_parameters,*global_field,global_team_selector_blue,global_team_selector_yellow, global_team_settings));

  stack.push_back(new PluginDetectBalls(_fb,lut_yuv,*camera_parameters,*global_field,global_ball_settings));

  stack.push_back(new PluginAutoColorCalibration(_fb,lut_yuv, (LUTWidget*) pluginColorCalibration->getControlWidget()));

  stack.push_back(new PluginSSLNetworkOutput(
      _fb,
      _ds_udp_server_new,
      *camera_parameters,
      *global_field));

  stack.push_back(new PluginLegacySSLNetworkOutput(
      _fb,
      _ds_udp_server_old,
      *camera_parameters,
      *global_field));

  stack.push_back(_global_plugin_publish_geometry);
  stack.push_back(_legacy_plugin_publish_geometry);

  PluginVisualize * vis = new PluginVisualize(_fb,*camera_parameters,*global_field);
  vis->setThresholdingLUT(lut_yuv);
  stack.push_back(vis);
}
string StackRoboCupSSL::getSettingsFileName() {
  return _cam_settings_filename;
}
StackRoboCupSSL::~StackRoboCupSSL() {
  delete lut_yuv;
  delete camera_parameters;
}


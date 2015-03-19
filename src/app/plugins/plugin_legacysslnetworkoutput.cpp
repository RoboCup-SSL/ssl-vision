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
  \file    plugin_legacysslnetworkoutput.cpp
  \brief   C++ Implementation: plugin_legacysslnetworkoutput
  \author  Joydeep Biswas, 2014
*/
//========================================================================
#include "plugin_legacysslnetworkoutput.h"

PluginLegacySSLNetworkOutput::PluginLegacySSLNetworkOutput(
    FrameBuffer * _fb,
    RoboCupSSLServer * ss_udp_server1,
    RoboCupSSLServer * ss_udp_server2,
    RoboCupSSLServer * ds_udp_server_old,
    const CameraParameters& camera_params,
    const RoboCupField& field) : 
    VisionPlugin(_fb), 
    _camera_params(camera_params), 
    _field(field),
    _p_ss_udp_server(ss_udp_server1),
    _s_ss_udp_server(ss_udp_server2),
    _ds_udp_server_old(ds_udp_server_old) {}

PluginLegacySSLNetworkOutput::~PluginLegacySSLNetworkOutput() {}

bool PluginLegacySSLNetworkOutput::DoubleSizeToSingleSize(
    const bool primary_field,  
    const float x_in, 
    const float y_in, 
    const float a_in,
    float* x_out, 
    float* y_out, 
    float* a_out) {
  const float half_field_length = 
      primary_field ?
      -min(GetFieldLine("TopTouchLine", _field.field_lines)->
              p1_x->getDouble(),
           GetFieldLine("TopTouchLine", _field.field_lines)->
              p2_x->getDouble()) :
      max(GetFieldLine("TopTouchLine", _field.field_lines)->p1_x->getDouble(),
          GetFieldLine("TopTouchLine", _field.field_lines)->p2_x->getDouble());
  _field.field_length->getDouble();
  const float line_width = 
      GetFieldLine("HalfwayLine", _field.field_lines)->thickness->getDouble();
  // Ignore robots not within the referee margin minus robot radius
  if (primary_field) {
    if (x_in > 425.0 - 90.0) return false;
    *y_out = -(x_in + 0.5 * half_field_length - 0.5 * line_width);
  } else {
    if (x_in < -425.0 + 90.0) return false;
    *y_out = -(x_in - 0.5 * half_field_length + 0.5 * line_width);
  }
  *x_out = y_in;
  *a_out = angle_mod(a_in - RAD(90.0));
  return true;
}

bool PluginLegacySSLNetworkOutput::DoubleSizeToSingleSize(
    const bool primary_field, 
    const SSL_DetectionRobot& robot_in, 
    SSL_DetectionRobot* robot_out) {
  float x_out(0), y_out(0), angle_out(0);
  if (DoubleSizeToSingleSize(
      primary_field, robot_in.x(), robot_in.y(), robot_in.orientation(),
      &x_out, &y_out, &angle_out)) {
    *robot_out = robot_in;
    robot_out->set_x(x_out);
    robot_out->set_y(y_out);
    robot_out->set_orientation(angle_out);
    return true;
  }
  return false;
}

bool PluginLegacySSLNetworkOutput::DoubleSizeToSingleSize(
    const bool primary_field, 
    const SSL_DetectionBall& ball_in, 
    SSL_DetectionBall* ball_out) {
  float x_out(0), y_out(0), angle_out(0);
  if (DoubleSizeToSingleSize(
      primary_field, ball_in.x(), ball_in.y(), 0,
      &x_out, &y_out, &angle_out)) {
    *ball_out = ball_in;
    ball_out->set_x(x_out);
    ball_out->set_y(y_out);
    return true;
  }
  return false;
}


ProcessResult PluginLegacySSLNetworkOutput::process(
    FrameData * data, RenderOptions * options) {
  (void)options;
  if (data==0) return ProcessingFailed;

  SSL_DetectionFrame * detection_frame = 0;

  detection_frame=(SSL_DetectionFrame *)data->map.get("ssl_detection_frame");
  if (detection_frame != 0) {
    const bool is_p_ss_field_camera = 
        (detection_frame->camera_id() == 1 || 
        detection_frame->camera_id() == 2);
    const bool is_s_ss_field_camera = 
        (detection_frame->camera_id() == 0 || 
        detection_frame->camera_id() == 3);
    detection_frame->set_t_capture(data->time);
    detection_frame->set_frame_number(data->number);
    detection_frame->set_camera_id(data->cam_id);
    detection_frame->set_t_sent(GetTimeSec());
    // The double-sized field server uses the normal field coordinates.
    _ds_udp_server_old->sendLegacyMessage(*detection_frame);

    // Primary Single-Sized Field detections
    SSL_DetectionFrame p_ss_detection;
    // Secoondary Single-Sized Field detections
    SSL_DetectionFrame s_ss_detection;
    // Copy over the header fields for the detection frame.
    p_ss_detection.set_frame_number(detection_frame->frame_number());
    p_ss_detection.set_t_capture(detection_frame->t_capture());
    p_ss_detection.set_t_sent(detection_frame->t_sent());
    p_ss_detection.set_camera_id(detection_frame->camera_id());
    s_ss_detection.set_frame_number(detection_frame->frame_number());
    s_ss_detection.set_t_capture(detection_frame->t_capture());
    s_ss_detection.set_t_sent(detection_frame->t_sent());
    s_ss_detection.set_camera_id(detection_frame->camera_id());
    // Reused detection proto for robots on the primary single-sized field.
    SSL_DetectionRobot p_ss_robot;
    // Reused detection proto for robots on the secondary single-sized field.
    SSL_DetectionRobot s_ss_robot;
    // Process blue robots.
    for (size_t i = 0; i < detection_frame->robots_blue_size(); ++i) {
      const SSL_DetectionRobot& robot_in = detection_frame->robots_blue(i);
      if (is_p_ss_field_camera &&
          DoubleSizeToSingleSize(true, robot_in, &p_ss_robot)) {
        *p_ss_detection.add_robots_blue() = p_ss_robot;
      }
      if (is_s_ss_field_camera &&
          DoubleSizeToSingleSize(false, robot_in, &s_ss_robot)) {
        *s_ss_detection.add_robots_blue() = s_ss_robot;
      }
    }
    // Process yellow robots.
    for (size_t i = 0; i < detection_frame->robots_yellow_size(); ++i) {
      const SSL_DetectionRobot& robot_in = detection_frame->robots_yellow(i);
      if (is_p_ss_field_camera &&
          DoubleSizeToSingleSize(true, robot_in, &p_ss_robot)) {
        *p_ss_detection.add_robots_yellow() = p_ss_robot;
      }
      if (is_s_ss_field_camera &&
          DoubleSizeToSingleSize(false, robot_in, &s_ss_robot)) {
        *s_ss_detection.add_robots_yellow() = s_ss_robot;
      }
    }

    // Reused detection proto for balls on the primary single-sized field.
    SSL_DetectionBall p_ss_ball;
    // Reused detection proto for balls on the secondary single-sized field.
    SSL_DetectionBall s_ss_ball;
    for (size_t i = 0; i < detection_frame->balls_size(); ++i) {
      const SSL_DetectionBall& ball_in = detection_frame->balls(i);
      if (DoubleSizeToSingleSize(true, ball_in, &p_ss_ball)) {
        *p_ss_detection.add_balls() = p_ss_ball;
      }
      if (DoubleSizeToSingleSize(false, ball_in, &s_ss_ball)) {
        *s_ss_detection.add_balls() = s_ss_ball;
      }
    }

    // Publish the detection results.
    if (is_p_ss_field_camera) {
      const int cam_id = 
          (detection_frame->camera_id() == 1) ? 0 : 1;
      p_ss_detection.set_camera_id(cam_id);
      _p_ss_udp_server->sendLegacyMessage(p_ss_detection);
    }
    if (is_s_ss_field_camera) {
      const int cam_id = 
          (detection_frame->camera_id() == 0) ? 0 : 1;
      s_ss_detection.set_camera_id(cam_id);
      _s_ss_udp_server->sendLegacyMessage(s_ss_detection);
    }
  }
  return ProcessingOk;
}

string PluginLegacySSLNetworkOutput::getName() {
  return "Legacy Network Output";
}

PluginLegacySSLNetworkOutputSettings::PluginLegacySSLNetworkOutputSettings()
{
  settings = new VarList("Legacy Network Output");

  settings->addChild(
      multicast_address = new VarString("Multicast Address","224.5.23.2"));
  settings->addChild(p_ss_multicast_port = 
      new VarInt("Primary Single-Size Field Multicast Port",10002,1,65535));
  settings->addChild(s_ss_multicast_port = 
      new VarInt("Secondary Single-Size Field Multicast Port",10004,1,65535));
  settings->addChild(ds_multicast_port_old = 
      new VarInt("Legacy Double-Size Field Multicast Port",10005,1,65535));
  settings->addChild(
      multicast_interface = new VarString("Multicast Interface",""));
}
  
VarList * PluginLegacySSLNetworkOutputSettings::getSettings()
{
  return settings;
}

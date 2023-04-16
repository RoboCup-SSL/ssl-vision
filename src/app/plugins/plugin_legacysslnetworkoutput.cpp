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
    RoboCupSSLServer * ds_udp_server_old,
    const CameraParameters& camera_params,
    const RoboCupField& field) :
    VisionPlugin(_fb),
    _camera_params(camera_params),
    _field(field),
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

ProcessResult PluginLegacySSLNetworkOutput::process(
    FrameData * data, RenderOptions * options) {
  (void)options;
  if (data== nullptr) return ProcessingFailed;

  SSL_DetectionFrame * detection_frame;

  detection_frame=(SSL_DetectionFrame *)data->map.get("ssl_detection_frame");
  if (detection_frame != nullptr) {
    detection_frame->set_t_capture(data->time);
    if (data->time_cam > 0) {
      detection_frame->set_t_capture_camera(data->time_cam);
    } else {
      detection_frame->clear_t_capture_camera();
    }
    detection_frame->set_frame_number(data->number);
    detection_frame->set_camera_id(_camera_params.additional_calibration_information->camera_index->getInt());
    detection_frame->set_t_sent(GetTimeSec());
    // The double-sized field server uses the normal field coordinates.
    _ds_udp_server_old->sendLegacyMessage(*detection_frame);
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
  settings->addChild(ds_multicast_port_old =
      new VarInt("Legacy Double-Size Field Multicast Port",10005,1,65535));
  settings->addChild(
      multicast_interface = new VarString("Multicast Interface",""));
}

VarList * PluginLegacySSLNetworkOutputSettings::getSettings()
{
  return settings;
}

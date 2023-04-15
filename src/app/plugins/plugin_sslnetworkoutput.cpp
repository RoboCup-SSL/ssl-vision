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
  \file    plugin_sslnetworkoutput.cpp
  \brief   C++ Implementation: plugin_sslnetworkoutput
  \author  Author Name, 2009
*/
//========================================================================
#include "plugin_sslnetworkoutput.h"

PluginSSLNetworkOutput::PluginSSLNetworkOutput(FrameBuffer * _fb, RoboCupSSLServer * udp_server, const CameraParameters& camera_params, const RoboCupField& field)
 : VisionPlugin(_fb), _camera_params(camera_params), _field(field)
{
  _udp_server=udp_server;
}

PluginSSLNetworkOutput::~PluginSSLNetworkOutput()
{

}


ProcessResult PluginSSLNetworkOutput::process(FrameData * data, RenderOptions * options)
{
  (void)options;
  if (data == nullptr) return ProcessingFailed;

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
    _udp_server->send(*detection_frame);
  }
  return ProcessingOk;
}

string PluginSSLNetworkOutput::getName() {
  return "Network Output";
}

PluginSSLNetworkOutputSettings::PluginSSLNetworkOutputSettings()
{
  settings = new VarList("Network Output");

  settings->addChild(multicast_address = new VarString("Multicast Address","224.5.23.2"));
  settings->addChild(multicast_port =
      new VarInt("Multicast Port",10006,1,65535));
  settings->addChild(multicast_interface = new VarString("Multicast Interface",""));
}

VarList * PluginSSLNetworkOutputSettings::getSettings()
{
  return settings;
}

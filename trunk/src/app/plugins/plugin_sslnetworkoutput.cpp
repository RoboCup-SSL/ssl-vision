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
  if (data==0) return ProcessingFailed;

  SSL_DetectionFrame * detection_frame = 0;

  printf("SENDING\n");

  detection_frame=(SSL_DetectionFrame *)data->map.get("ssl_detection_frame");
  if (detection_frame != 0) {
    detection_frame->set_timestamp(data->time);
    detection_frame->set_frame_number(data->number);
    detection_frame->set_camera_id(data->cam_id);
    _udp_server->send(*detection_frame);
  }
  return ProcessingOk;
}

string PluginSSLNetworkOutput::getName() {
  return "Network Output";
}

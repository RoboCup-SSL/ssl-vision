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
  \file    plugin_legacysslnetworkoutput.h
  \brief   C++ Interface: plugin_legacysslnetworkoutput
  \author  Joydeep Biswas, 2014
*/
//========================================================================
#ifndef PLUGIN_LEGACYSSLNETWORKOUTPUT_H
#define PLUGIN_LEGACYSSLNETWORKOUTPUT_H

#include <visionplugin.h>
#include "plugin_legacypublishgeometry.h"
#include "robocup_ssl_server.h"
#include "camera_calibration.h"
#include "field.h"
#include "timer.h"

class PluginLegacySSLNetworkOutput : public VisionPlugin
{
protected:
 const CameraParameters& _camera_params;
 const RoboCupField& _field;
 // UDP Server for Double-Sized field, old protobuf format.
 RoboCupSSLServer * _ds_udp_server_old;

public:
  PluginLegacySSLNetworkOutput(FrameBuffer * _fb,
                               RoboCupSSLServer * ds_udp_server_old,
                               const CameraParameters& camera_params,
                               const RoboCupField& field);

  ~PluginLegacySSLNetworkOutput();

  virtual ProcessResult process(FrameData * data, RenderOptions * options);
  virtual string getName();
  bool DoubleSizeToSingleSize(
    const bool primary_field,
    const float x_in,
    const float y_in,
    const float a_in,
    float* x_out,
    float* y_out,
    float* a_out);
};

class PluginLegacySSLNetworkOutputSettings {
public:
  VarList * settings;
  VarString * multicast_address;
  // UDP port for Double-Sized field, old protobuf format.
  VarInt * ds_multicast_port_old;
  VarString * multicast_interface;

  PluginLegacySSLNetworkOutputSettings();
  VarList * getSettings();
};

#endif

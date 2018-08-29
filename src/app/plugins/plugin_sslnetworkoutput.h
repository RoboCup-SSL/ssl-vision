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
  \file    plugin_sslnetworkoutput.h
  \brief   C++ Interface: plugin_sslnetworkoutput
  \author  Stefan Zickler, 2009
*/
//========================================================================
#ifndef PLUGIN_SSLNETWORKOUTPUT_H
#define PLUGIN_SSLNETWORKOUTPUT_H

#include <visionplugin.h>
#include "robocup_ssl_server.h"
#include "camera_calibration.h"
#include "field.h"
#include "timer.h"

/**
	@author Stefan Zickler
*/
class PluginSSLNetworkOutput : public VisionPlugin
{
protected:
 const CameraParameters& _camera_params;
 const RoboCupField& _field;
 RoboCupSSLServer * _udp_server;
public:
    PluginSSLNetworkOutput(FrameBuffer * _fb, RoboCupSSLServer * udp_server, const CameraParameters& camera_params, const RoboCupField& field);

    ~PluginSSLNetworkOutput();

    virtual ProcessResult process(FrameData * data, RenderOptions * options);
    virtual string getName();
};

class PluginSSLNetworkOutputSettings {
public:
  VarList * settings;
  VarString * multicast_address;
  VarInt * multicast_port;
  VarString * multicast_interface;

  PluginSSLNetworkOutputSettings();
  VarList * getSettings();
};

#endif

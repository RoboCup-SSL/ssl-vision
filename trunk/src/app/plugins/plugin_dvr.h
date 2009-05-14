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
  \file    plugin_dvr.h
  \brief   C++ Interface: plugin_dvr
  \author  Stefan Zickler, 2009
*/
//========================================================================
#ifndef plugin_dvr_H
#define plugin_dvr_H

#include <visionplugin.h>
#include "robocup_ssl_server.h"
#include "camera_calibration.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "VarTypes.h"

/**
	@author Stefan Zickler
*/
class PluginDVR : public VisionPlugin
{
Q_OBJECT
protected:
  VarList * _settings;
public:
    PluginDVR(FrameBuffer * fb);
    virtual VarList * getSettings();
    virtual ~PluginDVR();
    virtual string getName();
    virtual ProcessResult process(FrameData * data, RenderOptions * options);
};

#endif

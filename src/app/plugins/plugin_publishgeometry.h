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
  \file    plugin_publishgeometry.h
  \brief   C++ Interface: plugin_publishgeometry
  \author  Author Name, 2009
*/
//========================================================================
#ifndef PLUGIN_PUBLISHGEOMETRY_H
#define PLUGIN_PUBLISHGEOMETRY_H

#include <visionplugin.h>
#include "robocup_ssl_server.h"
#include "camera_calibration.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "VarTypes.h"

/**
	@author Author Name
*/
class PluginPublishGeometry : public VisionPlugin
{
Q_OBJECT
protected:
  RoboCupSSLServer * _server;
  const RoboCupField & _field;
  vector<CameraParameters *> params;
  VarList * _settings;
  VarTrigger * _pub;
  VarBool * _pub_auto_enable;
  VarDouble * _pub_auto_interval;
  VarList * _pub_auto;
  QMutex mutex;
  void sendGeometry();
  double last_t;
protected slots:
  void slotPublishTriggered();
public:
    PluginPublishGeometry(FrameBuffer * fb, RoboCupSSLServer * server, const RoboCupField & field);
    void addCameraParameters(CameraParameters * param);
    virtual VarList * getSettings();
    virtual ~PluginPublishGeometry();

    virtual string getName();
    virtual ProcessResult process(FrameData * data, RenderOptions * options);
};

#endif

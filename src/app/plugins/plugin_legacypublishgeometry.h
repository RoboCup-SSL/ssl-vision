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
  \file    plugin_legacypublishgeometry.h
  \brief   C++ Interface: plugin_legacypublishgeometry
  \author  Joydeep Biswas, 2014
*/
//========================================================================
#ifndef PLUGIN_LEGACYPUBLISHGEOMETRY_H
#define PLUGIN_LEGACYPUBLISHGEOMETRY_H

#include <visionplugin.h>
#include "robocup_ssl_server.h"
#include "camera_calibration.h"
#include "messages_robocup_ssl_geometry_legacy.pb.h"
#include "VarTypes.h"

const FieldLine* GetFieldLine(
    const string& line_name,
    const vector<FieldLine*>& field_lines);

const FieldCircularArc* GetFieldCircularArc(
    const string& arc_name,
    const vector<FieldCircularArc*>& field_arcs);

class PluginLegacyPublishGeometry : public VisionPlugin
{
Q_OBJECT
protected:
  // UDP Server for Double-Sized field, old protobuf format.
  RoboCupSSLServer * _ds_udp_server_old;
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
  PluginLegacyPublishGeometry(FrameBuffer* fb,
                              RoboCupSSLServer* ds_udp_server_old,
                              const RoboCupField& field);
  void addCameraParameters(CameraParameters * param);
  virtual VarList * getSettings();
  virtual ~PluginLegacyPublishGeometry();

  virtual string getName();
  virtual ProcessResult process(FrameData * data, RenderOptions * options);
};

#endif

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
  \file    plugin_publishgeometry.cpp
  \brief   C++ Implementation: plugin_publishgeometry
  \author  Author Name, 2009
*/
//========================================================================
#include "plugin_publishgeometry.h"

PluginPublishGeometry::PluginPublishGeometry(FrameBuffer * fb, RoboCupSSLServer * server, const RoboCupField & field)
 : VisionPlugin(fb), _field(field)
{
  _server=server;
  setSharedAmongStacks(true);
  _settings=new VarList("Publish Geometry");
  _settings->addChild(_pub=new VarTrigger("Publish","Publish!"));
  _settings->addChild(_pub_auto=new VarList("Auto Publish"));
  _pub_auto->addChild(_pub_auto_enable=new VarBool("Enable",true));
  _pub_auto->addChild(_pub_auto_interval=new VarDouble("Interval (seconds)",3.0));
  last_t=0;
  connect(_pub,SIGNAL(signalTriggered()),this,SLOT(slotPublishTriggered()));
}

void PluginPublishGeometry::addCameraParameters(CameraParameters * param) {
  lock();
  params.push_back(param);
  unlock();
}

PluginPublishGeometry::~PluginPublishGeometry()
{
  delete _settings;
  delete _pub;
}

VarList * PluginPublishGeometry::getSettings() {
  return _settings;
}

string PluginPublishGeometry::getName() {
  return "Publish Geometry";
}

void PluginPublishGeometry::sendGeometry() {
  SSL_GeometryData geodata;
  _field.toProtoBuffer(*geodata.mutable_field());
  _field.toProtoBuffer(*geodata.mutable_models());
  for (unsigned int i = 0; i < params.size(); i++) {
    int camId = params[i]->additional_calibration_information->camera_index->get();
    if(camId < 0 || camId >= _field.num_cameras_total->get()) {
      continue;
    }
    SSL_GeometryCameraCalibration * calib = geodata.add_calib();
    params[i]->toProtoBuffer(*calib);
  }
  _server->send(geodata);
}

void PluginPublishGeometry::slotPublishTriggered() {
  lock();
    sendGeometry();
  unlock();
}

ProcessResult PluginPublishGeometry::process(FrameData * data, RenderOptions * options) {
  (void)data;
  (void)options;
  //TODO: check client requests in server process
  //      if requested, call sendGeometry();
  if (_pub_auto_enable->getBool()==true) {
    double t = data->time - last_t;
    if (t > _pub_auto_interval->getDouble()) {
      sendGeometry();
      last_t=data->time;
    }
  }
  return ProcessingOk;
}

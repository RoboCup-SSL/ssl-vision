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
  \file    plugin_legacypublishgeometry.cpp
  \brief   C++ Implementation: plugin_legacypublishgeometry
  \author  Joydeep Biswas, 2014
*/
//========================================================================
#include "plugin_legacypublishgeometry.h"

const FieldLine* GetFieldLine(
    const string& line_name, 
    const vector<FieldLine*>& field_lines) {
  for (size_t i = 0; i < field_lines.size(); ++i) {
    if (line_name.compare(field_lines[i]->name->getString()) == 0) {
      return (field_lines[i]);
    }
  }
  fprintf(stderr, "ERROR: Field Line \"%s\" not found!\n", line_name.c_str());
  return NULL;
}

const FieldCircularArc* GetFieldCircularArc(
    const string& arc_name, 
    const vector<FieldCircularArc*>& field_arcs) {
  for (size_t i = 0; i < field_arcs.size(); ++i) {
    if (arc_name.compare(field_arcs[i]->name->getString()) == 0) {
      return (field_arcs[i]);
    }
  }
  fprintf(stderr, "ERROR: Field Arc \"%s\" not found!\n", arc_name.c_str());
  return NULL;
}

PluginLegacyPublishGeometry::PluginLegacyPublishGeometry(
    FrameBuffer* fb, 
    RoboCupSSLServer* p_ss_udp_server, 
    RoboCupSSLServer* s_ss_udp_server, 
    RoboCupSSLServer* ds_udp_server_old, 
    const RoboCupField& field) : 
    _p_ss_udp_server(p_ss_udp_server),
    _s_ss_udp_server(s_ss_udp_server),
    _ds_udp_server_old(ds_udp_server_old),
    VisionPlugin(fb), 
    _field(field) {
  setSharedAmongStacks(true);
  _settings=new VarList("Publish Legacy Geometry");
  _settings->addChild(_pub=new VarTrigger("Publish","Publish!"));
  _settings->addChild(_pub_auto=new VarList("Auto Publish"));
  _pub_auto->addChild(_pub_auto_enable=new VarBool("Enable",true));
  _pub_auto->addChild(_pub_auto_interval=new VarDouble("Interval (seconds)",3.0));
  last_t=0;
  connect(_pub,SIGNAL(signalTriggered()),this,SLOT(slotPublishTriggered()));
}

void PluginLegacyPublishGeometry::addCameraParameters(CameraParameters * param) {
  lock();
  params.push_back(param);
  unlock();
}

PluginLegacyPublishGeometry::~PluginLegacyPublishGeometry() {
  delete _settings;
  delete _pub;
}

VarList * PluginLegacyPublishGeometry::getSettings() {
  return _settings;
}

string PluginLegacyPublishGeometry::getName() {
  return "Publish Geometry";
}

void PluginLegacyPublishGeometry::sendGeometry() {
  // NOTE: The field dimensions are derived from one half irrespective of
  // the exact field half / end in question.
  SSL_GeometryFieldSize field;
  _field.toProtoBuffer(field);

  // Geometry data for double-sized field in old format.
  RoboCup2014Legacy::Geometry::SSL_GeometryData ds_geodata_old;
  // Field data for double-sized field in old format.
  RoboCup2014Legacy::Geometry::SSL_GeometryFieldSize ds_field_old;

  // Geometry data for primary single-sized field in old format.
  RoboCup2014Legacy::Geometry::SSL_GeometryData p_ss_geodata;
  // Field data for primary single-sized field in old format.
  RoboCup2014Legacy::Geometry::SSL_GeometryFieldSize p_ss_field;

  // Geometry data for primary single-sized field in old format.
  RoboCup2014Legacy::Geometry::SSL_GeometryData s_ss_geodata;
  // Field data for primary single-sized field in old format.
  RoboCup2014Legacy::Geometry::SSL_GeometryFieldSize s_ss_field;

  ds_field_old.set_field_length(field.field_length());
  ds_field_old.set_field_width(field.field_width());
  ds_field_old.set_goal_width(field.goal_width());
  ds_field_old.set_goal_depth(field.goal_depth());
  ds_field_old.set_boundary_width(field.boundary_width());
  ds_field_old.set_center_circle_radius(
      GetFieldCircularArcRadius("CenterCircle"));
  ds_field_old.set_defense_radius(
      GetFieldCircularArcRadius("LeftFieldLeftPenaltyArc"));
  ds_field_old.set_defense_stretch(GetFieldLineLength("LeftPenaltyStretch"));
  // The following fields no longer exist in ssl-vision, and are hard-coded
  // from the ssl-vision SVN trunk, r233.
  ds_field_old.set_line_width(10);
  ds_field_old.set_referee_width(425);
  ds_field_old.set_goal_wall_width(20);
  ds_field_old.set_free_kick_from_defense_dist(200);
  ds_field_old.set_penalty_spot_from_field_line_dist(750);
  ds_field_old.set_penalty_line_from_spot_dist(400);
  *ds_geodata_old.mutable_field() = ds_field_old;

  p_ss_field.set_field_length(field.field_width());
  p_ss_field.set_field_width(0.5 * field.field_length() + 
      0.5 * GetFieldLineThickness("HalfwayLine"));
  p_ss_field.set_boundary_width(field.boundary_width());
  p_ss_field.set_center_circle_radius(
      GetFieldCircularArcRadius("PrimarySingleSize_CenterCircle"));
  p_ss_field.set_defense_radius(
      GetFieldCircularArcRadius("PrimarySingleSize_LeftFieldLeftPenaltyArc"));
  p_ss_field.set_defense_stretch(
      GetFieldLineLength("PrimarySingleSize_LeftPenaltyStretch"));
  p_ss_field.set_goal_depth(field.goal_depth());
  // The following fields no longer exist in ssl-vision, and are hard-coded
  // from the ssl-vision SVN trunk, r233.
  p_ss_field.set_line_width(10);
  p_ss_field.set_referee_width(425);
  p_ss_field.set_goal_width(700);
  p_ss_field.set_goal_wall_width(20);
  p_ss_field.set_free_kick_from_defense_dist(200);
  p_ss_field.set_penalty_spot_from_field_line_dist(750);
  p_ss_field.set_penalty_line_from_spot_dist(400);
  *p_ss_geodata.mutable_field() = p_ss_field;

  s_ss_field.set_field_length(field.field_width());
  s_ss_field.set_field_width(0.5 * field.field_length() +
      0.5 * GetFieldLineThickness("HalfwayLine"));
  s_ss_field.set_boundary_width(field.boundary_width());
  s_ss_field.set_center_circle_radius(
      GetFieldCircularArcRadius("SecondarySingleSize_CenterCircle"));
  s_ss_field.set_defense_radius(
      GetFieldCircularArcRadius("SecondarySingleSize_LeftFieldLeftPenaltyArc"));
  s_ss_field.set_defense_stretch(
      GetFieldLineLength("SecondarySingleSize_LeftPenaltyStretch"));
  s_ss_field.set_goal_depth(field.goal_depth());
  // The following fields no longer exist in ssl-vision, and are hard-coded
  // from the ssl-vision SVN trunk, r233.
  s_ss_field.set_line_width(10);
  s_ss_field.set_referee_width(425);
  s_ss_field.set_goal_width(700);
  s_ss_field.set_goal_wall_width(20);
  s_ss_field.set_free_kick_from_defense_dist(200);
  s_ss_field.set_penalty_spot_from_field_line_dist(750);
  s_ss_field.set_penalty_line_from_spot_dist(400);
  *s_ss_geodata.mutable_field() = s_ss_field;

  // Copy over the camera calibrations.
  for (unsigned int i = 0; i < params.size(); i++) {
    SSL_GeometryCameraCalibration* ds_calib_old = ds_geodata_old.add_calib();
    SSL_GeometryCameraCalibration* p_ss_calib = p_ss_geodata.add_calib();
    SSL_GeometryCameraCalibration* s_ss_calib = s_ss_geodata.add_calib();
    params[i]->toProtoBuffer(*ds_calib_old,i);
    params[i]->toProtoBuffer(*p_ss_calib,i);
    params[i]->toProtoBuffer(*s_ss_calib,i);
    DoubleToSingleFieldCameraCalib(true, p_ss_calib);
    DoubleToSingleFieldCameraCalib(false, s_ss_calib);
  }

  _ds_udp_server_old->sendLegacyMessage(ds_geodata_old);
  _p_ss_udp_server->sendLegacyMessage(p_ss_geodata);
  _s_ss_udp_server->sendLegacyMessage(s_ss_geodata);
}

void PluginLegacyPublishGeometry::DoubleToSingleFieldCameraCalib(
    bool primary_field,
    SSL_GeometryCameraCalibration* calib) {
  const float y_offset =
      (primary_field ? 1 : -1) *
      (0.25 * _field.field_length->getDouble() -
          0.5 * GetFieldLineThickness("HalfwayLine"));

  const float qx = calib->q1();
  const float qy = -calib->q0();
  calib->set_q0(qx);
  calib->set_q1(qy);

  const float tx = calib->ty();
  const float ty = -calib->tx() + y_offset;
  calib->set_tx(tx);
  calib->set_ty(ty);

  float derived_tx = calib->derived_camera_world_tx();
  float derived_ty = calib->derived_camera_world_ty() + y_offset;
  calib->set_derived_camera_world_tx(derived_tx);
  calib->set_derived_camera_world_ty(derived_ty);
}

float PluginLegacyPublishGeometry::GetFieldLineLength(const string& line_name) {
  const FieldLine* line = GetFieldLine(line_name, _field.field_lines);
  if (line == NULL) return 0;
  return (sqrt(sq(line->p1_x - line->p2_x) + sq(line->p1_y - line->p2_y)));
}

float PluginLegacyPublishGeometry::GetFieldLineThickness(
    const string& line_name) {
  const FieldLine* line = GetFieldLine(line_name, _field.field_lines);
  if (line == NULL) return 0;
  return (line->thickness->getDouble());
}

float PluginLegacyPublishGeometry::GetFieldCircularArcRadius(
    const string& arc_name) {
  const FieldCircularArc* arc = 
      GetFieldCircularArc(arc_name, _field.field_arcs);
  if (arc == NULL) return 0;
  return (arc->radius->getDouble() + 0.5 * arc->thickness->getDouble());
}

void PluginLegacyPublishGeometry::slotPublishTriggered() {
  lock();
  sendGeometry();
  unlock();
}

ProcessResult PluginLegacyPublishGeometry::process(FrameData * data, RenderOptions * options) {
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

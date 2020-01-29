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
 \file    field.cpp
 \brief   Field markings management
 \author  Joydeep Biswas, (C) 2013
 */
//========================================================================

#include "field.h"
#include "field_default_constants.h"

FieldLine* FieldLine::FromVarList(VarList* list) {
  vector<VarType*> list_entries = list->getChildren();
  if (list_entries.size() != static_cast<size_t>(7)) {
    // The list should have exactly the expected number of entries.
    return NULL;
  }
  VarString* name = NULL;
  VarStringEnum* type = NULL;
  VarDouble* p1_x = NULL;
  VarDouble* p1_y = NULL;
  VarDouble* p2_x = NULL;
  VarDouble* p2_y = NULL;
  VarDouble* thickness = NULL;
  for (size_t i = 0; i < list_entries.size(); ++i) {
    VarType* entry = list_entries[i];
    if (entry->getName().compare("Name") == 0) {
      name = reinterpret_cast<VarString*>(entry);
    } else if (entry->getName().compare("Type") == 0) {
      type = reinterpret_cast<VarStringEnum*>(entry);
    } else if (entry->getName().compare("P1.x") == 0) {
      p1_x = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("P1.y") == 0) {
      p1_y = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("P2.x") == 0) {
      p2_x = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("P2.y") == 0) {
      p2_y = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("Line thickness") == 0) {
      thickness = reinterpret_cast<VarDouble*>(entry);
    } else {
      // Unexpected entry encountered: fail.
      return NULL;
    }
  }
  return (new FieldLine(name, type, p1_x, p1_y, p2_x, p2_y, thickness, list));
}

FieldLine::FieldLine(
    VarString* name_,
    VarStringEnum* type_,
    VarDouble* p1_x_,
    VarDouble* p1_y_,
    VarDouble* p2_x_,
    VarDouble* p2_y_,
    VarDouble* thickness_,
    VarList* list_): QObject(),
    name(name_), type(type_), p1_x(p1_x_), p1_y(p1_y_), p2_x(p2_x_), p2_y(p2_y_),
    thickness(thickness_), list(list_) {
  connect(name, SIGNAL(wasEdited(VarType*)),
          this, SLOT(Rename()));
}

FieldLine::FieldLine(const FieldLine& other) :
    name(new VarString("Name", other.name->getString())),
    type(new VarStringEnum("Type", other.type->getName())),
    p1_x(new VarDouble("P1.x", other.p1_x->getDouble())),
    p1_y(new VarDouble("P1.y", other.p1_y->getDouble())),
    p2_x(new VarDouble("P2.x", other.p2_x->getDouble())),
    p2_y(new VarDouble("P2.y", other.p2_y->getDouble())),
    thickness(new VarDouble("Line thickness", other.thickness->getDouble())),
    list(new VarList(other.name->getString())) {
  list->addChild(name);
  list->addChild(type);
  list->addChild(p1_x);
  list->addChild(p1_y);
  list->addChild(p2_x);
  list->addChild(p2_y);
  list->addChild(thickness);
  fillTypeEnum();
  connect(name, SIGNAL(wasEdited(VarType*)), this, SLOT(Rename()));
}

void FieldLine::fillTypeEnum() const {
    type->addItem("Undefined");
    type->addItem("TopTouchLine");
    type->addItem("BottomTouchLine");
    type->addItem("LeftGoalLine");
    type->addItem("RightGoalLine");
    type->addItem("HalfwayLine");
    type->addItem("CenterLine");
    type->addItem("LeftPenaltyStretch");
    type->addItem("RightPenaltyStretch");
    type->addItem("LeftFieldLeftPenaltyStretch");
    type->addItem("LeftFieldRightPenaltyStretch");
    type->addItem("RightFieldLeftPenaltyStretch");
    type->addItem("RightFieldRightPenaltyStretch");
}

FieldLine::FieldLine(const string& marking_name) :
    name(new VarString("Name", marking_name)),
    type(new VarStringEnum("Type", "Undefined")),
    p1_x(new VarDouble("P1.x")),
    p1_y(new VarDouble("P1.y")),
    p2_x(new VarDouble("P2.x")),
    p2_y(new VarDouble("P2.y")),
    thickness(new VarDouble("Line thickness", 10)),
    list(new VarList(marking_name)) {
  list->addChild(name);
  list->addChild(type);
  list->addChild(p1_x);
  list->addChild(p1_y);
  list->addChild(p2_x);
  list->addChild(p2_y);
  list->addChild(thickness);
  fillTypeEnum();
  connect(name, SIGNAL(wasEdited(VarType*)), this, SLOT(Rename()));
}

FieldLine::FieldLine(
    const string& marking_name,
    double p1_x_,
    double p1_y_,
    double p2_x_,
    double p2_y_,
    double thickness_) :
    name(new VarString("Name", marking_name)),
    type(new VarStringEnum("Type", marking_name)),
    p1_x(new VarDouble("P1.x", p1_x_)),
    p1_y(new VarDouble("P1.y", p1_y_)),
    p2_x(new VarDouble("P2.x", p2_x_)),
    p2_y(new VarDouble("P2.y", p2_y_)),
    thickness(new VarDouble("Line thickness", thickness_)),
    list(new VarList(marking_name)) {
  list->addChild(name);
  list->addChild(type);
  list->addChild(p1_x);
  list->addChild(p1_y);
  list->addChild(p2_x);
  list->addChild(p2_y);
  list->addChild(thickness);
  fillTypeEnum();
  connect(name, SIGNAL(wasEdited(VarType*)), this, SLOT(Rename()));
}

void FieldLine::Rename() {
  list->setName(name->getString());
}

FieldLine::~FieldLine() {
  list->removeChild(thickness);
  list->removeChild(p2_x);
  list->removeChild(p2_y);
  list->removeChild(p1_x);
  list->removeChild(p1_y);
  list->removeChild(name);
  list->removeChild(type);
  delete name;
  delete type;
  delete p1_x;
  delete p1_y;
  delete p2_x;
  delete p2_y;
  delete thickness;
  delete list;
}

FieldCircularArc* FieldCircularArc::FromVarList(VarList* list) {
  vector<VarType*> list_entries = list->getChildren();
  if (list_entries.size() != static_cast<size_t>(8)) {
      // The list should have exactly the expected number of entries.
    return NULL;
  }
  VarString* name = NULL;
  VarStringEnum* type = NULL;
  VarDouble* center_x = NULL;
  VarDouble* center_y = NULL;
  VarDouble* radius = NULL;
  VarDouble* a1 = NULL;
  VarDouble* a2 = NULL;
  VarDouble* thickness = NULL;
  for (size_t i = 0; i < list_entries.size(); ++i) {
    VarType* entry = list_entries[i];
    if (entry->getName().compare("Name") == 0) {
      name = reinterpret_cast<VarString*>(entry);
    } else if (entry->getName().compare("Type") == 0) {
      type = reinterpret_cast<VarStringEnum*>(entry);
    } else if (entry->getName().compare("Center.x") == 0) {
      center_x = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("Center.y") == 0) {
      center_y = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("Radius") == 0) {
      radius = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("Start angle") == 0) {
      a1 = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("End angle") == 0) {
      a2 = reinterpret_cast<VarDouble*>(entry);
    } else if (entry->getName().compare("Line thickness") == 0) {
      thickness = reinterpret_cast<VarDouble*>(entry);
    } else {
      // Unexpected entry encountered: fail.
      return NULL;
    }
  }
  FieldCircularArc* new_arc = new FieldCircularArc(
      name, type, center_x, center_y, radius, a1, a2, thickness, list);
  return (new_arc);
}

FieldCircularArc::FieldCircularArc(
    VarString* name_, VarStringEnum* type_, VarDouble* center_x_, VarDouble* center_y_,
    VarDouble* radius_, VarDouble* a1_, VarDouble* a2_, VarDouble* thickness_,
    VarList* list_): QObject(),
    name(name_), type(type_), center_x(center_x_), center_y(center_y_), radius(radius_),
    a1(a1_), a2(a2_), thickness(thickness_), list(list_) {
  fillTypeEnum();
  connect(name, SIGNAL(wasEdited(VarType*)), this, SLOT(Rename()));
}

FieldCircularArc::FieldCircularArc(const FieldCircularArc& other) :
    name(new VarString("Name", other.name->getString())),
    type(new VarStringEnum("Type", other.type->getString())),
    center_x(new VarDouble("Center.x", other.center_x->getDouble())),
    center_y(new VarDouble("Center.y", other.center_y->getDouble())),
    radius(new VarDouble("Radius", other.radius->getDouble())),
    a1(new VarDouble("Start angle", other.a1->getDouble())),
    a2(new VarDouble("End angle", other.a2->getDouble())),
    thickness(new VarDouble("Line thickness", other.thickness->getDouble())),
    list(new VarList(other.name->getString())) {
  list->addChild(name);
  list->addChild(type);
  list->addChild(center_x);
  list->addChild(center_y);
  list->addChild(radius);
  list->addChild(a1);
  list->addChild(a2);
  list->addChild(thickness);
  fillTypeEnum();
  connect(name, SIGNAL(wasEdited(VarType*)), this, SLOT(Rename()));
}

FieldCircularArc::FieldCircularArc(
    const string& marking_name,
    double center_x_, double center_y_,
    double radius_, double a1_, double a2_, double thickness_) :
    name(new VarString("Name", marking_name)),
    type(new VarStringEnum("Type", marking_name)),
    center_x(new VarDouble("Center.x", center_x_)),
    center_y(new VarDouble("Center.y", center_y_)),
    radius(new VarDouble("Radius", radius_)),
    a1(new VarDouble("Start angle", a1_)),
    a2(new VarDouble("End angle", a2_)),
    thickness(new VarDouble("Line thickness", thickness_)),
    list(new VarList(marking_name)) {
  list->addChild(name);
  list->addChild(type);
  list->addChild(center_x);
  list->addChild(center_y);
  list->addChild(radius);
  list->addChild(a1);
  list->addChild(a2);
  list->addChild(thickness);
  fillTypeEnum();
  connect(name, SIGNAL(wasEdited(VarType*)), this, SLOT(Rename()));
}

FieldCircularArc::FieldCircularArc(const string& marking_name) :
    name(new VarString("Name", marking_name)),
    type(new VarStringEnum("type", "Undefined")),
    center_x(new VarDouble("Center.x")),
    center_y(new VarDouble("Center.y")),
    radius(new VarDouble("Radius")),
    a1(new VarDouble("Start angle")),
    a2(new VarDouble("End angle")),
    thickness(new VarDouble("Line thickness", 10)),
    list(new VarList(marking_name)) {
  list->addChild(name);
  list->addChild(type);
  list->addChild(center_x);
  list->addChild(center_y);
  list->addChild(radius);
  list->addChild(a1);
  list->addChild(a2);
  list->addChild(thickness);
  fillTypeEnum();
  connect(name, SIGNAL(wasEdited(VarType*)), this, SLOT(Rename()));
}

FieldCircularArc::~FieldCircularArc() {
  list->removeChild(thickness);
  list->removeChild(type);
  list->removeChild(a2);
  list->removeChild(a1);
  list->removeChild(radius);
  list->removeChild(center_y);
  list->removeChild(center_x);
  list->removeChild(name);
  delete name;
  delete type;
  delete center_x;
  delete center_y;
  delete radius;
  delete a1;
  delete a2;
  delete thickness;
  delete list;
}

void FieldCircularArc::fillTypeEnum() const {
  type->addItem("Undefined");
  type->addItem("CenterCircle");
}

void FieldCircularArc::Rename() {
  list->setName(name->getString());
}

RoboCupField::RoboCupField() {
  settings = new VarList("Field Configuration");
  field_length = new VarDouble(
      "Field Length", FieldConstantsRoboCup2018A::kFieldLength);
  field_width = new VarDouble(
      "Field Width", FieldConstantsRoboCup2018A::kFieldWidth);
  goal_width = new VarDouble(
      "Goal Width", FieldConstantsRoboCup2018A::kGoalWidth);
  goal_depth = new VarDouble(
      "Goal Depth", FieldConstantsRoboCup2018A::kGoalDepth);
  boundary_width = new VarDouble(
      "Boundary Width", FieldConstantsRoboCup2018A::kBoundaryWidth);
  line_thickness = new VarDouble(
      "Line Thickness", FieldConstantsRoboCup2018A::kLineThickness);
  penalty_area_depth = new VarDouble(
       "Penalty Area Depth", FieldConstantsRoboCup2018A::kPenaltyAreaDepth);
  penalty_area_width = new VarDouble(
       "Penalty Area Width", FieldConstantsRoboCup2018A::kPenaltyAreaWidth);
  num_cameras_total = new VarInt(
       "Total Number of Cameras", FieldConstantsRoboCup2018A::kNumCamerasTotal);
  num_cameras_local = new VarInt(
       "Local Number of Cameras", FieldConstantsRoboCup2018A::kNumCamerasLocal);

  updateShapes = new VarTrigger("Field Lines/Arcs","Use Defaults");
  var_num_lines = new VarInt("Number of Line Segments", 0);
  var_num_arcs = new VarInt("Number of Arcs", 0);
  field_lines_list = new VarList("Field Lines");
  field_arcs_list = new VarList("Field Arcs");

  settings->addChild(field_length);
  settings->addChild(field_width);
  settings->addChild(goal_width);
  settings->addChild(goal_depth);
  settings->addChild(boundary_width);
  settings->addChild(line_thickness);
  settings->addChild(penalty_area_depth);
  settings->addChild(penalty_area_width);
  settings->addChild(num_cameras_total);
  settings->addChild(num_cameras_local);
  settings->addChild(var_num_lines);
  settings->addChild(var_num_arcs);
  settings->addChild(updateShapes);

  settings->addChild(field_lines_list);
  settings->addChild(field_arcs_list);

  connect(updateShapes,SIGNAL(wasEdited(VarType*)),this,SLOT(updateFieldLinesAndArcs()));
  connect(var_num_lines, SIGNAL(wasEdited(VarType*)),
          this, SLOT(ResizeFieldLines()));
  connect(var_num_arcs, SIGNAL(wasEdited(VarType*)),
          this, SLOT(ResizeFieldArcs()));
  connect(field_lines_list, SIGNAL(XMLwasRead(VarType*)),
          this, SLOT(ProcessNewFieldLines()));
  connect(field_arcs_list, SIGNAL(XMLwasRead(VarType*)),
          this, SLOT(ProcessNewFieldArcs()));

  shapeTypeMap["Undefined"] = Undefined;
  shapeTypeMap["CenterCircle"] = CenterCircle;
  shapeTypeMap["TopTouchLine"] = TopTouchLine;
  shapeTypeMap["BottomTouchLine"] = BottomTouchLine;
  shapeTypeMap["LeftGoalLine"] = LeftGoalLine;
  shapeTypeMap["RightGoalLine"] = RightGoalLine;
  shapeTypeMap["HalfwayLine"] = HalfwayLine;
  shapeTypeMap["CenterLine"] = CenterLine;
  shapeTypeMap["LeftPenaltyStretch"] = LeftPenaltyStretch;
  shapeTypeMap["RightPenaltyStretch"] = RightPenaltyStretch;
  shapeTypeMap["LeftFieldLeftPenaltyStretch"] = LeftFieldLeftPenaltyStretch;
  shapeTypeMap["LeftFieldRightPenaltyStretch"] = LeftFieldRightPenaltyStretch;
  shapeTypeMap["RightFieldLeftPenaltyStretch"] = RightFieldLeftPenaltyStretch;
  shapeTypeMap["RightFieldRightPenaltyStretch"] = RightFieldRightPenaltyStretch;

  emit calibrationChanged();
}

RoboCupField::~RoboCupField() {
  delete field_length;
  delete field_width;
  delete goal_width;
  delete goal_depth;
  delete boundary_width;
  for (size_t i = 0; i < field_lines.size(); ++i) {
    delete field_lines[i];
  }
  for (size_t i = 0; i < field_arcs.size(); ++i) {
    delete field_arcs[i];
  }
  delete updateShapes;
  delete settings;
}

SSL_FieldShapeType RoboCupField::parseShapeType(const VarStringEnum* value) const {
  std::string type = value->getString();
  if(shapeTypeMap.find(type) != shapeTypeMap.end()) {
    return shapeTypeMap.at(type);
  }
  return Undefined;
}

void RoboCupField::toProtoBuffer(SSL_GeometryFieldSize& buffer) const {
  field_markings_mutex.lockForRead();
  buffer.Clear();
  buffer.set_field_length(field_length->getDouble());
  buffer.set_field_width(field_width->getDouble());
  buffer.set_goal_width(goal_width->getDouble());
  buffer.set_goal_depth(goal_depth->getDouble());
  buffer.set_boundary_width(boundary_width->getDouble());
  buffer.set_penalty_area_depth(penalty_area_depth->getDouble());
  buffer.set_penalty_area_width(penalty_area_width->getDouble());
  for (size_t i = 0; i < field_lines.size(); ++i) {
    const FieldLine& line = *(field_lines[i]);
    SSL_FieldLineSegment proto_line;
    proto_line.set_name(line.name->getString());
    proto_line.set_type(parseShapeType(line.type));
    proto_line.mutable_p1()->set_x(line.p1_x->getDouble());
    proto_line.mutable_p1()->set_y(line.p1_y->getDouble());
    proto_line.mutable_p2()->set_x(line.p2_x->getDouble());
    proto_line.mutable_p2()->set_y(line.p2_y->getDouble());
    proto_line.set_thickness(line.thickness->getDouble());
    *(buffer.add_field_lines()) = proto_line;
  }
  for (size_t i = 0; i < field_arcs.size(); ++i) {
    const FieldCircularArc& arc = *(field_arcs[i]);
    SSL_FieldCicularArc proto_arc;
    proto_arc.set_name(arc.name->getString());
    proto_arc.set_type(parseShapeType(arc.type));
    proto_arc.mutable_center()->set_x(arc.center_x->getDouble());
    proto_arc.mutable_center()->set_y(arc.center_y->getDouble());
    proto_arc.set_radius(arc.radius->getDouble());
    proto_arc.set_a1(arc.a1->getDouble());
    proto_arc.set_a2(arc.a2->getDouble());
    proto_arc.set_thickness(arc.thickness->getDouble());
    *(buffer.add_field_arcs()) = proto_arc;
  }
  field_markings_mutex.unlock();
}

void RoboCupField::ProcessNewFieldArcs() {
  field_markings_mutex.lockForWrite();
  vector<VarType*> field_arc_entries = field_arcs_list->getChildren();
  for (size_t i = 0; i < field_arc_entries.size(); ++i) {
    VarList* field_arc_list_ptr =
        reinterpret_cast<VarList*>(field_arc_entries[i]);
    bool found = false;
    for (size_t j = 0; !found && j < field_arcs.size(); ++j) {
      found = (field_arcs[j]->list == field_arc_list_ptr);
    }
    if (!found) {
      // This item is not in field_arcs, try to cast and add it.
      FieldCircularArc* new_arc =
          FieldCircularArc::FromVarList(field_arc_list_ptr);
      if (new_arc != NULL) {
        // Cast succeeded, add it.
        field_arcs.push_back(new_arc);
      } else {
        // Cast failed, drop this entry.
        field_arcs_list->removeChild(field_arc_list_ptr);
      }
    }
  }
  if (static_cast<size_t>(field_arcs_list->getChildrenCount()) != field_arcs.size()) {
    fprintf(stderr, "Bug discovered, please report to the developers: "
            "field_arcs_list->getChildrenCount() != field_arcs.size(), @ "
            "%s:%d\n", __FILE__, __LINE__);
  }
  var_num_arcs->setInt(field_arcs.size());
  field_markings_mutex.unlock();
}

void RoboCupField::ProcessNewFieldLines() {
  field_markings_mutex.lockForWrite();
  vector<VarType*> field_line_entries = field_lines_list->getChildren();
  for (size_t i = 0; i < field_line_entries.size(); ++i) {
    VarList* field_line_list_ptr =
        reinterpret_cast<VarList*>(field_line_entries[i]);
    bool found = false;
    for (size_t j = 0; !found && j < field_lines.size(); ++j) {
      found = (field_lines[j]->list == field_line_list_ptr);
    }
    if (!found) {
      // This item is not in field_lines, try to cast and add it.
      FieldLine* new_line = FieldLine::FromVarList(field_line_list_ptr);
      if (new_line != NULL) {
        // Cast succeeded, add it.
        field_lines.push_back(new_line);
      } else {
        // Cast failed, drop this entry.
        field_lines_list->removeChild(field_line_list_ptr);
      }
    }
  }
  if (static_cast<size_t>(field_lines_list->getChildrenCount()) != field_lines.size()) {
    fprintf(stderr, "Bug discovered, please report to the developers: "
            "field_lines_list->getChildrenCount() != field_lines.size(), @ "
            "%s:%d\n", __FILE__, __LINE__);
  }
  var_num_lines->setInt(field_lines.size());
  field_markings_mutex.unlock();
}

void RoboCupField::ResizeFieldLines() {
  field_markings_mutex.lockForWrite();
  // Resize the field_lines list.
  const size_t new_num_lines = static_cast<size_t>(var_num_lines->getInt());
  const size_t old_num_lines = field_lines.size();
  if (new_num_lines < old_num_lines) {
    // Remove the last few line segments and their associated VarType objects.
    for (size_t i = new_num_lines; i < old_num_lines; ++i) {
      field_lines_list->removeChild(field_lines[i]->list);
      delete field_lines[i];
    }
    field_lines.resize(new_num_lines);
  }
  if (new_num_lines > old_num_lines) {
    for (size_t i = old_num_lines; i < new_num_lines; ++i) {
      const string name = StringPrintf("Line %d", static_cast<int>(i));
      field_lines.push_back(new FieldLine(name));
      field_lines_list->addChild(field_lines[i]->list);
    }
  }
  if (static_cast<size_t>(field_lines_list->getChildrenCount()) != field_lines.size()) {
    fprintf(stderr, "Bug discovered, please report to the developers: "
            "field_lines_list->getChildrenCount() != field_lines.size(), @ "
            "%s:%d\n", __FILE__, __LINE__);
  }
  field_markings_mutex.unlock();
}

void RoboCupField::ResizeFieldArcs() {
  field_markings_mutex.lockForWrite();
  // Resize the field_arcs list.
  const size_t new_num_arcs = static_cast<size_t>(var_num_arcs->getInt());
  const size_t old_num_arcs = field_arcs.size();
  if (new_num_arcs < old_num_arcs) {
    // Remove the last few arc segments and their associated VarType objects.
    for (size_t i = new_num_arcs; i < old_num_arcs; ++i) {
      field_arcs_list->removeChild(field_arcs[i]->list);
      delete field_arcs[i];
    }
    field_arcs.resize(new_num_arcs);
  }
  if (new_num_arcs > old_num_arcs) {
    for (size_t i = old_num_arcs; i < new_num_arcs; ++i) {
      const string name = StringPrintf("Arc %d", static_cast<int>(i));
      field_arcs.push_back(new FieldCircularArc(name));
      field_arcs_list->addChild(field_arcs[i]->list);
    }
  }
  if (static_cast<size_t>(field_arcs_list->getChildrenCount()) != field_arcs.size()) {
    fprintf(stderr, "Bug discovered, please report to the developers: "
            "field_arcs_list->getChildrenCount() != field_arcs.size(), @ "
            "%s:%d\n", __FILE__, __LINE__);
  }
  field_markings_mutex.unlock();
}

void RoboCupField::updateFieldLinesAndArcs() {
    field_markings_mutex.lockForWrite();

    // Delete all old lines.
    vector<VarType*> old_lines = field_lines_list->getChildren();
    for (size_t i = 0; i < old_lines.size(); ++i) {
    field_lines_list->removeChild(old_lines[i]);
    delete field_lines[i];
    }
    field_lines.clear();

    // Load default lines.
    double fieldLengthHalf = field_length->getDouble() / 2.0;
    double fieldWidthHalf = field_width->getDouble() / 2.0;
    double penAreaX = fieldLengthHalf - penalty_area_depth->getDouble();
    double penAreaY = penalty_area_width->getDouble() / 2.0;
    field_lines.push_back(new FieldLine("TopTouchLine", -fieldLengthHalf, fieldWidthHalf, fieldLengthHalf, fieldWidthHalf, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("BottomTouchLine", -fieldLengthHalf, -fieldWidthHalf, fieldLengthHalf, -fieldWidthHalf, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("LeftGoalLine", -fieldLengthHalf, -fieldWidthHalf, -fieldLengthHalf, fieldWidthHalf, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("RightGoalLine", fieldLengthHalf, -fieldWidthHalf, fieldLengthHalf, fieldWidthHalf, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("HalfwayLine", 0, -fieldWidthHalf, 0, fieldWidthHalf, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("CenterLine", -fieldLengthHalf, 0, fieldLengthHalf, 0, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("LeftPenaltyStretch", -penAreaX, -penAreaY, -penAreaX, penAreaY, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("RightPenaltyStretch", penAreaX, -penAreaY, penAreaX, penAreaY, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("LeftFieldLeftPenaltyStretch", -fieldLengthHalf, -penAreaY, -penAreaX, -penAreaY, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("LeftFieldRightPenaltyStretch", -fieldLengthHalf, penAreaY, -penAreaX, penAreaY, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("RightFieldRightPenaltyStretch", fieldLengthHalf, -penAreaY, penAreaX, -penAreaY, line_thickness->getDouble()));
    field_lines.push_back(new FieldLine("RightFieldLeftPenaltyStretch", fieldLengthHalf, penAreaY, penAreaX, penAreaY, line_thickness->getDouble()));

    var_num_lines->setInt(field_lines.size());
    for (size_t i = 0; i < field_lines.size(); ++i) {
        field_lines_list->addChild(field_lines[i]->list);
    }

    // Delete all old arcs.
    vector<VarType*> old_arcs = field_arcs_list->getChildren();
    for (size_t i = 0; i < old_arcs.size(); ++i) {
      field_arcs_list->removeChild(old_arcs[i]);
      delete field_arcs[i];
    }
    field_arcs.clear();

    // Load default arcs.
    field_arcs.push_back(new FieldCircularArc("CenterCircle", 0, 0, FieldConstantsRoboCup2018A::kCenterCircleRadius, 0, 2.0 * M_PI, 10));

    var_num_arcs->setInt(field_arcs.size());
    for (size_t i = 0; i < field_arcs.size(); ++i) {
      field_arcs_list->addChild(field_arcs[i]->list);
    }
    field_markings_mutex.unlock();
}

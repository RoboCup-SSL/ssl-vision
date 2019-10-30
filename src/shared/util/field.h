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
  \file    field.h
  \brief   Definition of field dimensions
  \author  Stefan Zickler / Tim Laue, (C) 2009
*/
//========================================================================

#ifndef FIELD_H
#define FIELD_H
#include "field_default_constants.h"

#include "VarTypes.h"
#include "helpers.h"
#include <QObject>
#include <QReadWriteLock>

#include "messages_robocup_ssl_geometry.pb.h"

using namespace VarTypes;

/*!
  \class RoboCupField

  \brief Definition of all variables for a symmetric, regulation-style RoboCup SSL field

  \author Stefan Zickler , (C) 2009
**/

class FieldLine : public QObject {
Q_OBJECT
protected:
  FieldLine(VarString* name_,
            VarStringEnum* type_,
            VarDouble* p1_x_,
            VarDouble* p1_y_,
            VarDouble* p2_x_,
            VarDouble* p2_y_,
            VarDouble* thickness_,
            VarList* list_);
private:
  // Disable assignment operator.
  const FieldLine& operator=(const FieldLine& other);
public:
  VarString* name;
  VarStringEnum* type;
  VarDouble* p1_x;
  VarDouble* p1_y;
  VarDouble* p2_x;
  VarDouble* p2_y;
  VarDouble* thickness;
  VarList* list;

  FieldLine(const FieldLine& other);
  FieldLine(const std::string& marking_name);
  FieldLine(const std::string& marking_name,
              double p1_x_, double p1_y_, double p2_x_, double p2_y_,
              double thickness_);
  ~FieldLine();

  // Reads the entries from the specified VarList and attempts to create a
  // FieldLine object from the entries. If succesful, it returns a valid pointer
  // to a newly created FieldLine object. If it fails, it returns NULL.
  static FieldLine* FromVarList(VarList* list);

protected slots:
  void Rename();

    void fillTypeEnum() const;
};

class FieldCircularArc : public QObject {
Q_OBJECT
protected:
  FieldCircularArc(VarString* name_,
                   VarStringEnum* type_,
                   VarDouble* center_x_,
                   VarDouble* center_y_,
                   VarDouble* radius_,
                   VarDouble* a1_,
                   VarDouble* a2_,
                   VarDouble* thickness_,
                   VarList* list_);

private:
  // Disable assignment operator.
  const FieldCircularArc& operator=(const FieldCircularArc& other);

public:
  VarString* name;
  VarStringEnum* type;
  VarDouble* center_x;
  VarDouble* center_y;
  VarDouble* radius;
  VarDouble* a1;
  VarDouble* a2;
  VarDouble* thickness;
  VarList* list;

  FieldCircularArc(const FieldCircularArc& other);
  FieldCircularArc(const std::string& marking_name);
  FieldCircularArc(const std::string& marking_name,
              double center_x_, double center_y_, double radius_,
              double a1_, double a2_, double thickness_);
  ~FieldCircularArc();

  // Reads the entries from the specified VarList and attempts to create a
  // FieldCircularArc object from the entries. If succesful, it returns a valid
  // pointer to a newly created FieldCircularArc object. If it fails, it returns
  // NULL.
  static FieldCircularArc* FromVarList(VarList* list);

private slots:
  void Rename();
  void fillTypeEnum() const;
};

class RoboCupField : public QObject {
Q_OBJECT
protected:
  VarList* settings;
  VarTrigger* updateShapes;
public:
  VarList* getSettings() const {
    return settings;
  }

  void toProtoBuffer(SSL_GeometryFieldSize& buffer) const ;

  mutable QReadWriteLock field_markings_mutex;
  VarDouble* field_length;
  VarDouble* field_width;
  VarDouble* goal_width;
  VarDouble* goal_depth;
  VarDouble* boundary_width;
  VarDouble* line_thickness;
  VarDouble* penalty_area_depth;
  VarDouble* penalty_area_width;
  VarInt* num_cameras_total;
  VarInt* num_cameras_local;
  VarInt* var_num_lines;
  VarInt* var_num_arcs;
  VarList* field_lines_list;
  VarList* field_arcs_list;
  vector<FieldLine*> field_lines;
  vector<FieldCircularArc*> field_arcs;

  RoboCupField();
  ~RoboCupField();

private:
  std::map<std::string, SSL_FieldShapeType> shapeTypeMap;
  SSL_FieldShapeType parseShapeType(const VarStringEnum* value) const;

 public:
  signals:
  void calibrationChanged();

protected slots:
  void ProcessNewFieldLines();
  void ProcessNewFieldArcs();
  void ResizeFieldLines();
  void ResizeFieldArcs();
  void changed() {
    calibrationChanged();
  }
  void updateFieldLinesAndArcs();
};


#endif // FIELD_H

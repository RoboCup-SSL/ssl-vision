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
  \file    field_filter.h
  \brief   A filter based on field location
  \author  Stefan Zickler, (C) 2009
*/
//========================================================================

#ifndef FIELD_FILTER_H
#define FIELD_FILTER_H

#include "field.h"

/*!
  \class Field Filter

  \brief   A filter based on field location, to be used for vision filtering

  \author Stefan Zickler, (C) 2009
**/
class FieldFilter
{
protected:
  double half_field_width;
  double half_field_length;
  double half_goal_width;
  double goal_depth;
  double boundary_width;
public:
  FieldFilter()
  {
    half_field_width=0.0;
    half_field_length=0.0;
    half_goal_width=0.0;
    goal_depth=0.0;
    boundary_width=0.0;
  }

  void update(const RoboCupField & field) {
    half_field_width  = (double)(field.half_field_width->getInt());
    half_field_length = (double)(field.half_field_length->getInt());
    half_goal_width   = (double)(field.half_goal_width->getInt());
    goal_depth   = (double)(field.goal_depth->getInt());
    boundary_width = (double)(field.boundary_width->getInt());
  }

  ///check whether a point is within the legal field or the boundary (but not the referee walking area)
  bool isInFieldOrPlayableBoundary(const vector2d & pos) {
    return (fabs(pos.x) <= (half_field_length+boundary_width) &&  fabs(pos.y) <= (half_field_width+boundary_width));
  }

  ///check whether a point is within the legal field (excluding all boundary areas) plus some threshold
  bool isInFieldPlusThreshold(const vector2d & pos, double threshold) {
    return (fabs(pos.x) <= (half_field_length+threshold) &&  fabs(pos.y) <= (half_field_width+threshold));
  }

  ///check whether a point is within the legal field (excluding all boundary areas)
  bool isInField(const vector2d & pos) {
    return (fabs(pos.x) <= half_field_length && fabs(pos.y) <= half_field_width);
  }

  ///checks whether a point is very far in the goal (more than half-way)
  ///this is mostly used for vision filtering
  bool isFarInGoal(const vector2d & pos) {
    return (fabs(pos.y) < half_goal_width &&
            fabs(pos.x) > half_field_length + (goal_depth/2));
  }
  
};


#endif // FIELD_FILTER_H

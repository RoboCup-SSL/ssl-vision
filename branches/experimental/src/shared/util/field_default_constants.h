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
  \file    field_default_constants.h
  \brief   Definition of field dimensions
  \author  Stefan Zickler / Tim Laue, (C) 2009
*/
//========================================================================

#ifndef FIELD_DEFAULT_CONSTANTS_H
#define FIELD_DEFAULT_CONSTANTS_H
namespace FieldConstantsRoboCup2009 {
  static const int line_width = 10;
  static const int field_length=6050;
  static const int field_width=4050;
  static const int boundary_width=250;
  static const int referee_width=425;
  static const int goal_width=700;
  static const int goal_depth=180;
  static const int goal_wall_width=20;
  static const int center_circle_radius=500;
  static const int defense_radius=500;
  static const int defense_stretch=350;
  static const int free_kick_from_defense_dist=200;
  static const int penalty_spot_from_field_line_dist=450;
  static const int penalty_line_from_spot_dist=400;
}
#endif // FIELD_H

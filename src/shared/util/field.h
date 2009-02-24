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
  \author  Tim Laue, (C) 2009
*/
//========================================================================

#ifndef FIELD_H
#define FIELD_H

#include <VarInt.h>

/*!
  \class Field

  \brief Definition of point coordinates (in mm) defining one half of the field

  \author Tim Laue, (C) 2009
**/
class Field
{
public:
  Field()
  {
    left_corner_x = new VarInt("left corner x", 3025);
    left_corner_y = new VarInt("left corner y", 2025);
    left_goal_area_x = new VarInt("left goal area x", 3025); 
    left_goal_area_y = new VarInt("left goal area y", 675); 
    left_goal_post_x = new VarInt("left post area x", 3025); 
    left_goal_post_y = new VarInt("left post area y", 350); 
    right_goal_post_x = new VarInt("right post area x", 3025); 
    right_goal_post_y = new VarInt("right post area y", -350); 
    right_goal_area_x = new VarInt("right goal area x", 3025); 
    right_goal_area_y = new VarInt("right goal area y", -675); 
    right_corner_x = new VarInt("right corner x", 3025);
    right_corner_y = new VarInt("right corner y", -2025);
    left_centerline_x = new VarInt("left centerline x", 0);
    left_centerline_y = new VarInt("left centerline y", 2025);
    left_centercircle_x = new VarInt("left centercircle x", 0); 
    left_centercircle_y = new VarInt("left centercircle y", 500); 
    centerpoint_x = new VarInt("centercircle x", 0); 
    centerpoint_y = new VarInt("centercircle y", 0); 
    right_centercircle_x = new VarInt("right centercircle x", 0); 
    right_centercircle_y = new VarInt("right centercircle y", -500); 
    right_centerline_x = new VarInt("right centerline x", 0);
    right_centerline_y = new VarInt("right centerline y", -2025);
  }
  
  ~Field()
  {
    delete left_corner_x;
    delete left_corner_y;
    delete left_goal_area_x; 
    delete left_goal_area_y; 
    delete left_goal_post_x; 
    delete left_goal_post_y; 
    delete right_goal_post_x; 
    delete right_goal_post_y; 
    delete right_goal_area_x; 
    delete right_goal_area_y; 
    delete right_corner_x;
    delete right_corner_y;
    delete left_centerline_x;
    delete left_centerline_y;
    delete left_centercircle_x;
    delete left_centercircle_y; 
    delete centerpoint_x; 
    delete centerpoint_y; 
    delete right_centercircle_x; 
    delete right_centercircle_y; 
    delete right_centerline_x;
    delete right_centerline_y;
  }
  
  void addSettingsToList(VarList& list) 
  {
    list.addChild(left_corner_x);
    list.addChild(left_corner_y);
    list.addChild(left_goal_area_x); 
    list.addChild(left_goal_area_y); 
    list.addChild(left_goal_post_x); 
    list.addChild(left_goal_post_y); 
    list.addChild(right_goal_post_x); 
    list.addChild(right_goal_post_y); 
    list.addChild(right_goal_area_x); 
    list.addChild(right_goal_area_y); 
    list.addChild(right_corner_x);
    list.addChild(right_corner_y);
    list.addChild(left_centerline_x);
    list.addChild(left_centerline_y);
    list.addChild(left_centercircle_x); 
    list.addChild(left_centercircle_y); 
    list.addChild(centerpoint_x); 
    list.addChild(centerpoint_y); 
    list.addChild(right_centercircle_x); 
    list.addChild(right_centercircle_y); 
    list.addChild(right_centerline_x);
    list.addChild(right_centerline_y);
  }
  
  VarInt* left_corner_x;
  VarInt* left_corner_y;
  VarInt* left_goal_area_x; 
  VarInt* left_goal_area_y; 
  VarInt* left_goal_post_x; 
  VarInt* left_goal_post_y; 
  VarInt* right_goal_post_x; 
  VarInt* right_goal_post_y; 
  VarInt* right_goal_area_x; 
  VarInt* right_goal_area_y; 
  VarInt* right_corner_x;
  VarInt* right_corner_y;
  VarInt* left_centerline_x;
  VarInt* left_centerline_y;
  VarInt* left_centercircle_x; 
  VarInt* left_centercircle_y; 
  VarInt* centerpoint_x; 
  VarInt* centerpoint_y; 
  VarInt* right_centercircle_x; 
  VarInt* right_centercircle_y; 
  VarInt* right_centerline_x;
  VarInt* right_centerline_y;
};


#endif // FIELD_H

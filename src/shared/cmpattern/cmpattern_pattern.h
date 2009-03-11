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
  \file    pattern.h
  \brief   C++ Interface: pattern
  \author  Author Name, 2009
*/
//========================================================================
#ifndef CM_PATTERN_PATTERN_H
#define CM_PATTERN_PATTERN_H
#include "image.h"
#include "cmvision_region.h"
namespace CMPattern {

/**
	@author Author Name
*/


class Marker {
public:
  float area;         // estimated area in sq. mm
  yuv   color_yuv;    // average color
  raw8  id;           // marker id [0,3] (white=0, green=1, pink=2, cyan=3)
  const CMVision::Region *reg;  // source region from vision

  // The following two fields are for use by callers (i.e. FindRobots)
  float dist;         // distance from center marker
  float angle;        // angle around center marker
  float next_dist;    // distance to next marker CW around pattern

  Marker *next; // linked list storage for markmap
public:
  Marker() {
    next=0;
    reg=0;
    id=0;
    area=0.0;
    dist=0.0;
    angle=0.0;
    next_dist=0.0;
  }
  //void set(Detect &det,const Region *_reg,float z);
};


class Pattern{
protected:
    Marker * markers;
    int num_markers;
public:
    int getNumMarkers();
    
    Pattern();

    ~Pattern();

};

}

#endif

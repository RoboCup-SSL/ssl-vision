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
  \file    cmvision_region.h
  \brief   C++ Interface: cmvision_region
  \author  Author Name, 2008
*/
//========================================================================
#ifndef CMVISION_REGION_H
#define CMVISION_REGION_H
#include "colors.h"
#include "image.h"

#define CMV_DEFAULT_MAX_RUNS 100000

namespace CMVision {


class Run{
public:
  int x,y,width;    // location and width of run
  raw8 color;       // which color(s) this run represents
  int parent,next;    // parent run and next run in run list
};


class Runlist {
private:
  Run * runs;
  int max_runs;
  int used_runs;
public:
  Runlist(int _max_runs) {
    runs=new Run[_max_runs];
    max_runs=_max_runs;
    used_runs=0;
  }
  void setUsedRuns(int runs) {
    used_runs=runs;
  }
  int getUsedRuns() {
    return used_runs;
  }
  ~Runlist() {
    delete[] runs;
  }
public:
  Run * getRunArrayPointer() {
    return runs;
  }
  int getMaxRuns() {
    return max_runs;
  }
};



class Region{
  public:
  raw8 color;        // id of the color
  int x1,y1,x2,y2;   // bounding box (x1,y1) - (x2,y2)
  float cen_x,cen_y; // centroid
  int area;          // occupied area in pixels
  int run_start;     // first run index for this region
  int iterator_id;   // id to prevent duplicate hits by an iterator
  Region *next;      // next region in list
  Region *tree_next; // next pointer for use in spatial lookup trees

  // accessor for centroid
  float operator[](int idx) const
    {return((&cen_x)[idx]);}

  // width/height accessors
  int width() const
    {return(x2-x1+1);}
  int height() const
    {return(y2-y1+1);}
};

class Regionlist {
private:
  Region * regions;
  int max_regions;
  int used_regions;
public:
  Regionlist(int _max_regions) {
    regions=new Region[_max_regions];
    max_regions=_max_regions;
    used_regions=0;
  }
  void setUsedRegions(int regions) {
    used_regions=regions;
  }
  int getUsedRegions() {
    return used_regions;
  }
  ~Regionlist() {
    delete[] regions;
  }
public:
  Region * getRegionArrayPointer() {
    return regions;
  }
  int getMaxRegions() {
    return max_regions;
  }
};



};


/**
	@author Author Name
*/
class CMVisionRegion{
protected:
  //==== Utility Functions ===========================================//
  // sum of integers over range [x,x+w)
  inline static int rangeSum(int x,int w)
  {
    return(w*(2*x + w-1) / 2);
  }

  // sum of integer squares over range [x,x+w)
  // S(n) = n*(n+1)*(2*n+1) / 6
  // R(x,w) = S(x+w-1) - S(x-1)
  // ref: http://mathworld.wolfram.com/SquarePyramidalNumber.html
  // note: if x+w > 1024, you must use 64-bit ints for correct results
  inline static int rangeSumSq(int x,int w)
  {
    int y = x + w;
    int rs = y*(y-1)*(2*y-1) - x*(x-1)*(2*x-1);
    return(rs / 6);
  }


public:
    CMVisionRegion();

    ~CMVisionRegion();

    static void encodeRuns(Image<raw8> * tmap, CMVision::Runlist * runlist);
    static void connectComponents(CMVision::Runlist * runlist);
    static void extractRegions(CMVision::Regionlist * reglist, CMVision::Runlist * runlist);

};

#endif

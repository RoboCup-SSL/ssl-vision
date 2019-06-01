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
  \author  James Bruce (Original CMVision implementation and algorithms),
           SSL-Vision code restructuring, and data structure changes: Stefan Zickler 2008
*/
//========================================================================
#ifndef CMVISION_REGION_H
#define CMVISION_REGION_H
#include "colors.h"
#include "image.h"
#include "geometry.h"
#include "nkdtree.h"
#include "cmvision_threshold.h"
#include "lut3d.h"

namespace CMVision {


class Run{
public:
  int x,y,width;    // location and width of run
  raw8 color;       // which color(s) this run represents
  int parent,next;    // parent run and next run in run list
};


class RunList {
private:
  Run * runs;
  int max_runs;
  int used_runs;
public:
  RunList(int _max_runs) {
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
  ~RunList() {
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

class RegionList {
private:
  Region * regions;
  int max_regions;
  int used_regions;
public:
  RegionList(int _max_regions) {
    regions=new Region[_max_regions];
    max_regions=_max_regions;
    used_regions=0;
  }
  void setUsedRegions(int regions) {
    used_regions=regions;
  }
  int getUsedRegions() const {
    return used_regions;
  }
  ~RegionList() {
    delete[] regions;
  }
public:
  Region * getRegionArrayPointer() const {
    return regions;
  }
  int getMaxRegions() const {
    return max_regions;
  }
};


class RegionLinkedList {
protected:
  Region * _first;
  int _num;
public:
  RegionLinkedList() {
    reset();
  }
  Region * getInitialElement() const {
    return _first;
  }
  int getNumRegions() const {
   return _num;
  };
  void setFront(Region * r) {
    _first=r;
  }
  void setNum(int num) {
    _num=num;
  }
  void reset() {
    _first=0;
    _num=0;
  }
  inline void insertFront(Region * r) {
    r->next=_first;
    _first=r;
    _num++;
  }
};

class ColorRegionList {
private:
  RegionLinkedList * color_regions;
  int num_color_regions;
public:
  ColorRegionList(int _num_color_regions) {
    color_regions=new RegionLinkedList[_num_color_regions];
    num_color_regions=_num_color_regions;
  }
  ~ColorRegionList() {
    delete[] color_regions;
  }
public:
  const RegionLinkedList & getRegionList(int idx) const {
    return color_regions[idx];
  }
  RegionLinkedList * getColorRegionArrayPointer() const {
    return color_regions;
  }
  int getNumColorRegions() const {
    return num_color_regions;
  }
};


class RegionFilter{
protected:
  const CMVision::Region *reg;
  int w,h;
  ClosedRangeInt area;
  ClosedRangeInt width;
  ClosedRangeInt height;
public:
  RegionFilter() {reg=0; w=0; h=0; area.set(0,1000000); width.set(0,1000); height.set(0,1000); }
  void setArea(ClosedRangeInt & _area) {
    area=_area;
  }
  void setWidth(ClosedRangeInt & _width) {
    width=_width;
  }
  void setHeight(ClosedRangeInt & _height) {
    height=_height;
  }
  void setArea(int _min, int _max) {
    area.min=_min;
    area.max=_max;
  }
  void setWidth(int _min, int _max) {
    width.min=_min;
    width.max=_max;
  }
  void setHeight(int _min, int _max) {
    height.min=_min;
    height.max=_max;
  }
  ClosedRangeInt getArea() {
    return area;
  }
  ClosedRangeInt getWidth() {
    return width;
  }
  ClosedRangeInt getHeight() {
    return height;
  }
  bool check(const CMVision::Region & reg) {
    int w = reg.x2 - reg.x1 + 1;
    int h = reg.y2 - reg.y1 + 1;

    return(area.inside(reg.area) && width.inside(w) && height.inside(h));
  }

  void init(const CMVision::Region *region_list) {
    reg = region_list;

    // skip too-large regions in sorted region list
    while(reg!=0 && reg->area>area.max) reg = reg->next;
  }

  const CMVision::Region * getNext()
  {
    // terminate when no regions, or no suitably large ones
    if(reg==0) return(0);

    // find the next region matching our ranges
    while(reg!=0) {
      w = reg->width();
      h = reg->height();

      if(reg->area < area.min) return(0);
      if(width.inside(w) && height.inside(h)){
        const CMVision::Region *match = reg;
        reg = reg->next;
        return(match);
      }

      reg = reg->next;
    }
    return(0);
  }
};


class RegionTreeGetNext{
public:
  Region *&operator()(Region *s)
    {return(s->tree_next);}
};


/**
  @author Author Name
*/
class RegionProcessing {
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
    RegionProcessing();

    ~RegionProcessing();

    static void encodeRuns(Image<raw8> * tmap, CMVision::RunList * runlist);
    static void connectComponents(CMVision::RunList * runlist);
    static void extractRegions(CMVision::RegionList * reglist, CMVision::RunList * runlist);
    //returns the max area found:
    static int  separateRegions(CMVision::ColorRegionList * colorlist, CMVision::RegionList * reglist, int min_area);

    static CMVision::Region * sortRegionListByArea(CMVision::Region *list,int passes);
    static void sortRegions(CMVision::ColorRegionList * colors,int max_area);

};

//a region-tree (assuming square-pixels):
typedef NKDTree<Region,float,2,false,CMVision::RegionTreeGetNext> RegionTree;

class ImageProcessor {
protected:
  YUVLUT * lut;
  int max_regions;
  int max_runs;
  CMVision::RegionList * reglist;
  CMVision::ColorRegionList * colorlist;
  CMVision::RunList * runlist;
  Image<raw8> * img_thresholded;
public:
  ImageProcessor(YUVLUT * _lut, int _max_regions=10000, int _max_runs=50000);
  ~ImageProcessor();
  void processYUV422_UYVY(const RawImage * image, int min_blob_area);
  void processYUV444(const ImageInterface * image, int min_blob_area);
  void processThresholded(Image<raw8> * _img_thresholded, int min_blob_area);
  ColorRegionList * getColorRegionList();
};

}

#endif

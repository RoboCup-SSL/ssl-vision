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
  \file    cmpattern_pattern.h
  \brief   C++ Interface: pattern
  \author  Core Algorithms (C) James Bruce
           Major code restructuring for SSL-Vision: Stefan Zickler, 2009
*/
//========================================================================
#ifndef CM_PATTERN_PATTERN_H
#define CM_PATTERN_PATTERN_H
#include "image.h"
#include "lut3d.h"
#include <algorithm>
#include "cmvision_region.h"
#include "util.h"
#include "vis_util.h"
#include "camera_calibration.h"
namespace CMPattern {

/**
	@author Author Name
*/

//formerly ModelMarker
typedef uint64_t pattern_t;
static const int MaxMarkers = 16;


class Marker {
public:
  float area;         // estimated area in sq. mm
  //yuv   color_yuv;    // average color
  raw8  id;           // marker id [0,3] (white=0, green=1, pink=2, cyan=3)
  const CMVision::Region *reg;  // source region from vision
  vector2f loc;
  float height;
  float conf;

  // The following two fields are for use by callers (i.e. FindRobots)
  float dist;         // distance from center marker
  float angle;        // angle around center marker
  float next_angle_dist;   // angular distance to next marker CW (always positive) around pattern
  float next_dist;    // distance to next marker CW around pattern

  Marker *next; // linked list storage for markmap
  void reset();
  void set(const CMVision::Region *_reg,const vector3d & _loc, double _area);
public:
  Marker() {
    reset();
  }
  //void set(Detect &det,const Region *_reg,float z);
};

class LessMarkerAngle {
  public:
  bool operator()(const Marker &a,const Marker &b){
    return(a.angle < b.angle);
  }
};

//formerly RobotCover
class PatternProcessing;
class Pattern {
friend class MultiPatternModel;
protected:
    bool enabled;
    Marker * markers; // orientation/id markers
    int num_markers;      // number of markers
    vector2f marker_mean; // mean location of markers
    pattern_t pattern;     // id pattern code
    float height;         // height of cover above ground plane
    int robot_id;         // robot id number

  void allocate(int num_markers);
public:
  void setEnabled(bool val);
  void copyMarkers(const vector<Marker> & mv);
  void reset();
    Pattern();

    ~Pattern();

};


class PatternProcessing {
public:
  static void sortMarkersByAngle(Marker *marker,int num);


};

class MultiPatternModel {
public:
  class ColorsUsed {
    protected:
    int n;
    uint8_t * used;
    public:
    ColorsUsed(uint8_t max_color_id=32) {
      n=max_color_id;
      used = new uint8_t[n];
      clear();
    }
    ~ColorsUsed() {
      delete[] used;
    }
    void clear() {
      for (int i=0;i<n;i++) {
        used[i]=0;
      }
    }
    void use(uint8_t color_id) {
      if (color_id < n) used[color_id]=1;
    }
    inline bool isUsed(uint8_t color_id) const {
      if (color_id >= n) return false;
      return used[color_id];
    }
  };
  class PatternFitParameters {
    public:
    float fit_max_error;
    float fit_variance;
    float fit_uniform;
    float fit_area_weight;
    float fit_cen_dist_weight;
    float fit_next_dist_weight;
    float fit_next_angle_dist_weight;
    void reset() {
      fit_max_error = 0.0;
      fit_variance  = 1.0;
      fit_uniform   = 0.1;
      fit_area_weight = 1.0;
      fit_cen_dist_weight = 1.0;
      fit_next_dist_weight = 1.0;
      fit_next_angle_dist_weight = 1.0;
    }
    PatternFitParameters() {
      reset();
    }
  };
  class PatternDetectionResult {
  public:
    vector2f loc;
    float angle;
    float conf;
    int id;
    int idx;
    void reset() {
      loc.set(0.0,0.0);
      angle=0.0;
      conf=0.0;
      id=0;
      idx=0;
    }
    PatternDetectionResult() {
      reset();
    }
  };
protected:
  float     marker_max_dist;
  int       num_patterns;
  Pattern * patterns;
  ColorsUsed used;
protected:
  void calcDerived();
  void allocate(int num_patterns);
  double calcFitError(const Marker *model, const Marker *markers, int num_markers, int ofs, const PatternFitParameters & fit_params) const;
public:
  MultiPatternModel();
  ~MultiPatternModel();
  Pattern & getPattern(int idx);
  int getNumPatterns();
  void clearPatternModels();
  bool usesColor(raw8 color_id) const;
  bool loadSinglePatternImage(const yuvImage & image, YUVLUT * _lut,int idx, float default_object_height=0.0);
  bool loadMultiPatternImage(const yuvImage & image, YUVLUT * _lut, int rows=4, int cols=4, float default_object_height=0.0);
  bool findPattern(PatternDetectionResult & result, Marker * markers,int num_markers, const PatternFitParameters & fit_params,const CameraParameters& camera_params) const;
  void recheckColorsUsed();//to be used if patterns have been enabled/disabled;
};



}
#endif

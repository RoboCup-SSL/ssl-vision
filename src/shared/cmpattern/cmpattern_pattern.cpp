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
  \file    cmpattern_pattern.cpp
  \brief   C++ Implementation: pattern
  \author  Core Algorithms (C) James Bruce
           Major code restructuring for SSL-Vision: Stefan Zickler, 2009
*/
//========================================================================
#include "cmpattern_pattern.h"

namespace CMPattern {

void Marker::reset() {
    next=0;
    reg=0;
    id=0;
    loc.set(0.0,0.0);
    height=0.0;
    area=0.0;
    dist=0.0;
    next_angle_dist=0.0;
    conf=0.0;
    angle=0.0;
    next_dist=0.0;
}

void Marker::set(const CMVision::Region *_reg, const vector3d & _loc, double _area) {
  reg = _reg;
  loc.set(_loc.x,_loc.y);
  height=_loc.z;
  area = _area;
  id = reg->color;
  conf = 1.0;
  // these fields are set later by consumers
  dist = 0.0;
  angle = 0.0;
  next_angle_dist=0.0;
  next_dist = 0.0;
  next = 0;
}

Pattern::Pattern() {
  reset();
}

Pattern::~Pattern()
{
  allocate(0);
}

void Pattern::reset() {
  enabled = false;
  markers = 0; // orientation/id markers
  num_markers = 0;      // number of markers
  marker_mean.set(0.0,0.0); // mean location of markers
  pattern=0x00;     // id pattern code
  height=0.0;         // height of cover above ground plane
  robot_id=0;         // robot id number
  allocate(0);
}

void Pattern::setEnabled(bool val) {
  enabled=val;
}

void Pattern::allocate(int num) {
  if (num_markers!=num) {
    if (markers!=0) {
      delete[] markers;
      markers=0;
    }
    if (num>0) {
      markers=new Marker[num];
    }
    num_markers=num;
  }
}

void Pattern::copyMarkers(const vector<Marker> & mv) {
  allocate(mv.size());
  //copy markers:
  for (unsigned int i=0;i<mv.size();i++) {
    markers[i]=mv[i];
  }
  //fix inter-list linking:
  for (unsigned int i=0;i<(mv.size()-1);i++) {
    markers[i].next= &(markers[i+1]);
  }
  //fix final element linking:
  if (mv.size() > 0) {
    markers[mv.size()-1].next=0;
  }
}

void PatternProcessing::sortMarkersByAngle(Marker *marker,int num) {
  std::sort(&marker[0],&marker[num],LessMarkerAngle());
}

MultiPatternModel::MultiPatternModel() {
  patterns=0;
  num_patterns=0;
  clearPatternModels();
}

MultiPatternModel::~MultiPatternModel() {
  allocate(0);
}

void MultiPatternModel::allocate(int num) {

  if (num_patterns!=num) {
    if (patterns!=0) {
      delete[] patterns;
      patterns=0;
    }
    if (num>0) {
      patterns=new Pattern[num];
    }
    num_patterns=num;
  }
}

int MultiPatternModel::getNumPatterns() {
  return num_patterns;
}

bool MultiPatternModel::usesColor(raw8 color_id) const {
  return used.isUsed(color_id.v);
}

bool MultiPatternModel::loadMultiPatternImage(const yuvImage & multi_image, YUVLUT * _lut, int rows, int cols, float default_object_height)
{
  yuvImage single_image;
  if (multi_image.getWidth()==0 || multi_image.getHeight()==0) {
    fprintf(stderr,"Input image was of zero size...or input format is not supported\n");
    fflush(stderr);
    return false;
  }

  int cell_w = multi_image.getWidth()  / cols;
  int cell_h = multi_image.getHeight() / rows;

  if ((cell_w * cols) != multi_image.getWidth() ||
      (cell_h * rows) != multi_image.getHeight()) {
    fprintf(stderr,"MultiPatternModel: image dimensions (%dx%d) is not exactly divisible by columns (%d) and/or rows (%d).\n",
           multi_image.getWidth(), multi_image.getHeight(), cols,rows);
    fflush(stderr);
    return(false);
  }
  allocate(rows * cols);


  single_image.allocate(cell_w,cell_h);
  clearPatternModels();
  int n = rows * cols;
  for(int idx=0; idx<n; idx++) {
    // extract single robot image from team
    single_image.copyFromRectArea(multi_image, (idx%cols)*cell_w, (idx/cols)*cell_h, cell_w, cell_h );

    // process robot image
    loadSinglePatternImage(single_image,_lut,idx,default_object_height);
  }

  marker_max_dist = 0.0;
  for(int i=0; i<num_patterns; i++){
    Pattern &p = patterns[i];
    for(int j=0; j< p.num_markers; j++){
      take_max(marker_max_dist,p.markers[j].loc.length());
    }
  }

  return(num_patterns > 0);
}

void MultiPatternModel::clearPatternModels() {
  used.clear();
  for (int i = 0; i < num_patterns; i ++) {
    patterns[i].reset();
  }
  marker_max_dist=0.0;
}


bool MultiPatternModel::loadSinglePatternImage(const yuvImage & image, YUVLUT * _lut,int idx, float default_object_height) {

  if (_lut==0) return false;
  int color_team_id=_lut->getChannelID("Blue");
  if (color_team_id==-1) {
    fprintf(stderr,"Unable to find color label 'Blue' in LUT!\n");
    fflush(stderr);
    return false;
  }

  int color_height_id=_lut->getChannelID("Yellow");
  if (color_height_id==-1) {
    fprintf(stderr,"Warning: Unable to find color label 'Yellow' in LUT!\n");
    fflush(stderr);
  }

  vector<int> marker_color_ids;
  vector<string> marker_color_labels;
  marker_color_labels.push_back("Pink");
  marker_color_labels.push_back("White");
  marker_color_labels.push_back("Cyan");
  marker_color_labels.push_back("Green");

  for (unsigned int i = 0 ; i < marker_color_labels.size(); i++) {
    int id=_lut->getChannelID(marker_color_labels[i]);
    if (id==-1) {
      fprintf(stderr,"Unable to find color label '%s' in LUT!\n",marker_color_labels[i].c_str());
      fflush(stderr);
    } else {
      marker_color_ids.push_back(id);
    }
  }

  CMVision::ImageProcessor proc(_lut);

/*  // remap all dark greens to vision's field green color
  int n = image.Width() * image.getHeight();
  rgb green = vision.color[ColorFieldGreen].color;
  for(int i=0; i<n; i++){
    rgb c = img.img[i];
    if(c.red==0 && c.blue==0 && c.green>0 && c.green<=128){
      img.img[i] = green;
    }
  }*/

  // run low level vision on the image

  proc.processYUV444(&image,10);

  CMVision::ColorRegionList * colors = proc.getColorRegionList();

  const CMVision::Region * reg = colors->getRegionList(color_team_id).getInitialElement();

  // find center dot
  if (reg==0) {
    printf("Info: Ignoring Pattern Image for Pattern ID %d. It is missing the center marker.\n",idx);
    return false;
  }

  vector2f cen(-reg->cen_y,-reg->cen_x);

  // find height
  float height = default_object_height;
  if (color_height_id!=-1) {
    reg = colors->getRegionList(color_height_id).getInitialElement();

    if(reg!=0 ){
      if(reg->width()<1 || reg->width() > 6) {
        printf("WARNING: No Object Height Indicator Found in Image (for robot id=%d)!\n",idx);
      } else if ( reg->next != 0) {
        printf("WARNING: Multiple Height Indicators Found in Image (for robot id=%d)!\n",idx);
      } else {
        height = reg->height();
      }
    } else {
      printf("WARNING: No Object Height Indicator Found in Image (for robot id=%d)!\n",idx);
    }
  }

  std::vector<Marker> markers;
  Marker m;
  m.reset();

  for(unsigned int c=0; c<marker_color_ids.size(); c++) {
    reg = colors->getRegionList(marker_color_ids[c]).getInitialElement();
    while(reg != 0){
      if (reg->width() > 3 && reg->height() > 3) {
        vector2f p(-reg->cen_y,-reg->cen_x);
        m.loc = p - cen;
        m.id = marker_color_ids[c];
        used.use(m.id.v);
        m.area = reg->area;
        m.angle = angle_pos(m.loc.angle());
        m.dist  = m.loc.length();
        markers.push_back(m);
        reg = reg->next;
      }
    }
  }

  std::sort(markers.begin(),markers.end(),LessMarkerAngle());

  int num_markers = markers.size();
  vector2f loc_sum;
  loc_sum.zero();
  pattern_t pattern = 0x00;
  for(int i=0; i<num_markers; i++){
    int j = (i+1) % num_markers;
    markers[i].next_dist = dist(markers[i].loc,markers[j].loc);
    markers[i].next_angle_dist = angle_pos(angle_diff(markers[i].angle,markers[j].angle));
    loc_sum += markers[i].loc;
    pattern = (pattern << 8) | (markers[i].id.v);
  }

  Pattern &p = patterns[idx];

  p.setEnabled(true);
  p.copyMarkers(markers);
  p.marker_mean = loc_sum / max(num_markers,1);
  p.pattern = pattern;
  p.height = height;
  p.robot_id = idx;

  //TODO:  a nice feature would be to automatically calculate histogram
  //       percentages here.

  return(true);
}


double MultiPatternModel::calcFitError(const Marker *model,
                                      const Marker *markers,
                                      int num_markers,int ofs, const PatternFitParameters & fit_params) const
{
  double sse = 0.0;
  for(int i=0; i<num_markers; i++){
    int j = (i + ofs) % num_markers;
    
    /* OLD FIT:
    sse +=
      
      (fit_params.fit_area_weight      * sq( model[i].area      - markers[j].area) +
      fit_params.fit_cen_dist_weight  * sq(  model[i].dist      - markers[j].dist) +
      fit_params.fit_next_dist_weight * sq(  model[i].next_dist - markers[j].next_dist) +
      fit_params.fit_next_angle_dist_weight * sq(  model[i].next_angle_dist - markers[j].next_angle_dist));
      printf("----------\narea: %8.2f\ncent: %8.2f\ndist: %8.2f\nangd: %8.2f\nTOTAL %8.2f\n",sq( model[i].area      - markers[j].area),sq(  model[i].dist      - markers[j].dist),sq(  model[i].next_dist - markers[j].next_dist),sq(  model[i].next_angle_dist - markers[j].next_angle_dist),sse);
      printf("areas: %f vs. %f\n",model[i].area,markers[j].area);
      printf("diff: %f\n", model[i].area      - markers[j].area\n);
      printf("diff sq: %f\n",sq( model[i].area      - markers[j].area));*/
      
     //NORMALIZED FIT:
    sse +=
      
      (fit_params.fit_area_weight      * sq( (model[i].area      - markers[j].area) / model[i].area) +
      fit_params.fit_cen_dist_weight  * sq(  (model[i].dist      - markers[j].dist) / model[i].dist) +
      fit_params.fit_next_dist_weight * sq(  (model[i].next_dist - markers[j].next_dist) / model[i].next_dist) +
      fit_params.fit_next_angle_dist_weight * sq(  (model[i].next_angle_dist - markers[j].next_angle_dist) /  model[i].next_angle_dist));
      /*printf("----------\narea: %8.5f\ncent: %8.5f\ndist: %8.5f\nangd: %8.5f\nTOTAL %8.5f\n",
      sq((model[i].area      - markers[j].area) / model[i].area),
      sq((model[i].dist      - markers[j].dist) / model[i].dist),
       sq(  (model[i].next_dist - markers[j].next_dist) / model[i].next_dist),
       sq(  (model[i].next_angle_dist - markers[j].next_angle_dist) /  model[i].next_angle_dist),sse);*/
      //sq(  model[i].dist      - markers[j].dist),sq(  model[i].next_dist - markers[j].next_dist),sq(  model[i].next_angle_dist - markers[j].next_angle_dist),sse);
      //printf("areas: %f vs. %f\n",model[i].area,markers[j].area);
      //printf("diff: %f\n", model[i].area      - markers[j].area);
      //printf("ang: %f  %f  dist: %f   sqdist: %f\n",model[i].next_angle_dist,markers[j].next_angle_dist,(model[i].next_angle_dist - markers[j].next_angle_dist),sq(  (model[i].next_angle_dist - markers[j].next_angle_dist) /  model[i].next_angle_dist));
      //printf("diff sq: %f\n",sq( model[i].area      - markers[j].area));
      
  }
  //normalize sse over number of markers:
  sse/=num_markers;
  return(sse);
}


Pattern & MultiPatternModel::getPattern(int idx) {
  return patterns[idx];
}

void MultiPatternModel::recheckColorsUsed() {
  used.clear();
  for (int i=0;i<num_patterns;i++) {
    if (getPattern(i).enabled) {
      for (int j=0;j<getPattern(i).num_markers;j++) {
        used.use(getPattern(i).markers[j].id.v);
      }
    }
  }
}

bool MultiPatternModel::findPattern(PatternDetectionResult & result, Marker * markers,int num_markers, const PatternFitParameters & fit_params,const CameraParameters& camera_params) const {
  if(markers==0 || num_markers<0) return(false);

  int best_idx = -1;
  int best_ofs = 0;
  double best_sse = sq(fit_params.fit_max_error);

  for(int ofs=0; ofs<num_markers; ofs++){
    // calculate pattern code
    pattern_t pattern = 0x00;
    for(int i=0; i<num_markers; i++){
      int j = (i + ofs) % num_markers;
      pattern = (pattern << 8) | markers[j].id.v;
    }

    // find covers with matching pattern code and number of markers
    for(int i=0; i<num_patterns; i++){
      if (patterns[i].enabled) {
        const Pattern &p = patterns[i];
        if(p.num_markers==num_markers && p.pattern==pattern){
          // calculate fit error for matching pattern
          double sse = calcFitError(p.markers,markers,num_markers,ofs,fit_params);
//          printf("SSE: %f\n",sse);
          /*if(unlikely(verbose > 1)){
            float conf = SSEVsUniform(sse,fit_variance,fit_uniform);
            printf("    0x%04X id=%X err=%6.3f conf=%0.6f\n",
                  pattern,rc.robot_id,sqrt(sse),conf);
          }*/
          if(sse < best_sse){
            best_idx = i;
            best_ofs = ofs;
            best_sse = sse;
          }
        }
      }
    }
  }

  if(best_idx >= 0){
    const Pattern &p = patterns[best_idx];

    // fix height of markers
    for(int i=0; i<num_markers; i++){
      vector2d marker_img_center(markers[i].reg->cen_x,markers[i].reg->cen_y);
      vector3d marker_center3d;
      camera_params.image2field(marker_center3d,marker_img_center,markers[i].height);
      markers[i].loc.set(marker_center3d.x,marker_center3d.y);
    }

    // rearrange vision markers so that the order matches the pattern model
    Marker tmp[MaxMarkers];
    roll(markers,tmp,num_markers,num_markers-best_ofs);

    // get the mean location of markers
    vector2f cen_avg;
    cen_avg.zero();
    for(int i=0; i<num_markers; i++){
      cen_avg += markers[i].loc;
    }
    cen_avg /= num_markers;

    // calculate orientation
    vector2f orient;
    orient.zero();
    for(int i=0; i<num_markers; i++){
      for(int j=0; j<i; j++){
        vector2f vo = markers[i].loc - markers[j].loc;
        vector2f dir = (p.markers[i].loc - p.markers[j].loc).norm();
        vector2f o = dir.project_in(vo);
        orient += o;
      }
    }
    float angle = orient.angle();
    orient.normalize();

    // fix bias in mean marker position
    cen_avg -= orient.project_out(p.marker_mean);

    // save results
    result.id = patterns[best_idx].robot_id;
    result.idx = best_idx;
    result.loc   = cen_avg;
    result.angle = angle;
    result.conf  = SSEVsUniform(best_sse,fit_params.fit_variance,fit_params.fit_uniform);
    return true;
  } else {
    result.reset();
    return false;
  }
}

}



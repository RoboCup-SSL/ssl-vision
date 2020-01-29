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
  \file    cmpattern_teamdetector.cpp
  \brief   C++ Implementation: teamdetector
  \author  Original CMDragons Robot Detection Code (C), James Bruce
           ssl-vision restructuring and modifications, Stefan Zickler, 2009
*/
//========================================================================
#include "cmpattern_teamdetector.h"

namespace CMPattern {

TeamDetectorSettings::TeamDetectorSettings(string external_file) {
 settings=new VarList("Robot Detection");
 settings->addChild(robotPatternSettings = new VarList("Pattern"));
 robotPattern = new RobotPattern(robotPatternSettings);
 if (external_file.length()==0) {
    settings->addChild(teams = new VarList("Teams"));; // a global variable, defining all teams
 } else {
    settings->addChild(teams = new VarExternal(external_file,"Teams"));; // a global variable, defining all teams
 }
 connect(teams,SIGNAL(childAdded(VarType *)),this,SLOT(slotTeamNodeAdded(VarType *)));
 settings->addChild(addTeam = new VarTrigger("Add", "Add Team..."));
 connect(addTeam,SIGNAL(signalTriggered()),this,SLOT(slotAddPressed()));

  connect(robotPattern,SIGNAL(signalChangeOccured(VarType*)),this,SLOT(slotTeamDataChanged()));
}

void TeamDetectorSettings::slotTeamNodeAdded(VarType * node) {
  team_vector.push_back(new Team((VarList *)node));
  connect(team_vector[team_vector.size()-1],SIGNAL(signalTeamNameChanged()),this,SIGNAL(teamInfoChanged()));
  emit(teamInfoChanged());
}

void TeamDetectorSettings::slotAddPressed() {
  teams->addChild(new VarList("New Team " + QString::number(teams->getChildrenCount()).toStdString()));
}

vector<Team *> TeamDetectorSettings::getTeams() const {
  return team_vector;
}

Team * TeamDetectorSettings::getTeam(int idx) const {
  if (idx < 0 || idx >= (int)team_vector.size()) return 0;
  return (team_vector[idx]);
}

TeamDetector::TeamDetector(LUT3D * lut3d, const CameraParameters& camera_params, const RoboCupField& field) : _camera_params(camera_params), _field(field) {
  _robotPattern=0;
  _lut3d=lut3d;

  histogram=0;

  color_id_cyan = _lut3d->getChannelID("Cyan");
  if (color_id_cyan == -1) printf("WARNING color label 'Cyan' not defined in LUT!!!\n");

  color_id_pink = _lut3d->getChannelID("Pink");
  if (color_id_pink == -1) printf("WARNING color label 'Pink' not defined in LUT!!!\n");

  color_id_green = _lut3d->getChannelID("Green");
  if (color_id_green == -1) printf("WARNING color label 'Green' not defined in LUT!!!\n");

  color_id_black = _lut3d->getChannelID("Black");
  if (color_id_black == -1) printf("WARNING color label 'Black' not defined in LUT!!!\n");

  color_id_white = _lut3d->getChannelID("White");
  if (color_id_white == -1) printf("WARNING color label 'White' not defined in LUT!!!\n");  

  color_id_clear = 0;

  color_id_green = _lut3d->getChannelID("Green");
  if (color_id_green == -1) printf("WARNING color label 'Green' not defined in LUT!!!\n");  

  color_id_field_green = _lut3d->getChannelID("Field Green");
  if (color_id_field_green == -1) printf("WARNING color label 'Field Green' not defined in LUT!!!\n");
}

void TeamDetector::init(RobotPattern * robotPattern, Team * team)
{
  _robotPattern=robotPattern;
  _team = team;

  if (histogram==0) histogram= new CMVision::Histogram(_lut3d->getChannelCount());

  //--------------THINGS THAT MIGHT CHANGE DURING RUNTIME BELOW:------------
  //update field:
  field_filter.update(_field);

  //read config:
  _unique_patterns=_robotPattern->_unique_patterns->getBool();
  _have_angle=_robotPattern->_have_angle->getBool();
  _load_markers_from_image_file=_robotPattern->_load_markers_from_image_file->getBool();
  _marker_image_file=_robotPattern->_marker_image_file->getString();
  _marker_image_rows=_robotPattern->_marker_image_rows->getInt();
  _marker_image_cols=_robotPattern->_marker_image_cols->getInt();
  _robot_height=_team->_robot_height->getDouble();

  _center_marker_area_mean=_robotPattern->_center_marker_area_mean->getDouble();
  _center_marker_area_stddev=_robotPattern->_center_marker_area_stddev->getDouble();
  _center_marker_uniform=_robotPattern->_center_marker_uniform->getDouble();
  _center_marker_duplicate_distance=_robotPattern->_center_marker_duplicate_distance->getDouble();

  _other_markers_max_detections=_robotPattern->_other_markers_max_detections->getInt();
  _other_markers_max_query_distance=_robotPattern->_other_markers_max_query_distance->getDouble();

  filter_team.setWidth(_robotPattern->_center_marker_min_width->getInt(),robotPattern->_center_marker_max_width->getInt());
  filter_team.setHeight(_robotPattern->_center_marker_min_height->getInt(),robotPattern->_center_marker_max_height->getInt());
  filter_team.setArea(_robotPattern->_center_marker_min_area->getInt(),robotPattern->_center_marker_max_area->getInt());

  filter_others.setWidth(_robotPattern->_other_markers_min_width->getInt(),robotPattern->_other_markers_max_width->getInt());
  filter_others.setHeight(_robotPattern->_other_markers_min_height->getInt(),robotPattern->_other_markers_max_height->getInt());
  filter_others.setArea(_robotPattern->_other_markers_min_area->getInt(),robotPattern->_other_markers_max_area->getInt());

  _histogram_enable=_robotPattern->_histogram_enable->getBool();
  _histogram_pixel_scan_radius=_robotPattern->_histogram_pixel_scan_radius->getInt();

  _histogram_markeryness.set(_robotPattern->_histogram_min_markeryness->getDouble(),_robotPattern->_histogram_max_markeryness->getDouble());
  _histogram_field_greenness.set(_robotPattern->_histogram_min_field_greenness->getDouble(),_robotPattern->_histogram_max_field_greenness->getDouble());
  _histogram_black_whiteness.set(_robotPattern->_histogram_min_black_whiteness->getDouble(),_robotPattern->_histogram_max_black_whiteness->getDouble());


  _pattern_max_dist=_robotPattern->_pattern_max_dist->getDouble();
  _pattern_fit_params.fit_area_weight=_robotPattern->_pattern_fitness_weight_area->getDouble();
  _pattern_fit_params.fit_cen_dist_weight=_robotPattern->_pattern_fitness_weight_center_distance->getDouble();
  _pattern_fit_params.fit_next_dist_weight=_robotPattern->_pattern_fitness_weight_next_distance->getDouble();
  _pattern_fit_params.fit_next_dist_weight=_robotPattern->_pattern_fitness_weight_next_angle_distance->getDouble();
  _pattern_fit_params.fit_max_error=_robotPattern->_pattern_fitness_max_error->getDouble();
  _pattern_fit_params.fit_variance=sq(_robotPattern->_pattern_fitness_stddev->getDouble());
  _pattern_fit_params.fit_uniform=_robotPattern->_pattern_fitness_uniform->getDouble();

  //load team image:


  if (_load_markers_from_image_file == true && _marker_image_file.length() > 0) {
    rgbImage rgbi;
    if (rgbi.load(_marker_image_file)) {
      //create a YUV lut that's based on color-labels not on custom data:
      YUVLUT minilut(4,4,4,"");
      minilut.copyChannels(*_lut3d);
      //compute a full LUT mapping based on NN-distance to color labels:
      minilut.computeLUTfromLabels();
      yuvImage yuvi;
      yuvi.allocate(rgbi.getWidth(),rgbi.getHeight());
      Images::convert(rgbi,yuvi);
      if (model.loadMultiPatternImage(yuvi,&minilut,_marker_image_rows,_marker_image_cols,_team->_robot_height->getDouble())==false) {
          fprintf(stderr,"Errors while processing team image file: '%s'.\n",_marker_image_file.c_str());
          fflush(stderr);
      }
    } else {
          fprintf(stderr,"Error loading team image file: '%s'.\n",_marker_image_file.c_str());
          fflush(stderr);
    }
    for (int i=0;i<model.getNumPatterns();i++) {
      model.getPattern(i).setEnabled(_robotPattern->_valid_patterns->isSelected(i));
    }
    model.recheckColorsUsed();
  }


}


TeamDetector::~TeamDetector()
{
  if (histogram !=0) delete histogram;
}

void TeamDetector::update(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots, int team_color_id, int max_robots, const Image<raw8> * image, CMVision::ColorRegionList * colorlist, CMVision::RegionTree & reg_tree) {
  color_id_team=team_color_id;
  _max_robots=max_robots;
  // robots->Clear();

  if (_unique_patterns) {
    findRobotsByModel(robots,team_color_id,image,colorlist,reg_tree);
  } else {
    findRobotsByTeamMarkerOnly(robots,team_color_id,image,colorlist);
  }

}





void TeamDetector::findRobotsByTeamMarkerOnly(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots, int team_color_id, const Image<raw8> * image, CMVision::ColorRegionList * colorlist)
{
  filter_team.init( colorlist->getRegionList(team_color_id).getInitialElement() );

  //TODO: change these to update on demand:
  //local variables
  const CMVision::Region * reg=0;
  SSL_DetectionRobot * robot=0;
  while((reg = filter_team.getNext()) != 0) {
    vector2d reg_img_center(reg->cen_x,reg->cen_y);
    vector3d reg_center3d;
    _camera_params.image2field(reg_center3d,reg_img_center,_robot_height);
    vector2d reg_center(reg_center3d.x,reg_center3d.y);

    //TODO: add confidence masking:
    //float conf = det.mask.get(reg->cen_x,reg->cen_y);
    double conf=1.0;
    if (field_filter.isInFieldOrPlayableBoundary(reg_center) &&  ((_histogram_enable==false) || checkHistogram(reg,image)==true)) {
      double area = getRegionArea(reg,_robot_height);
      double area_err = fabs(area - _center_marker_area_mean);

      conf *= GaussianVsUniform(area_err, sq(_center_marker_area_stddev), _center_marker_uniform);
      //printf("-------------\n");
      if (conf < 1e-6) conf=0.0;

      /*if(unlikely(verbose > 2)){
        printf("    area=%0.1f err=%4.1f conf=%0.6f\n",area,area_err,conf);
      }
      if(det.debug) det.color(reg,rc,conf);*/

      //allow twice as many robots for now...
      //duplicate filtering will take care of the rest below:
      robot=addRobot(robots,conf,_max_robots*2);

      if (robot!=0) {
        //setup robot:
        robot->set_x(reg_center.x);
        robot->set_y(reg_center.y);
        robot->set_pixel_x(reg->cen_x);
        robot->set_pixel_y(reg->cen_y);
        robot->set_height(_robot_height);
      }
    }
  }

  // remove duplicates ... keep the ones with higher confidence:
  int size=robots->size();
  for(int i=0; i<size; i++){
    for(int j=i+1; j<size; j++){
      if(sqdist(vector2d(robots->Get(i).x(),robots->Get(i).y()),vector2d(robots->Get(j).x(),robots->Get(j).y())) < sq(_center_marker_duplicate_distance)) {
        robots->Mutable(i)->set_confidence(0.0);
      }
    }
  }

  //remove items with 0-confidence:
  stripRobots(robots);

  //remove extra items:
  while(robots->size() > _max_robots) {
    robots->RemoveLast();
  }

}







double TeamDetector::getRegionArea(const CMVision::Region * reg, double z) const {
  // calculate area of bounding box in sq mm
  vector3d a,b;
  vector2d right(reg->x2+1,reg->y2+1);
  vector2d left(reg->x1,reg->y1);
  _camera_params.image2field(a,right,z);
  _camera_params.image2field(b,left,z);
  vector3d box = a-b;

  double box_area = fabs(box.x) * fabs(box.y);
  int box_pixels = (reg->x2+1 - reg->x1) * (reg->y2+1 - reg->y1);

  // estimate world coordinate area of region
  double area = ((double)reg->area) * box_area / ((double)box_pixels);

  return(area);
}


bool TeamDetector::checkHistogram(const CMVision::Region * reg, const Image<raw8> * image) {

  if(_histogram_pixel_scan_radius == 0) return(true);

  histogram->clear();

  int ix = (int)(reg->cen_x);
  int iy = (int)(reg->cen_y);
  int num = histogram->addBox(image,ix-_histogram_pixel_scan_radius,iy-_histogram_pixel_scan_radius,
              ix+_histogram_pixel_scan_radius,iy+_histogram_pixel_scan_radius);

  float inv_num = 1.0 / num;

  float f_markeryness =
    (float)(histogram->getChannel(color_id_pink) +
            histogram->getChannel(color_id_green) +
            histogram->getChannel(color_id_cyan)) / histogram->getChannel(color_id_team);

  float f_greenness = (float)histogram->getChannel(color_id_field_green) * inv_num;

  float f_black_white =
    (float)(histogram->getChannel(color_id_white) + histogram->getChannel(color_id_black) + histogram->getChannel(color_id_clear)) * inv_num;

  /*
  if(unlikely(verbose > 0)){
    bool mky_ok = markeryness.inside(f_markeryness);
    bool grn_ok = greenness  .inside(f_greenness  );
    bool  bw_ok = black_white.inside(f_black_white);

    printf("  hist (%5.1f,%5.1f) ",reg->cen_x,reg->cen_y);

    if(verbose == 1){
      printf("mky=%0.3f grn=%0.3f baw=%0.3f [%d%d%d]\n",
             f_markeryness,f_greenness,f_black_white,
             mky_ok,grn_ok,bw_ok);
    }else{
      printf("rad=%d num=%d\n",pixel_radius,num);

      if(verbose > 2){
        printf("    cnt:  ");
        for(int i=0; i<NumColors; i++) printf(" %4d",hist[i]);
        printf("\n");

        printf("    frac: ");
        for(int i=0; i<NumColors; i++) printf(" %4d",hist[i]);
        printf("\n");
      }

      printf("    %d mky = %0.3f = %d+%d+%d / %d\n",
             mky_ok,f_markeryness,
             hist[ColorPink],hist[ColorBrightGreen],hist[ColorCyan],
             hist[team_color]);
      printf("    %d grn = %0.3f = %d / %d\n",
             grn_ok,f_greenness,
             hist[ColorFieldGreen],num);
      printf("    %d b&w = %0.3f %d/%d * %d/%d\n",
             bw_ok,f_black_white,
             hist[ColorBackground],num,
             hist[ColorWhite     ],num);
    }
  }*/

  return(
  _histogram_markeryness.inside(f_markeryness) &&
   _histogram_field_greenness.inside(f_greenness) &&
  _histogram_black_whiteness.inside(f_black_white));
}


SSL_DetectionRobot * TeamDetector::addRobot(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots, double conf, int max_robots) {

  int size=robots->size();
  SSL_DetectionRobot * result_robot = 0;
  for (int i=0;i<size;i++) {
    if (robots->Get(i).confidence() < conf) {
      //allocate new robot at end of array
      //and shift everything down by 1...making room for newly inserted item.
      if (size < max_robots) {
        //we can expand the array by 1.
        robots->Add();
        size++;
      }
      for (int j=size-1; j>i; j--) {
        (*(robots->Mutable(j)))=robots->Get(j-1);
      }
      result_robot = robots->Mutable(i);
      result_robot->Clear();
      result_robot->set_confidence(conf);

      return result_robot;
    }
  }
  if (size < max_robots) {
    result_robot = robots->Add();
    result_robot->set_confidence(conf);
  }
  return result_robot;
}


void TeamDetector::stripRobots(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots) {
  int size=robots->size();

  int tgt=0;
  for (int src=0;src<size;src++) {
    if (robots->Get(src).confidence() != 0.0) {
      if (tgt!=src) {
        //copy operation is needed:
        (*(robots->Mutable(tgt)))=robots->Get(src);
      }
      tgt++;
    }
  }
  for (int i=tgt;i<size;i++) {
    robots->RemoveLast();
  }
}











void TeamDetector::findRobotsByModel(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots, int team_color_id, const Image<raw8> * image, CMVision::ColorRegionList * colorlist, CMVision::RegionTree & reg_tree)
{

  (void)image;
  const int MaxDetections = _other_markers_max_detections;
  Marker cen; // center marker
  Marker *markers = new Marker[MaxDetections];
  const float marker_max_query_dist = _other_markers_max_query_distance;
  const float marker_max_dist = _pattern_max_dist;

  // partially forget old detections
  //decaySeen();

  filter_team.init( colorlist->getRegionList(team_color_id).getInitialElement());
  const CMVision::Region * reg=0;
  SSL_DetectionRobot * robot=0;

  MultiPatternModel::PatternDetectionResult res;

  while((reg = filter_team.getNext()) != 0) {
    vector2d reg_img_center(reg->cen_x,reg->cen_y);
    vector3d reg_center3d;
    _camera_params.image2field(reg_center3d,reg_img_center,_robot_height);
    vector2d reg_center(reg_center3d.x,reg_center3d.y);
    //TODO add masking:
    //if(det.mask.get(reg->cen_x,reg->cen_y) >= 0.5){
    if (field_filter.isInFieldOrPlayableBoundary(reg_center)) {
      cen.set(reg,reg_center3d,getRegionArea(reg,_robot_height));
      int num_markers = 0;

      reg_tree.startQuery(*reg,marker_max_query_dist);
      double sd=0.0;
      CMVision::Region *mreg;
      while((mreg=reg_tree.getNextNearest(sd))!=0 && num_markers<MaxDetections) {
        //TODO: implement masking:
        // filter_other.check(*mreg) && det.mask.get(mreg->cen_x,mreg->cen_y)>=0.5

        if(filter_others.check(*mreg) && model.usesColor(mreg->color)) {
          vector2d marker_img_center(mreg->cen_x,mreg->cen_y);
          vector3d marker_center3d;
          _camera_params.image2field(marker_center3d,marker_img_center,_robot_height);
          Marker &m = markers[num_markers];

          m.set(mreg,marker_center3d,getRegionArea(mreg,_robot_height));
          vector2f ofs = m.loc - cen.loc;
          m.dist = ofs.length();
          m.angle = ofs.angle();

          if(m.dist>0.0 && m.dist<marker_max_dist){
            num_markers++;
          }
        }
      }
      reg_tree.endQuery();

      if(num_markers >= 2){
        CMPattern::PatternProcessing::sortMarkersByAngle(markers,num_markers);
        for(int i=0; i<num_markers; i++){
          /*DEBUG CODE:
          char colorchar='?';
          if (markers[i].id==color_id_green) colorchar='g';
          if (markers[i].id==color_id_pink) colorchar='p';
          if (markers[i].id==color_id_white) colorchar='w';
          if (markers[i].id==color_id_team) colorchar='t';
          if (markers[i].id==color_id_field_green) colorchar='f';
          if (markers[i].id==color_id_cyan) colorchar='c';
          printf("%c ",colorchar);*/
          int j = (i + 1) % num_markers;
          markers[i].next_dist = dist(markers[i].loc,markers[j].loc);
          markers[i].next_angle_dist = angle_pos(angle_diff(markers[i].angle,markers[j].angle));
        }

        if (model.findPattern(res,markers,num_markers,_pattern_fit_params,_camera_params)) {
              robot=addRobot(robots,res.conf,_max_robots*2);
              if (robot!=0) {
                //setup robot:
                robot->set_x(cen.loc.x);
                robot->set_y(cen.loc.y);
                if (_have_angle) robot->set_orientation(res.angle);
                robot->set_robot_id(res.id);
                robot->set_pixel_x(reg->cen_x);
                robot->set_pixel_y(reg->cen_y);
                robot->set_height(cen.height);
              }
        }
      }
    }
  }
  //remove items with 0-confidence:
  stripRobots(robots);

  //remove extra items:
  while(robots->size() > _max_robots) {
    robots->RemoveLast();
  }

  delete[] markers;
}



};
















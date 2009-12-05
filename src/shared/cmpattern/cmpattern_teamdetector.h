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
  \file    cmpattern_teamdetector.h
  \brief   C++ Interface: teamdetector
  \author  Original CMDragons Robot Detection Code (C), James Bruce
           ssl-vision restructuring and modifications, Stefan Zickler, 2009
*/
//========================================================================
#ifndef CM_PATTERN_TEAMDETECTOR_H
#define CM_PATTERN_TEAMDETECTOR_H
#include "VarTypes.h"
#include "messages_robocup_ssl_detection.pb.h"
#include "image.h"
#include "lut3d.h"
#include "cmpattern_team.h"
#include "cmpattern_pattern.h"
#include "cmvision_region.h"
#include "field.h"
#include "camera_calibration.h"
#include "field_filter.h"
#include "vis_util.h"
#include "cmvision_histogram.h"
#include <string.h>
#include <vector>
#include <QObject>

using namespace std;
namespace CMPattern {

class TeamDetectorInterface {
public:
  virtual VarList * getSettings() { return 0; };
  virtual void update(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots) { (void)robots; };
  virtual ~TeamDetectorInterface() {};
};


class TeamDetector;
class TeamSelector;

class TeamDetectorSettings : public QObject {
Q_OBJECT
 friend class TeamDetector;
 friend class TeamSelector;
 protected:
 VarList * settings;
 VarList * teams; // a global variable, defining all teams
 VarTrigger * addTeam;
 vector<Team *> team_vector;
signals:
  void teamInfoChanged();
 protected slots:
   void slotTeamNodeAdded(VarType * node);
   void slotAddPressed();
 public:
 vector<Team *> getTeams() const;
 Team * getTeam(int idx) const;
 TeamDetectorSettings(string external_file="");
 VarList * getSettings() {
   return settings;
 }
 //TODO: add notifier factory.

};


class TeamSelector : public QObject {
Q_OBJECT
signals:
  void signalTeamDataChanged();
protected:
  TeamDetectorSettings * _detector_settings;
  VarList * _settings;
  VarStringEnum * _selector;
  VarInt * _num_robots;
  Team * current_team;
  void update() {
    vector<Team *> teams = _detector_settings->getTeams();
    _selector->setSize(teams.size(), "");
    int old_select_index = _selector->getIndex();
    for (unsigned int i=0;i<teams.size();i++) {
      _selector->setLabel(i,teams[i]-> _team_name->getString());
    }
    Team * new_team=0;
    if (teams.size() > 0) {
      if (old_select_index >= 0 && old_select_index < (int)teams.size()) {
         _selector->selectIndex(old_select_index);
      } else {
         _selector->selectIndex(0);
      }
      new_team=_detector_settings->getTeam(_selector->getIndex());
    } else {
      new_team=0;
    }
    if (new_team!=current_team) {
      if (current_team!=0) {
        disconnect(current_team,SIGNAL(signalChangeOccured(VarType*)),this,SLOT(slotTeamDataChanged()));
      }
      if (new_team!=0) {
        connect(new_team,SIGNAL(signalChangeOccured(VarType*)),this,SLOT(slotTeamDataChanged()));
      }
      current_team=new_team;
    }
    emit(signalTeamDataChanged());
  }
protected slots:
  void slotTeamInfoChanged() {
    update();
  }
  void slotTeamDataChanged() {
    emit(signalTeamDataChanged());
  }
public:
  TeamSelector(string label, TeamDetectorSettings * detector_settings) {
    _detector_settings=detector_settings;
    connect(_detector_settings,SIGNAL(teamInfoChanged()),this,SLOT(slotTeamInfoChanged()));
    current_team=0;
    _settings= new VarList(label);
    _settings->addChild(_selector = new VarStringEnum("Team",""));
    _selector->addFlags(VARTYPE_FLAG_NOLOAD_ENUM_CHILDREN);
    _settings->addChild(_num_robots = new VarInt("Max Robots",5));
    update();
  }
  VarList * getSettings() {
    return _settings;
  }
  Team * getSelectedTeam() {
    if (_selector->getIndex() == -1) {
      update();
    } else if (current_team!=_detector_settings->getTeam(_selector->getIndex())) {
      update();
    }
    return current_team;
  }
  int getNumberRobots() {
    return _num_robots->getInt();
  }
  ~TeamSelector() {
    delete _settings;
  }
};



class TeamDetector : public TeamDetectorInterface {
protected:

  //TeamDetectorSettings * _detector_settings;

  const CameraParameters& _camera_params;
  const RoboCupField& _field;
  Team * _team;
  LUT3D * _lut3d;
  FieldFilter field_filter;
  MultiPatternModel model;

  //-----TEAM CONFIG---------
  CMVision::RegionFilter filter_team;
  CMVision::RegionFilter filter_others;
  bool   _unique_patterns;
  bool   _have_angle;
  bool   _load_markers_from_image_file;
  string _marker_image_file;
  int    _marker_image_rows;
  int    _marker_image_cols;

  int    _max_robots;
  double _robot_height;

  double _center_marker_area_mean;
  double _center_marker_area_stddev;
  double _center_marker_uniform;
  double _center_marker_duplicate_distance;

  bool  _histogram_enable;
  int    _histogram_pixel_scan_radius;

  ClosedRangeFloat _histogram_markeryness;
  ClosedRangeFloat _histogram_field_greenness;
  ClosedRangeFloat _histogram_black_whiteness;

  double _pattern_max_dist;
  MultiPatternModel::PatternFitParameters _pattern_fit_params;

  //----END OF TEAM CONFIG---------

  //color ids:
  int color_id_cyan;
  int color_id_pink;
  int color_id_green;
  int color_id_field_green;
  int color_id_black;
  int color_id_clear;
  int color_id_white;
  int color_id_team;

protected:
    double getRegionArea(const CMVision::Region * reg, double z) const;
    bool checkHistogram(const CMVision::Region * reg, const Image<raw8> * image);

    //returns a mutable pointer if the add was successful
    //returns 0 if there already are max_robots with higher confidence than conf
    SSL_DetectionRobot * addRobot(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots, double conf, int max_robots);

    //remove anything with a confidence of 0:
    void stripRobots(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots);

public:
    TeamDetector(LUT3D * lut3d, const CameraParameters& camera_params, const RoboCupField& field);

    virtual ~TeamDetector();

    VarList * getSettings() {
      return 0;
    }
    CMVision::Histogram * histogram;

    void init(Team * team);

    void findRobotsByModel(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots, int team_color_id, const Image<raw8> * image, CMVision::ColorRegionList * colorlist, CMVision::RegionTree & reg_tree);

    void findRobotsByTeamMarkerOnly(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots, int team_color_id, const Image<raw8> * image, CMVision::ColorRegionList * colorlist);

    void update(::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot >* robots, int team_color_id, int max_robots, const Image<raw8> * image, CMVision::ColorRegionList * colorlist, CMVision::RegionTree & reg_tree);
};

}
#endif

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
  \file    cmpattern_team.h
  \brief   C++ Interface: team
  \author  Stefan Zickler, 2009
*/
//========================================================================
#ifndef CM_PATTERN_TEAM_H
#define CM_PATTERN_TEAM_H
#include <QObject>
#include "VarTypes.h"
#include "geometry.h"
#include "VarNotifier.h"
using namespace VarTypes;
namespace CMPattern {

class TeamSelector;
class TeamDetector;

class Team : public QObject {
Q_OBJECT

signals:
    void signalTeamNameChanged();
    void signalChangeOccured(VarType * item);
protected slots:
    void slotTeamNameChanged();
    void slotChangeOccured(VarType * item);

protected:
    VarNotifier _notifier;
    VarList * _settings;

public:
    VarString * _team_name;
    VarDouble * _robot_height;

    Team(VarList * team_root);
    ~Team();
};

class RobotPattern : public QObject {
Q_OBJECT

friend class TeamSelector;
friend class TeamDetector;
signals:
   void signalChangeOccured(VarType * item);
protected slots:
   void slotChangeOccured(VarType * item);
protected:
  VarNotifier _notifier;
  VarList * _settings;
    VarBool * _unique_patterns;
    VarBool * _have_angle;
    VarBool * _load_markers_from_image_file;
    VarString * _marker_image_file;
    VarInt *    _marker_image_rows;
    VarInt *    _marker_image_cols;
    VarSelection * _valid_patterns;
    VarList * _marker_image;

    VarList * _center_marker_filter;
      VarDouble * _center_marker_area_mean;
      VarDouble * _center_marker_area_stddev;
      VarDouble * _center_marker_uniform;
      VarInt * _center_marker_min_width;
      VarInt * _center_marker_max_width;
      VarInt * _center_marker_min_height;
      VarInt * _center_marker_max_height;
      VarInt * _center_marker_min_area;
      VarInt * _center_marker_max_area;
      VarInt * _center_marker_duplicate_distance;

    VarList * _other_markers_filter;
      VarInt * _other_markers_min_width;
      VarInt * _other_markers_max_width;
      VarInt * _other_markers_min_height;
      VarInt * _other_markers_max_height;
      VarInt * _other_markers_min_area;
      VarInt * _other_markers_max_area;
      VarInt * _other_markers_max_detections;
      VarDouble * _other_markers_max_query_distance;

    VarList * _histogram_settings;
      VarBool * _histogram_enable;
      VarInt * _histogram_pixel_scan_radius;
      VarDouble * _histogram_min_markeryness;
      VarDouble * _histogram_max_markeryness;
      VarDouble * _histogram_min_field_greenness;
      VarDouble * _histogram_max_field_greenness;
      VarDouble * _histogram_min_black_whiteness;
      VarDouble * _histogram_max_black_whiteness;

    VarList * _pattern_fitness;
      VarDouble * _pattern_max_dist;
      VarDouble * _pattern_fitness_weight_area;
      VarDouble * _pattern_fitness_weight_center_distance;
      VarDouble * _pattern_fitness_weight_next_distance;
      VarDouble * _pattern_fitness_weight_next_angle_distance;
      VarDouble * _pattern_fitness_max_error;
      VarDouble * _pattern_fitness_stddev;
      VarDouble * _pattern_fitness_uniform;

public:
    RobotPattern(VarList * team_root);

    ~RobotPattern();

};

}
#endif

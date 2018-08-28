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
  \file    cmpattern_team.cpp
  \brief   C++ Implementation: team
  \author  Stefan Zickler, 2009
*/
//========================================================================
#include "cmpattern_team.h"

namespace CMPattern {

void Team::slotTeamNameChanged() {
  if (_settings!=0) {
    _settings->setName(_team_name->getString());
  }
  emit(signalTeamNameChanged());
}

void RobotPattern::slotChangeOccured(VarType * item) {
  emit(signalChangeOccured(item));
}

void Team::slotChangeOccured(VarType * item) {
    emit(signalChangeOccured(item));
}


Team::Team(VarList * team_root)
{
    _settings=team_root;
    _team_name = _settings->findChildOrReplace(new VarString("Team Name", team_root->getName()));
    connect(_team_name,SIGNAL(hasChanged(VarType *)),this,SLOT(slotTeamNameChanged()));
    _robot_height = _settings->findChildOrReplace(new VarDouble("Robot Height (mm)", 140.0));

    _notifier.addRecursive(_settings);
    connect(&_notifier,SIGNAL(changeOccured(VarType*)),this,SLOT(slotChangeOccured(VarType *)));
}

Team::~Team() = default;

RobotPattern::RobotPattern(VarList * team_root)
{
    _settings=team_root;
    _unique_patterns = _settings->findChildOrReplace(new VarBool("Unique Patterns", true));
    _have_angle = _settings->findChildOrReplace(new VarBool("Have Angles", true));

    _marker_image = _settings->findChildOrReplace(new VarList("Marker Image"));
      _load_markers_from_image_file = _marker_image->findChildOrReplace(new VarBool("Load Image File",true));
      _marker_image_file = _marker_image->findChildOrReplace(new VarString("Marker Image File", "patterns/teams/standard2010_16.png"));
      _marker_image_rows = _marker_image->findChildOrReplace(new VarInt("Marker Image Rows",4));
      _marker_image_cols = _marker_image->findChildOrReplace(new VarInt("Marker Image Cols",4));
      _valid_patterns = _marker_image->findChildOrReplace(new VarSelection("Valid Patterns",16,true));
      _valid_patterns->addFlags(VARTYPE_FLAG_PERSISTENT);

    _center_marker_filter = _settings->findChildOrReplace(new VarList("Center Marker Settings"));
      _center_marker_area_mean = _center_marker_filter->findChildOrReplace(new VarDouble("Expected Area Mean (sq-mm)",sq(50.0)));
      _center_marker_area_stddev = _center_marker_filter->findChildOrReplace(new VarDouble("Expected StdDev (sq-mm)",sq(30.0)));
      _center_marker_uniform = _center_marker_filter->findChildOrReplace(new VarDouble("Uniform",0.05));
      _center_marker_min_width = _center_marker_filter->findChildOrReplace(new VarInt("Min Width (pixels)",4));
      _center_marker_max_width = _center_marker_filter->findChildOrReplace(new VarInt("Max Width (pixels)",30));
      _center_marker_min_height = _center_marker_filter->findChildOrReplace(new VarInt("Min Height (pixels)",4));
      _center_marker_max_height = _center_marker_filter->findChildOrReplace(new VarInt("Max Height (pixels)",30));
      _center_marker_min_area = _center_marker_filter->findChildOrReplace(new VarInt("Min Area (sq-pixels)",15));
      _center_marker_max_area = _center_marker_filter->findChildOrReplace(new VarInt("Max Area (sq-pixels)",400));
      _center_marker_duplicate_distance = _center_marker_filter->findChildOrReplace(new VarInt("Duplicate Merge Distance (mm)",135));

    _other_markers_filter = _settings->findChildOrReplace(new VarList("Other Markers Settings"));
      _other_markers_min_width = _other_markers_filter->findChildOrReplace(new VarInt("Min Width (pixels)",4));
      _other_markers_max_width = _other_markers_filter->findChildOrReplace(new VarInt("Max Width (pixels)",40));
      _other_markers_min_height = _other_markers_filter->findChildOrReplace(new VarInt("Min Height (pixels)",4));
      _other_markers_max_height = _other_markers_filter->findChildOrReplace(new VarInt("Max Height (pixels)",40));
      _other_markers_min_area = _other_markers_filter->findChildOrReplace(new VarInt("Min Area (sq-pixels)",15));
      _other_markers_max_area = _other_markers_filter->findChildOrReplace(new VarInt("Max Area (sq-pixels)",600));
      _other_markers_max_detections = _other_markers_filter->findChildOrReplace(new VarInt("Max Num Markers To Detect", 16));
      _other_markers_max_query_distance = _other_markers_filter->findChildOrReplace(new VarDouble("Max Query Distance", 20.0));

    _histogram_settings = _settings->findChildOrReplace(new VarList("Histogram Settings"));
      _histogram_enable = _histogram_settings->findChildOrReplace(new VarBool("Enable",true));
      _histogram_pixel_scan_radius = _histogram_settings->findChildOrReplace(new VarInt("Scan Radius (pixels)",16));
      _histogram_min_markeryness = _histogram_settings->findChildOrReplace(new VarDouble("Min Markeryness",0.3));
      _histogram_max_markeryness = _histogram_settings->findChildOrReplace(new VarDouble("Max Markeryness",12.0));
      _histogram_min_field_greenness = _histogram_settings->findChildOrReplace(new VarDouble("Min Field-Greenness",0.0,0.0,1.0));
      _histogram_max_field_greenness = _histogram_settings->findChildOrReplace(new VarDouble("Max Field-Greenness",1.0,0.0,1.0));
      _histogram_min_black_whiteness = _histogram_settings->findChildOrReplace(new VarDouble("Min Black/Whiteness",0.0,0.0,1.0));
      _histogram_max_black_whiteness = _histogram_settings->findChildOrReplace(new VarDouble("Max Black/Whiteness",1.0,0.0,1.0));

    _pattern_fitness = _settings->findChildOrReplace(new VarList("Pattern Fitting"));
      _pattern_max_dist = _pattern_fitness->findChildOrReplace(new VarDouble("Max Marker Center Dist (mm)",100));
      _pattern_fitness_weight_area = _pattern_fitness->findChildOrReplace(new VarDouble("Weight Area",0.1));
      _pattern_fitness_weight_center_distance = _pattern_fitness->findChildOrReplace(new VarDouble("Weight Center-Dist",0.2));
      _pattern_fitness_weight_next_distance = _pattern_fitness->findChildOrReplace(new VarDouble("Weight Next-Dist",1.0));
      _pattern_fitness_weight_next_angle_distance = _pattern_fitness->findChildOrReplace(new VarDouble("Weight Next-Angle-Dist",0.5));
      _pattern_fitness_max_error = _pattern_fitness->findChildOrReplace(new VarDouble("Max Error",1.0));
      _pattern_fitness_stddev = _pattern_fitness->findChildOrReplace(new VarDouble("Expected StdDev",0.5));
      _pattern_fitness_uniform = _pattern_fitness->findChildOrReplace(new VarDouble("Uniform",0.05));

  _notifier.addRecursive(_settings);
  connect(&_notifier,SIGNAL(changeOccured(VarType*)),this,SLOT(slotChangeOccured(VarType *)));
}


RobotPattern::~RobotPattern()
{
}


}

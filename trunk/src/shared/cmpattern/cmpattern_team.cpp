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
  \file    team.cpp
  \brief   C++ Implementation: team
  \author  Author Name, 2009
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


Team::Team(VarList * team_root)
{
    _settings=team_root;
    _team_name = _settings->findChildOrReplace(new VarString("Team Name", team_root->getName()));
    connect(_team_name,SIGNAL(hasChanged()),this,SLOT(slotTeamNameChanged()));
    _unique_patterns = _settings->findChildOrReplace(new VarBool("Unique Patterns"));
    _have_angle = _settings->findChildOrReplace(new VarBool("Have Angles"));
    _load_markers_from_image_file = _settings->findChildOrReplace(new VarBool("Load Image File",true));
    _marker_image_file = _settings->findChildOrReplace(new VarString("Marker Image File"));
    _robot_height = _settings->findChildOrReplace(new VarDouble("Robot Height (mm)", 140.0));

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
      _other_markers_min_width = _other_markers_filter->findChildOrReplace(new VarInt("Min Width (pixels)"));
      _other_markers_max_width = _other_markers_filter->findChildOrReplace(new VarInt("Max Width (pixels)"));
      _other_markers_min_height = _other_markers_filter->findChildOrReplace(new VarInt("Min Height (pixels)"));
      _other_markers_max_height = _other_markers_filter->findChildOrReplace(new VarInt("Max Height (pixels)"));
      _other_markers_min_area = _other_markers_filter->findChildOrReplace(new VarInt("Min Area (sq-pixels)"));
      _other_markers_max_area = _other_markers_filter->findChildOrReplace(new VarInt("Max Area (sq-pixels)"));

    _histogram_settings = _settings->findChildOrReplace(new VarList("Histogram Settings"));
      _histogram_enable = _histogram_settings->findChildOrReplace(new VarBool("Enable",true));
      _histogram_pixel_scan_radius = _histogram_settings->findChildOrReplace(new VarInt("Scan Radius (pixels)",16));
      _histogram_min_markeryness = _histogram_settings->findChildOrReplace(new VarDouble("Min Markeryness",0.3));
      _histogram_max_markeryness = _histogram_settings->findChildOrReplace(new VarDouble("Max Markeryness",12.0));
      _histogram_min_field_greenness = _histogram_settings->findChildOrReplace(new VarDouble("Min Field-Greenness",0.0));
      _histogram_max_field_greenness = _histogram_settings->findChildOrReplace(new VarDouble("Max Field-Greenness",2.0));
      _histogram_min_black_whiteness = _histogram_settings->findChildOrReplace(new VarDouble("Min Black/Whiteness",0.0));
      _histogram_max_black_whiteness = _histogram_settings->findChildOrReplace(new VarDouble("Max Black/Whiteness",12.0));

    _pattern_fitness = _settings->findChildOrReplace(new VarList("Pattern Fitting"));
      _pattern_fitness_weight_area = _pattern_fitness->findChildOrReplace(new VarDouble("Weight Area"));
      _pattern_fitness_weight_center_distance = _pattern_fitness->findChildOrReplace(new VarDouble("Weight Center-Dist"));
      _pattern_fitness_weight_next_distance = _pattern_fitness->findChildOrReplace(new VarDouble("Weight Next-Dist"));
      _pattern_fitness_max_error = _pattern_fitness->findChildOrReplace(new VarDouble("Max Error"));
      _pattern_fitness_variance = _pattern_fitness->findChildOrReplace(new VarDouble("Expected Variance"));
      _pattern_fitness_uniform = _pattern_fitness->findChildOrReplace(new VarDouble("Uniform"));


///BACKUP OLD:
/*
  _settings = new VarList("settings");
    _team_name = new VarString("Team Name");
    _unique_patterns = new VarBool("Unique Patterns");
    _have_angle = new VarBool("Have Angles");
    _load_markers_from_image_file = new VarBool("Load Image File",true);
    _marker_image_file = new VarString("Marker Image File");
    _robot_height = new VarDouble("Robot Height (mm)", 140.0);

    _center_marker_filter = new VarList("Center Marker Settings");
      _center_marker_filter->addChild(_center_marker_area_mean = new VarDouble("Expected Area Mean"));
      _center_marker_filter->addChild(_center_marker_area_stddev = new VarDouble("Expected Area Variance"));
      _center_marker_filter->addChild(_center_marker_uniform = new VarDouble("Uniform"));
      _center_marker_filter->addChild(_center_marker_min_width = new VarInt("Min Width (pixels)"));
      _center_marker_filter->addChild(_center_marker_max_width = new VarInt("Max Width (pixels)"));
      _center_marker_filter->addChild(_center_marker_min_height = new VarInt("Min Height (pixels)"));
      _center_marker_filter->addChild(_center_marker_max_height = new VarInt("Max Height (pixels)"));
      _center_marker_filter->addChild(_center_marker_min_area = new VarInt("Min Area (sq-pixels)"));
      _center_marker_filter->addChild(_center_marker_max_area = new VarInt("Max Area (sq-pixels)"));

    _other_markers_filter = new VarList("Other Markers Settings");
      _other_markers_filter->addChild(_other_markers_min_width = new VarInt("Min Width (pixels)"));
      _other_markers_filter->addChild(_other_markers_max_width = new VarInt("Max Width (pixels)"));
      _other_markers_filter->addChild(_other_markers_min_height = new VarInt("Min Height (pixels)"));
      _other_markers_filter->addChild(_other_markers_max_height = new VarInt("Max Height (pixels)"));
      _other_markers_filter->addChild(_other_markers_min_area = new VarInt("Min Area (sq-pixels)"));
      _other_markers_filter->addChild(_other_markers_max_area = new VarInt("Max Area (sq-pixels)"));

    _histogram_settings = new VarList("Histogram Settings");
      _histogram_settings->addChild(_histogram_enable = new VarBool("Enable"));
      _histogram_settings->addChild(_histogram_pixel_scan_radius = new VarInt("Scan Radius (pixels)"));
      _histogram_settings->addChild(_histogram_min_markeryness = new VarDouble("Min Markeryness"));
      _histogram_settings->addChild(_histogram_max_markeryness = new VarDouble("Max Markeryness"));
      _histogram_settings->addChild(_histogram_min_field_greenness = new VarDouble("Min Greenness"));
      _histogram_settings->addChild(_histogram_max_field_greenness = new VarDouble("Max Greenness"));
      _histogram_settings->addChild(_histogram_min_black_whiteness = new VarDouble("Min Black/Whiteness"));
      _histogram_settings->addChild(_histogram_max_black_whiteness = new VarDouble("Max Black/Whiteness"));

    _pattern_fitness = new VarList("Pattern Fitting");
      _pattern_fitness->addChild(_pattern_fitness_weight_area = new VarDouble("Weight Area"));
      _pattern_fitness->addChild(_pattern_fitness_weight_center_distance = new VarDouble("Weight Center-Dist"));
      _pattern_fitness->addChild(_pattern_fitness_weight_next_distance = new VarDouble("Weight Next-Dist"));
      _pattern_fitness->addChild(_pattern_fitness_max_error = new VarDouble("Max Error"));
      _pattern_fitness->addChild(_pattern_fitness_variance = new VarDouble("Expected Variance"));
      _pattern_fitness->addChild(_pattern_fitness_uniform = new VarDouble("Uniform"));

*/
}


Team::~Team()
{
}


}

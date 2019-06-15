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
  \file    plugin_detect_balls.cpp
  \brief   C++ Implementation: plugin_detect_balls
  \author  Author Name, 2009
*/
//========================================================================
#include <list>
#include "plugin_detect_balls.h"

PluginDetectBalls::PluginDetectBalls ( FrameBuffer * _buffer, LUT3D * lut, const CameraParameters& camera_params, const RoboCupField& field,PluginDetectBallsSettings * settings )
    : VisionPlugin ( _buffer ), camera_parameters ( camera_params ), field ( field ) {
  _lut=lut;

  _settings=settings;
  _have_local_settings=false;
  if ( _settings==0 ) {
    _settings = new PluginDetectBallsSettings();
    _have_local_settings=true;
  }

  vnotify.addRecursive(_settings->getSettings());
  vnotify.addRecursive(field.getSettings());


  //read-out important LUT data:
  histogram = new CMVision::Histogram ( _lut->getChannelCount() );
  color_id_orange = _lut->getChannelID ( "Orange" );
  if ( color_id_orange == -1 ) printf ( "WARNING color label 'Orange' not defined in LUT!!!\n" );
  color_id_pink = _lut->getChannelID ( "Pink" );
  if ( color_id_pink == -1 ) printf ( "WARNING color label 'Pink' not defined in LUT!!!\n" );
  color_id_yellow = _lut->getChannelID ( "Yellow" );
  if ( color_id_yellow == -1 ) printf ( "WARNING color label 'Yellow' not defined in LUT!!!\n" );
  color_id_field = _lut->getChannelID ( "Field Green" );
  if ( color_id_field == -1 ) printf ( "WARNING color label 'Field Green' not defined in LUT!!!\n" );




}


PluginDetectBalls::~PluginDetectBalls() {
  delete histogram;
}


VarList * PluginDetectBalls::getSettings() {
  if ( _have_local_settings ) {
    return _settings->getSettings();
  } else {
    return 0;
  }
}

string PluginDetectBalls::getName() {
  return "DetectBalls";
}

bool PluginDetectBalls::checkHistogram ( const Image<raw8> * image, const CMVision::Region * reg, double min_greenness, double max_markeryness ) {
  static const int PixelRadius = 4;

  histogram->clear();

  int num = histogram->addBox ( image, reg->x1 - PixelRadius, reg->y1 - PixelRadius,
                                reg->x2 + PixelRadius, reg->y2 + PixelRadius );


  float pf = ( float ) ( histogram->getChannel ( color_id_pink ) ) / ( float ) ( histogram->getChannel ( color_id_orange ) );
  float yf = ( float ) ( histogram->getChannel ( color_id_yellow ) ) / ( float ) ( histogram->getChannel ( color_id_orange ) );
  float markeryness = ( pf + 1 ) * ( yf + 1 ) - 1;
  float greenness = ( float ) ( histogram->getChannel ( color_id_field ) ) / ( ( ( float ) ( num - histogram->getChannel ( color_id_orange ) ) ) + 1E-6 );

  if ( greenness   > min_greenness ) return ( true );
  if ( markeryness > max_markeryness ) return ( false );
  return ( true );
}

//Data structure for storing and sorting the filtered regions
class BallDetectResult
{
public:
  const CMVision::Region* reg;
  float conf;

  BallDetectResult(const CMVision::Region* reg, float conf) {
    this->reg = reg;
    this->conf = conf;    
  }

  bool operator< (BallDetectResult a) {
    return conf < a.conf;
  }
};

ProcessResult PluginDetectBalls::process ( FrameData * data, RenderOptions * options ) {
  ( void ) options;
  if ( data==0 ) return ProcessingFailed;

  SSL_DetectionFrame * detection_frame = 0;

  detection_frame= ( SSL_DetectionFrame * ) data->map.get ( "ssl_detection_frame" );
  if ( detection_frame == 0 ) detection_frame= ( SSL_DetectionFrame * ) data->map.insert ( "ssl_detection_frame",new SSL_DetectionFrame() );

  int color_id_ball = _lut->getChannelID ( _settings->_color_label->getString() );
  if ( color_id_ball == -1 ) {
    printf ( "Unknown Ball Detection Color Label: '%s'\nAborting Plugin!\n",_settings->_color_label->getString().c_str() );
    return ProcessingFailed;
  }

  //delete any previous detection results:
  detection_frame->clear_balls();

  //TODO: add a vartype notifier for better performance.

  //initialize filter:
  if (vnotify.hasChanged()) {
    max_balls = _settings->_max_balls->getInt();
    filter.setWidth ( _settings->_ball_min_width->getInt(),_settings->_ball_max_width->getInt() );
    filter.setHeight ( _settings->_ball_min_height->getInt(),_settings->_ball_max_height->getInt() );
    filter.setArea ( _settings->_ball_min_area->getInt(),_settings->_ball_max_area->getInt() );
    field_filter.update ( field );

    //copy all vartypes to local variables for faster repeated lookup:
    filter_ball_in_field = _settings->_ball_on_field_filter->getBool();
    filter_ball_on_field_filter_threshold = _settings->_ball_on_field_filter_threshold->getDouble();
    filter_ball_in_goal  = _settings->_ball_in_goal_filter->getBool();
    filter_ball_histogram = _settings->_ball_histogram_enabled->getBool();
    if ( filter_ball_histogram ) {
      if ( color_id_ball != color_id_orange ) {
        printf ( "Warning: ball histogram check is only configured for orange balls!\n" );
        printf ( "Please disable the histogram check in the Ball Detection Plugin settings\n" );
      }
      if ( color_id_pink==-1 || color_id_orange==-1 || color_id_yellow==-1 || color_id_field==-1 ) {
        printf ( "WARNING: some LUT color labels where undefined for the ball detection plugin\n" );
        printf ( "         Disabling histogram check!\n" );
        filter_ball_histogram=false;
      }
    }

    min_greenness = _settings->_ball_histogram_min_greenness->getDouble();
    max_markeryness = _settings->_ball_histogram_max_markeryness->getDouble();

    //setup values used for the gaussian confidence measurement:
    filter_gauss = _settings->_ball_gauss_enabled->getBool();
    exp_area_min =  _settings->_ball_gauss_min->getInt();
    exp_area_max = _settings->_ball_gauss_max->getInt();
    exp_area_var = sq ( _settings->_ball_gauss_stddev->getDouble() );
    z_height= _settings->_ball_z_height->getDouble();

    near_robot_filter = _settings->_ball_too_near_robot_enabled->getBool();
    near_robot_dist_sq = sq(_settings->_ball_too_near_robot_dist->getDouble());
  }

  const CMVision::Region * reg = 0;

  //acquire orange region list from data-map:
  CMVision::ColorRegionList * colorlist;
  colorlist= ( CMVision::ColorRegionList * ) data->map.get ( "cmv_colorlist" );
  if ( colorlist==0 ) {
    printf ( "error in ball detection plugin: no region-lists were found!\n" );
    return ProcessingFailed;
  }
  reg = colorlist->getRegionList ( color_id_ball ).getInitialElement();

  //acquire color-labeled image from data-map:
  const Image<raw8> * image = ( Image<raw8> * ) ( data->map.get ( "cmv_threshold" ) );
  if ( image==0 ) {
    printf ( "error in ball detection plugin: no color-thresholded image was found!\n" );
    return ProcessingFailed;
  }

  int robots_blue_n=0;
  int robots_yellow_n=0;
  bool use_near_robot_filter=near_robot_filter;
  if ( use_near_robot_filter ) {
    SSL_DetectionFrame * detection_frame = ( SSL_DetectionFrame * ) data->map.get ( "ssl_detection_frame" );
    if ( detection_frame==0 ) {
      use_near_robot_filter=false;
    } else {
      robots_blue_n=detection_frame->robots_blue_size();
      robots_yellow_n=detection_frame->robots_yellow_size();
      if (robots_blue_n==0 && robots_yellow_n==0) use_near_robot_filter=false;
    }
  }

  if ( max_balls > 0 ) {
    list<BallDetectResult> result;
    filter.init ( reg );
    
    while ( ( reg = filter.getNext() ) != 0 ) {
      float conf = 1.0;

      if ( filter_gauss==true ) {
        int a = reg->area - bound ( reg->area,exp_area_min,exp_area_max );
        conf = gaussian ( a / exp_area_var );
      }

      //TODO: add a plugin for confidence masking... possibly multi-layered.
      //      to replace the commented det.mask.get(...) below:
      //if (filter_conf_mask) conf*=det.mask.get(reg->cen_x,reg->cen_y));

      //convert from image to field coordinates:
      vector2d pixel_pos ( reg->cen_x,reg->cen_y );
      vector3d field_pos_3d;
      camera_parameters.image2field ( field_pos_3d,pixel_pos,z_height );
      vector2d field_pos ( field_pos_3d.x,field_pos_3d.y );

      //filter points that are outside of the field:
      if ( filter_ball_in_field==true && field_filter.isInFieldPlusThreshold ( field_pos, max(0.0,filter_ball_on_field_filter_threshold) ) ==false ) {
        conf = 0.0;
      }

      //filter out points that are deep inside the goal-box
      if ( filter_ball_in_goal==true && field_filter.isFarInGoal ( field_pos ) ==true ) {
        conf = 0.0;
      }

      //TODO add ball-too-near-robot filter
      if ( use_near_robot_filter && conf > 0.0 ) {
        int robots_n=0;
        for (int team = 0; team < 2; team++) {
          if (team==0) {
            robots_n=robots_blue_n;
          } else {
            robots_n=robots_yellow_n;
          }
          if (robots_n > 0) {
            ::google::protobuf::RepeatedPtrField< ::SSL_DetectionRobot > * robots;
            if (team==0) {
              robots = detection_frame->mutable_robots_blue();
            } else {
              robots = detection_frame->mutable_robots_yellow();
            }
            for (int r = 0; r < robots_n; r++) {
              const SSL_DetectionRobot & robot = robots->Get(r);
              if (robot.confidence() > 0.0) {
                vector3d field_on_bot_pos_3d;
                camera_parameters.image2field ( field_on_bot_pos_3d, pixel_pos, robot.height());
                if ((sq((double)(robot.x())-(double)(field_on_bot_pos_3d.x)) + sq((double)(robot.y())-(double)(field_on_bot_pos_3d.y))) < near_robot_dist_sq) {
                  conf = 0.0;
                  break;
                }
              }
            }
            if (conf==0.0) break;
          }
        }
      }

      // histogram check if enabled
      if ( filter_ball_histogram && conf > 0.0 && checkHistogram ( image, reg, min_greenness, max_markeryness ) ==false ) {
        conf = 0.0;
      }

      // add filtered region to the region list
      if(conf > 0) {
        result.push_back(BallDetectResult(reg,conf));
      }

    }

    // sort result by confidence and output first max_balls region(s)
    result.sort();
    
    int num_ball = 0;
    list<BallDetectResult>::reverse_iterator it;
    for(it=result.rbegin(); it!=result.rend(); it++) {
      if(++num_ball > max_balls)
        break;

      //update result:
      SSL_DetectionBall* ball = detection_frame->add_balls();

      ball->set_confidence ( it->conf );

      vector2d pixel_pos ( it->reg->cen_x,it->reg->cen_y );
      vector3d field_pos_3d;
      camera_parameters.image2field ( field_pos_3d,pixel_pos,z_height );

      ball->set_area ( it->reg->area );
      ball->set_x ( field_pos_3d.x );
      ball->set_y ( field_pos_3d.y );
      ball->set_pixel_x ( it->reg->cen_x );
      ball->set_pixel_y ( it->reg->cen_y );
    }

  }

  return ProcessingOk;

}


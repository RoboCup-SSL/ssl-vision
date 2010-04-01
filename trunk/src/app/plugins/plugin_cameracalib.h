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
  \file    plugin_cameracalib.h
  \brief   C++ Interface: plugin_cameracalib
  \author  Tim Laue, 2009
*/
//========================================================================
#ifndef PLUGIN_CAMERACALIB_H
#define PLUGIN_CAMERACALIB_H

#include "framedata.h"
#include "VarTypes.h"
#include "visionplugin.h"
#include "camera_calibration.h"
#include "field.h"
#include "image.h"
#include "cameracalibwidget.h"

/**
*	@author Tim Laue <Tim.Laue@dfki.de>
*/
class PluginCameraCalibration : public VisionPlugin 
{
protected:
  VarList* settings;
  VarList* camera_settings;
  VarList* field_settings;
  VarList* calibration_settings;
  CameraParameters& camera_parameters;
  RoboCupCalibrationHalfField& field;
  CameraCalibrationWidget * ccw;
  greyImage* grey_image;
  rgbImage* rgb_image;
  int video_width;
  int video_height;
  void mouseEvent ( QMouseEvent * event, pixelloc loc );
  enum ImageSide {IMG_LEFT, IMG_RIGHT, IMG_TOP, IMG_BOTTOM};

  bool doing_drag;
  VarDouble* drag_x;
  VarDouble* drag_y;

  void sanitizeSobel(greyImage * img, GVector::vector2d<double> & val,int sobel_border=1);
    
  ImageSide getImageSide(const GVector::vector3d<double>& start,
                         const GVector::vector3d<double>& end);
  
  void detectEdges(FrameData * data);
  void detectEdgesOnSingleArc(const GVector::vector3d<double>& center,
                              double radius, double theta1, double theta2,
                              int numPoints);
  void detectEdgesOnSingleLine(const GVector::vector3d<double>& start,
                               const GVector::vector3d<double>& end,
                               int pointsPerLine, bool detectCenter = false);

public:
  PluginCameraCalibration(FrameBuffer * _buffer, CameraParameters& camera_params, RoboCupCalibrationHalfField& _field);
  ~PluginCameraCalibration();
  virtual ProcessResult process(FrameData * data, RenderOptions * options);
  virtual VarList * getSettings();
  virtual std::string getName();
  virtual QWidget * getControlWidget();

  virtual void keyPressEvent ( QKeyEvent * event );
  virtual void mousePressEvent ( QMouseEvent * event, pixelloc loc );
  virtual void mouseReleaseEvent ( QMouseEvent * event, pixelloc loc );
  virtual void mouseMoveEvent ( QMouseEvent * event, pixelloc loc );
  
};

#endif //PLUGIN_CAMERACALIB_H

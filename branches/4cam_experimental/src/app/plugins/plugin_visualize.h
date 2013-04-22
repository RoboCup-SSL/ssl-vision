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
  \file    plugin_visualize.h
  \brief   C++ Interface: plugin_visualize
  \author  Stefan Zickler, 2008
*/
//========================================================================
#ifndef PLUGIN_VISUALIZE_H
#define PLUGIN_VISUALIZE_H

#include <visionplugin.h>
#include "image.h"
#include "conversions.h"
#include "lut3d.h"
#include "cmvision_region.h"
#include "camera_calibration.h"
#include "field.h"

/**
	@author Stefan Zickler
	\brief This plugin creates an image copy used solely for visualization purposes.
	       It should furthermore allow optional rendering of
            - undistortion,
            - thresholded color output,
            - other things...
         It operates by checking what data is currently available in the framedata
            container and will then offer visualization options accordingly.
*/

class VisualizationFrame {
  public:
    rgbImage data;
    bool valid;
    VisualizationFrame() {
      valid=false;
    }
};

class PluginVisualize : public VisionPlugin
{
protected:
  VarList * _settings;
  VarBool * _v_enabled;
  VarBool * _v_image;
  VarBool * _v_greyscale;
  VarBool * _v_thresholded;
  VarBool * _v_blobs;
  VarBool * _v_camera_calibration;
  VarBool * _v_calibration_result;
  VarBool * _v_complete_sobel;
  VarBool * _v_detected_edges;

  const CameraParameters& camera_parameters;
  const RoboCupField& real_field;
  const RoboCupCalibrationHalfField& calib_field;

  LUT3D * _threshold_lut;
  greyImage* edge_image;
  greyImage* temp_grey_image;
  
  void drawFieldLine(double xStart, double yStart, double xEnd, double yEnd, int steps,
                     VisualizationFrame * vis_frame, 
                     unsigned char r=255, unsigned char g=100, unsigned char b=100);
  
public:
    PluginVisualize(FrameBuffer * _buffer, const CameraParameters& camera_params, const RoboCupField& real_field, const RoboCupCalibrationHalfField& calib_field);

    ~PluginVisualize();

   void setThresholdingLUT(LUT3D * threshold_lut);
   virtual ProcessResult process(FrameData * data, RenderOptions * options);
   virtual VarList * getSettings();
   virtual string getName();
};

#endif

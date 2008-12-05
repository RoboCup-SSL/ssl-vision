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
  
  LUT3D * _threshold_lut;
public:
    PluginVisualize(FrameBuffer * _buffer);

    ~PluginVisualize();

   void setThresholdingLUT(LUT3D * threshold_lut);
   virtual ProcessResult process(FrameData * data, RenderOptions * options);
   virtual VarList * getSettings();
   virtual string getName();
};

#endif

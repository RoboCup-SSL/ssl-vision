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
  \file    plugin_visualize.cpp
  \brief   C++ Implementation: plugin_visualize
  \author  Stefan Zickler, 2008
*/
//========================================================================
#include "plugin_visualize.h"



PluginVisualize::PluginVisualize(FrameBuffer * _buffer)
 : VisionPlugin(_buffer)
{
  _settings=new VarList("Visualization");
  _settings->addChild(_v_enabled=new VarBool("enable", true));
  _settings->addChild(_v_image=new VarBool("image", true));
  _settings->addChild(_v_greyscale=new VarBool("greyscale", true));
  _settings->addChild(_v_thresholded=new VarBool("thresholded", true));
  _threshold_lut=0;
}


PluginVisualize::~PluginVisualize()
{
}

VarList * PluginVisualize::getSettings() {
  return _settings;
}

string PluginVisualize::getName() {
  return "Visualization";
}

ProcessResult PluginVisualize::process(FrameData * data, RenderOptions * options)
{
  (void)options;
  if (data==0) return ProcessingFailed;

  VisualizationFrame * vis_frame;
  if ((vis_frame=(VisualizationFrame *)data->map.get("vis_frame")) == 0) {
    vis_frame=(VisualizationFrame *)data->map.insert("vis_frame",new VisualizationFrame());
  }

  if (_v_enabled->getBool()==true) {
    //check video data...
    if (data->video.getWidth() == 0 || data->video.getHeight()==0) {
      //there is no valid video data
      //mark visualization data as invalid
      vis_frame->valid=false;
      return ProcessingOk;
    } else {
      //allocate visualization frame accordingly:
      vis_frame->data.allocate(data->video.getWidth(), data->video.getHeight());
    }

    if (_v_image->getBool()==true) {
      //if converting entire image then blanking is not needed
      ColorFormat source_format=data->video.getColorFormat();
      if (source_format==COLOR_RGB8) {
        //plain copy of data
        memcpy(vis_frame->data.getData(),data->video.getData(),data->video.getNumBytes());
      } else if (source_format==COLOR_YUV422_UYVY) {
        Conversions::uyvy2rgb(data->video.getData(),(unsigned char*)(vis_frame->data.getData()),data->video.getWidth(),data->video.getHeight());
      } else {
        //blank it:
        vis_frame->data.fillBlack();
        fprintf(stderr,"Unable to visualize color format: %s\n",Colors::colorFormatToString(source_format).c_str());
        fprintf(stderr,"Currently supported are rgb8 and yuv422 (UYVY).\n");
        fprintf(stderr,"(Feel free to add more conversions to plugin_visualize.cpp).\n");
      }
      if (_v_greyscale->getBool()==true) {
        unsigned int n = vis_frame->data.getNumPixels();
        rgb * vis_ptr = vis_frame->data.getPixelData();
        rgb color;
        for (unsigned int i=0;i<n;i++) {
          color=vis_ptr[i];
          color.r=color.g=color.b=((color.r+color.g+color.b)/3);
          vis_ptr[i]=color;
        }
      }
    } else {
      vis_frame->data.fillBlack();
    }

    if (_v_thresholded->getBool()==true) {
      if (_threshold_lut!=0) {
        Image<raw8> * img_thresholded=(Image<raw8> *)(data->map.get("cmv_threshold"));
        if (img_thresholded!=0) {
          int n = vis_frame->data.getNumPixels();
          if (img_thresholded->getNumPixels()==n) {
            rgb * vis_ptr = vis_frame->data.getPixelData();
            raw8 * seg_ptr = img_thresholded->getPixelData();
            for (int i=0;i<n;i++) {
              if (seg_ptr[i].getIntensity() !=0) {
                vis_ptr[i]=_threshold_lut->getChannel(seg_ptr[i].getIntensity()).draw_color;
              }
            }
          }
        }
      }
    }

    
    //_threshold_lut->getChannel(0).draw_color
    vis_frame->valid=true;
    //otherwise...blank it out.
    
    //transfer image...optionally applying filtering effects
    
    
    //check for availability of thresholdation results...
    
    
    //check for availability of geometric calibration data...
    
    
    
  } else {
    vis_frame->valid=false;
  }
  return ProcessingOk;
}

void PluginVisualize::setThresholdingLUT(LUT3D * threshold_lut) {
  _threshold_lut=threshold_lut;
}

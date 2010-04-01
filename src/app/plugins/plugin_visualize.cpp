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
#include <sobel.h>


PluginVisualize::PluginVisualize(FrameBuffer * _buffer, const CameraParameters& camera_params, const RoboCupField& real_field, const RoboCupCalibrationHalfField& calib_field)
 : VisionPlugin(_buffer), camera_parameters(camera_params), real_field(real_field), calib_field(calib_field)
{
  _settings=new VarList("Visualization");
  _settings->addChild(_v_enabled=new VarBool("enable", true));
  _settings->addChild(_v_image=new VarBool("image", true));
  _settings->addChild(_v_greyscale=new VarBool("greyscale", false));
  _settings->addChild(_v_thresholded=new VarBool("thresholded", false));
  _settings->addChild(_v_blobs=new VarBool("blobs", false));
  _settings->addChild(_v_camera_calibration=new VarBool("camera calibration", false));
  _settings->addChild(_v_calibration_result=new VarBool("calibration result", false));
  _settings->addChild(_v_detected_edges=new VarBool("detected edges", false));
  _settings->addChild(_v_complete_sobel=new VarBool("complete edge detection", false));
  _v_complete_sobel->setBool(false);
  _threshold_lut=0;
  edge_image = 0;
  temp_grey_image = 0;
}


PluginVisualize::~PluginVisualize()
{
  if(edge_image)
    delete edge_image;
  if(temp_grey_image)
    delete temp_grey_image;
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

    //draw blob finding results:
    if (_v_blobs->getBool()==true) {
      CMVision::ColorRegionList * colorlist;
      colorlist=(CMVision::ColorRegionList *)data->map.get("cmv_colorlist");
      if (colorlist!=0) {
        CMVision::RegionLinkedList * regionlist;
        regionlist = colorlist->getColorRegionArrayPointer();
        for (int i=0;i<colorlist->getNumColorRegions();i++) {
          rgb blob_draw_color;
          if (_threshold_lut!=0) {
            blob_draw_color= _threshold_lut->getChannel(i).draw_color;
          } else {
            blob_draw_color.set(255,255,255);
          }
          CMVision::Region * blob=regionlist[i].getInitialElement();
          while (blob != 0) {
            vis_frame->data.drawLine(blob->x1,blob->y1,blob->x2,blob->y1,blob_draw_color);
            vis_frame->data.drawLine(blob->x1,blob->y1,blob->x1,blob->y2,blob_draw_color);
            vis_frame->data.drawLine(blob->x1,blob->y2,blob->x2,blob->y2,blob_draw_color);
            vis_frame->data.drawLine(blob->x2,blob->y1,blob->x2,blob->y2,blob_draw_color);
            blob=blob->next;
          }
        }
      }
    }
    //transfer image...optionally applying filtering effects
    
    // Camera calibration 
    if (_v_camera_calibration->getBool()==true) 
    {
      // Principal point
      rgb ppoint_draw_color;
      ppoint_draw_color.set(255,0,0);
      int x = camera_parameters.principal_point_x->getDouble();
      int y = camera_parameters.principal_point_y->getDouble();
      vis_frame->data.drawFatLine(x-15,y-15,x+15,y+15,ppoint_draw_color);
      vis_frame->data.drawFatLine(x+15,y-15,x-15,y+15,ppoint_draw_color);
      // Calibration points
      rgb cpoint_draw_color;
      cpoint_draw_color.set(0,255,255);
      int bx = camera_parameters.additional_calibration_information->left_corner_image_x->getDouble();
      int by = camera_parameters.additional_calibration_information->left_corner_image_y->getDouble();
      vis_frame->data.drawFatBox(bx-5,by-5,11,11,cpoint_draw_color);
      vis_frame->data.drawString(bx-40,by-25,"Left", cpoint_draw_color);
      vis_frame->data.drawString(bx-40,by-15,"Corner", cpoint_draw_color);
      bx = camera_parameters.additional_calibration_information->right_corner_image_x->getDouble();
      by = camera_parameters.additional_calibration_information->right_corner_image_y->getDouble();
      vis_frame->data.drawFatBox(bx-5,by-5,11,11,cpoint_draw_color);
      vis_frame->data.drawString(bx+5,by-25,"Right", cpoint_draw_color);
      vis_frame->data.drawString(bx+5,by-15,"Corner", cpoint_draw_color);
      bx = camera_parameters.additional_calibration_information->left_centerline_image_x->getDouble();
      by = camera_parameters.additional_calibration_information->left_centerline_image_y->getDouble();
      vis_frame->data.drawFatBox(bx-5,by-5,11,11,cpoint_draw_color);
      vis_frame->data.drawString(bx-40,by+15,"Left", cpoint_draw_color);
      vis_frame->data.drawString(bx-40,by+25,"Center", cpoint_draw_color);
      bx = camera_parameters.additional_calibration_information->right_centerline_image_x->getDouble();
      by = camera_parameters.additional_calibration_information->right_centerline_image_y->getDouble();
      vis_frame->data.drawFatBox(bx-5,by-5,11,11,cpoint_draw_color);
      vis_frame->data.drawString(bx+5,by+15,"Right", cpoint_draw_color);
      vis_frame->data.drawString(bx+5,by+25,"Center", cpoint_draw_color);
    }
    
    // Result of camera calibration, draws field to image
    if (_v_calibration_result->getBool()==true) 
    {
      int stepsPerLine(20);
      // Left side line:
      drawFieldLine(camera_parameters.field.left_corner_x->getInt(),
                    camera_parameters.field.left_corner_y->getInt(),
                    camera_parameters.field.left_centerline_x->getInt(),
                    camera_parameters.field.left_centerline_y->getInt(), 
                    stepsPerLine, vis_frame);
      // Right side line:
      drawFieldLine(camera_parameters.field.right_corner_x->getInt(),
                    camera_parameters.field.right_corner_y->getInt(),
                    camera_parameters.field.right_centerline_x->getInt(),
                    camera_parameters.field.right_centerline_y->getInt(), 
                    stepsPerLine, vis_frame);
      // Goal line:
      drawFieldLine(camera_parameters.field.right_corner_x->getInt(),
                    camera_parameters.field.right_corner_y->getInt(),
                    camera_parameters.field.left_corner_x->getInt(),
                    camera_parameters.field.left_corner_y->getInt(),
                    stepsPerLine, vis_frame);
      // Center line:
      drawFieldLine(camera_parameters.field.left_centerline_x->getInt(),
                    camera_parameters.field.left_centerline_y->getInt(),
                    camera_parameters.field.right_centerline_x->getInt(),
                    camera_parameters.field.right_centerline_y->getInt(), 
                    stepsPerLine, vis_frame);

      //draw boundary lines:
      // Left boundary line:
      double boundary_width=real_field.boundary_width->getInt();
      drawFieldLine(camera_parameters.field.left_corner_x->getInt() + (sign(camera_parameters.field.left_corner_x->getInt()) * boundary_width),
                    camera_parameters.field.left_corner_y->getInt() + (sign(camera_parameters.field.left_corner_y->getInt()) * boundary_width),
                    camera_parameters.field.left_centerline_x->getInt(),
                    camera_parameters.field.left_centerline_y->getInt() + (sign(camera_parameters.field.left_centerline_y->getInt()) * boundary_width), 
                    stepsPerLine, vis_frame);

      // Right boundary line:
      drawFieldLine(camera_parameters.field.right_corner_x->getInt() + (sign(camera_parameters.field.right_corner_x->getInt()) * boundary_width),
                    camera_parameters.field.right_corner_y->getInt() + (sign(camera_parameters.field.right_corner_y->getInt()) * boundary_width),
                    camera_parameters.field.right_centerline_x->getInt(),
                    camera_parameters.field.right_centerline_y->getInt() + (sign(camera_parameters.field.right_centerline_y->getInt()) * boundary_width),
                    stepsPerLine, vis_frame);

      // Goal boundary line:
      drawFieldLine(camera_parameters.field.right_corner_x->getInt() + (sign(camera_parameters.field.right_corner_x->getInt()) * boundary_width),
                    camera_parameters.field.right_corner_y->getInt() + (sign(camera_parameters.field.right_corner_y->getInt()) * boundary_width),
                    camera_parameters.field.left_corner_x->getInt() + (sign(camera_parameters.field.left_corner_x->getInt()) * boundary_width),
                    camera_parameters.field.left_corner_y->getInt() + (sign(camera_parameters.field.left_corner_y->getInt()) * boundary_width),
                    stepsPerLine, vis_frame);
      

      const int mult = calib_field.isCamPosHalfNegX() ? -1 : 1;

      // Center circle:
      double prev_x = 0;
      double prev_y = real_field.center_circle_radius->getInt();
      
      for (int i=1; i <= 10; i ++)
      {
        double y = cos(i * 0.314) * real_field.center_circle_radius->getInt();
        double x = sin(mult * i * 0.314) * real_field.center_circle_radius->getInt();

        drawFieldLine((int) prev_x,
                      (int) prev_y,
                      (int) x,
                      (int) y, 
                      stepsPerLine, vis_frame);
        
        prev_x = x;
        prev_y = y;
      }
      
      // Goal area:
      int defense_radius=real_field.defense_radius->getInt();
      int defense_x=mult * real_field.half_field_length->getInt();
      int defense_stretch_h=real_field.half_defense_stretch->getInt();
      prev_x = 0;
      prev_y = -defense_radius;
      
      for (double i=3.14; i<=3.14+3.14/2; i+= 0.314)
      {
        double y = cos(i) * defense_radius;
        double x = sin(mult * i) * defense_radius;
        
        drawFieldLine((int) prev_x +  defense_x,
                      (int) prev_y - defense_stretch_h,
                      (int) x + defense_x,
                      (int) y - defense_stretch_h, 
                      stepsPerLine, vis_frame);
        
        prev_x = x;
        prev_y = y;
      }
      
      drawFieldLine(defense_x - mult * defense_radius, -defense_stretch_h, defense_x - mult * defense_radius, defense_stretch_h, stepsPerLine, vis_frame);
      
      prev_x = mult * -defense_radius;
      prev_y = 0;
      
      for (double i=3.14+3.14/2; i<=3.14+3.14; i+= 0.314)
      {
        double y = cos(i) * defense_radius;
        double x = sin(mult * i) * defense_radius;
        
        drawFieldLine((int) prev_x + defense_x,
                      (int) prev_y + defense_stretch_h,
                      (int) x + defense_x,
                      (int) y + defense_stretch_h, 
                      stepsPerLine, vis_frame);

        prev_x = x;
        prev_y = y;
      }  

      
       int field_length = real_field.half_field_length->getInt();
       int field_width = real_field.half_field_width->getInt();

       for (int grid_y=0; grid_y < field_width; grid_y += 500)
       {
         drawFieldLine((int) 0,
                       (int) -grid_y,
                       (int) mult * field_length,
                       (int) -grid_y, 
                       stepsPerLine, vis_frame);
        
         drawFieldLine((int) 0,
                       (int) grid_y,
                       (int) mult * field_length,
                       (int) grid_y, 
                       stepsPerLine, vis_frame);
        
       }
       for (int grid_x=0; grid_x < field_length; grid_x += 500)
       {
         drawFieldLine((int) mult * grid_x,
                       (int) -field_width,
                       (int) mult * grid_x,
                       (int) field_width, 
                       stepsPerLine, vis_frame);
       } 
    }
    
    // Test edge detection for calibration
    if (_v_complete_sobel->getBool()==true) 
    {
      if(edge_image == 0)
      { 
        edge_image = new greyImage(data->video.getWidth(),data->video.getHeight());
        temp_grey_image = new greyImage(data->video.getWidth(),data->video.getHeight());
      }
      Images::convert(vis_frame->data, *temp_grey_image);
      // Draw sobel image: Contrast towards more brightness is painted white,
      //                   Contrast towards more darkness is painted green
      for(int y=1; y<temp_grey_image->getHeight()-1; ++y)
      {
        for(int x=1; x<temp_grey_image->getWidth()-1; ++x)
        {
          int colVB,colVD,colHB,colHD;
          colVB = Sobel::verticalBrighter(*temp_grey_image,x,y,30);
          colVD = Sobel::verticalDarker(*temp_grey_image,x,y,30);
          colHB = Sobel::horizontalBrighter(*temp_grey_image,x,y,30);
          colHD = Sobel::horizontalDarker(*temp_grey_image,x,y,30);
          int dMax = colVD > colHD ? colVD : colHD;
          int bMax = colVB > colHB ? colVB : colHB;
          grey col;
          if(dMax > bMax)
            col.v = 1;
          else if (bMax > dMax)
            col.v = 2;
          else
            col.v = 0;
          edge_image->setPixel(x,y,col);
        }
      }
      unsigned int n = edge_image->getNumPixels();
      rgb * vis_ptr = vis_frame->data.getPixelData();
      grey * edge_ptr = edge_image->getPixelData();
      rgb color;
      for (unsigned int i=0;i<n;i++) {
        unsigned char col = edge_ptr[i].v;
        if(col == 0)
        {
          color.r=color.g=color.b=col;
        }
        else if(col == 1)
        {
          color.r=0; color.g=255; color.b=0;
        }
        else if(col == 2)
        {
          color.r=255; color.g=255; color.b=255;
        }
        vis_ptr[i]=color;
      }
    }
    
    // Result of edge detection for second calibration step
    if (_v_detected_edges->getBool()==true) 
    {
      // The edges:
      rgb edge_draw_color;
      for(unsigned int ls=0; ls<camera_parameters.calibrationSegments.size(); ++ls)
      {
        const CameraParameters::CalibrationData& segment =
            camera_parameters.calibrationSegments[ls];
        for(unsigned int edge=0; edge<segment.imgPts.size(); ++edge)
        {
          const GVector::vector2d<double>& pt = segment.imgPts[edge].first;
          if(segment.imgPts[edge].second)
            edge_draw_color.set(255,0,0);
          else
            edge_draw_color.set(255,0,255);
          vis_frame->data.drawBox(pt.x-5,pt.y-5,11,11,edge_draw_color);
          if(segment.straightLine){
            if(segment.horizontal)
              vis_frame->data.drawLine(pt.x,pt.y-2,pt.x,pt.y+2,edge_draw_color);
            else
              vis_frame->data.drawLine(pt.x-2,pt.y,pt.x+2,pt.y,edge_draw_color);
          }else{
            vis_frame->data.drawLine(pt.x,pt.y-2,pt.x,pt.y+2,edge_draw_color);
            vis_frame->data.drawLine(pt.x-2,pt.y,pt.x+2,pt.y,edge_draw_color);
          }
        }
      }
      // The search corridor:
      double corridorWidth = camera_parameters.additional_calibration_information->line_search_corridor_width->getDouble();
      double offset = corridorWidth/2;
      int stepsPerLine(20);
      
      double xLeftCorner(camera_parameters.field.left_corner_x->getInt());
      double yLeftCorner(camera_parameters.field.left_corner_y->getInt());
      double xRightCorner(camera_parameters.field.right_corner_x->getInt());
      double yRightCorner(camera_parameters.field.right_corner_y->getInt());
      double xLeftCenter(camera_parameters.field.left_centerline_x->getInt());
      double yLeftCenter(camera_parameters.field.left_centerline_y->getInt());
      double xRightCenter(camera_parameters.field.right_centerline_x->getInt());
      double yRightCenter(camera_parameters.field.right_centerline_y->getInt());
      
      double xLeftCornerOuter = xLeftCorner > 0 ? xLeftCorner + offset : xLeftCorner - offset;
      double yLeftCornerOuter = yLeftCorner > 0 ? yLeftCorner + offset : yLeftCorner - offset;
      double xRightCornerOuter = xRightCorner > 0 ? xRightCorner + offset : xRightCorner - offset;
      double yRightCornerOuter = yRightCorner > 0 ? yRightCorner + offset : yRightCorner - offset;
      double xLeftCenterOuter = xLeftCorner > 0 ? xLeftCenter - offset : xLeftCenter + offset;
      double yLeftCenterOuter = yLeftCenter > 0 ? yLeftCenter + offset : yLeftCenter - offset;
      double xRightCenterOuter = xRightCorner > 0 ? xRightCenter - offset : xRightCenter + offset;
      double yRightCenterOuter = yRightCenter > 0 ? yRightCenter + offset : yRightCenter - offset;
      double xLeftCornerInner = xLeftCorner > 0 ? xLeftCorner - offset : xLeftCorner + offset;
      double yLeftCornerInner = yLeftCorner > 0 ? yLeftCorner - offset : yLeftCorner + offset;
      double xRightCornerInner = xRightCorner > 0 ? xRightCorner - offset : xRightCorner + offset;
      double yRightCornerInner = yRightCorner > 0 ? yRightCorner - offset : yRightCorner + offset;
      double xLeftCenterInner = xLeftCorner > 0 ? xLeftCenter + offset : xLeftCenter - offset;
      double yLeftCenterInner = yLeftCenter > 0 ? yLeftCenter - offset : yLeftCenter + offset;
      double xRightCenterInner = xRightCorner > 0 ? xRightCenter + offset : xRightCenter - offset;
      double yRightCenterInner = yRightCenter > 0 ? yRightCenter - offset : yRightCenter + offset;
      
      // Outer left side line:
      drawFieldLine(xLeftCornerOuter, yLeftCornerOuter, xLeftCenterOuter, yLeftCenterOuter,
                   stepsPerLine, vis_frame, 180, 180, 255);
      // Outer right side line:
      drawFieldLine(xRightCornerOuter, yRightCornerOuter, xRightCenterOuter, yRightCenterOuter,
                    stepsPerLine, vis_frame, 180, 180, 255);
      // Outer goal line:
      drawFieldLine(xRightCornerOuter, yRightCornerOuter, xLeftCornerOuter, yLeftCornerOuter,
                    stepsPerLine, vis_frame, 180, 180, 255);
      // Outer center line:
      drawFieldLine(xRightCenterOuter, yRightCenterOuter, xLeftCenterOuter, yLeftCenterOuter,
                    stepsPerLine, vis_frame, 180, 180, 255);
      // Inner left side line:
      drawFieldLine(xLeftCornerInner, yLeftCornerInner, xLeftCenterInner, yLeftCenterInner,
                    stepsPerLine, vis_frame, 180, 180, 255);
      // Inner right side line:
      drawFieldLine(xRightCornerInner, yRightCornerInner, xRightCenterInner, yRightCenterInner,
                    stepsPerLine, vis_frame, 180, 180, 255);
      // Inner goal line:
      drawFieldLine(xRightCornerInner, yRightCornerInner, xLeftCornerInner, yLeftCornerInner,
                    stepsPerLine, vis_frame, 180, 180, 255);
      // Inner center line:
      drawFieldLine(xRightCenterInner, yRightCenterInner, xLeftCenterInner, yLeftCenterInner,
                    stepsPerLine, vis_frame, 180, 180, 255);
    }  
    vis_frame->valid=true;
  } 
  else 
  {
    vis_frame->valid=false;
  }
  return ProcessingOk;
}

void PluginVisualize::setThresholdingLUT(LUT3D * threshold_lut) {
  _threshold_lut=threshold_lut;
}

void PluginVisualize::drawFieldLine(double xStart, double yStart, 
                                    double xEnd, double yEnd, 
                                    int steps, VisualizationFrame * vis_frame,
                                    unsigned char r, unsigned char g, unsigned char b)
{
  GVector::vector3d<double> start(xStart, yStart,0);
  GVector::vector3d<double> end(xEnd, yEnd,0);
  GVector::vector3d<double> offset;
  offset = (end - start);
  offset *= 1.0/steps;
  GVector::vector2d<double> lastInImage;
  GVector::vector3d<double> lastInWorld(start);
  camera_parameters.field2image(lastInWorld, lastInImage);
  for(int i=0; i<steps; ++i)
  {
    GVector::vector3d<double> nextInWorld = lastInWorld + offset;
    GVector::vector2d<double> nextInImage;
    camera_parameters.field2image(nextInWorld, nextInImage);
    //    std::cout<<"Point in image: "<<posInImage.x<<","<<posInImage.y<<std::endl;
    rgb draw_color;
    draw_color.set(r,g,b);
    vis_frame->data.drawFatLine(lastInImage.x,lastInImage.y,nextInImage.x,nextInImage.y,draw_color);
    
    lastInWorld = nextInWorld;
    lastInImage = nextInImage;
  }
}

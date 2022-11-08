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
#include <opencv2/opencv.hpp>
#include "convex_hull.h"
#include <mutex>

namespace {
typedef CameraParameters::AdditionalCalibrationInformation AddnlCalibInfo;
}  // namespace

PluginVisualize::PluginVisualize(
    FrameBuffer* _buffer, const CameraParameters& camera_params,
    const RoboCupField& real_field, const ConvexHullImageMask& mask) :
    VisionPlugin(_buffer), camera_parameters(camera_params),
    real_field(real_field),
    _image_mask(mask){
  _v_enabled = new VarBool("enable", true);
  _v_image = new VarBool("image", true);
  _v_greyscale = new VarBool("greyscale", false);
  _v_thresholded = new VarBool("thresholded", true);
  _v_blobs = new VarBool("blobs", true);
  _v_camera_calibration = new VarBool("camera calibration", true);
  _v_calibration_result = new VarBool("calibration result", true);
  _v_detected_edges = new VarBool("detected edges", false);
  _v_complete_sobel = new VarBool("complete edge detection", false);
  _v_complete_sobel->setBool(false);

  _v_mask_hull = new VarBool("image mask hull", true);

  _settings = new VarList("Visualization");
  _settings->addChild(_v_enabled);
  _settings->addChild(_v_image);
  _settings->addChild(_v_greyscale);
  _settings->addChild(_v_thresholded);
  _settings->addChild(_v_blobs);
  _settings->addChild(_v_camera_calibration);
  _settings->addChild(_v_calibration_result);
  _settings->addChild(_v_detected_edges);
  _settings->addChild(_v_complete_sobel);
  _settings->addChild(_v_mask_hull);
  _threshold_lut=0;
  edge_image = 0;
  temp_grey_image = 0;
}


PluginVisualize::~PluginVisualize() {
  if (edge_image) delete edge_image;
  if (temp_grey_image) delete temp_grey_image;
}

VarList * PluginVisualize::getSettings() {
  return _settings;
}

string PluginVisualize::getName() {
  return "Visualization";
}

void PluginVisualize::DrawCameraImage(
    FrameData* data, VisualizationFrame* vis_frame) {
  //if converting entire image then blanking is not needed
  const ColorFormat source_format = data->video.getColorFormat();
  if (source_format == COLOR_RGB8) {
    //plain copy of data
    memcpy(vis_frame->data.getData(), data->video.getData(),
            data->video.getNumBytes());
  } else if (source_format==COLOR_YUV422_UYVY) {
    Conversions::uyvy2rgb(
        data->video.getData(),
        reinterpret_cast<unsigned char*>(vis_frame->data.getData()),
        data->video.getWidth(), data->video.getHeight());
  } else if (source_format==COLOR_RAW8) {
    cv::Mat src(data->video.getWidth(), data->video.getHeight(), CV_8UC1, data->video.getData());
    cv::Mat dst(data->video.getWidth(), data->video.getHeight(), CV_8UC3, vis_frame->data.getData());
    cvtColor(src, dst, cv::COLOR_BayerBG2BGR);
  } else {
    //blank it:
    vis_frame->data.fillBlack();
    fprintf(stderr, "Unable to visualize color format: %s\n",
            Colors::colorFormatToString(source_format).c_str());
    fprintf(stderr, "Currently supported are rgb8 and yuv422 (UYVY).\n");
    fprintf(stderr, "(Feel free to add more conversions to %s in %s).\n",
            __FUNCTION__, __FILE__);
  }
  if (_v_greyscale->getBool() == true) {
    unsigned int n = vis_frame->data.getNumPixels();
    rgb * vis_ptr = vis_frame->data.getPixelData();
    rgb color;

    for (unsigned int i = 0; i < n; i++) {
      color = vis_ptr[i];
      color.r = color.g = color.b = ((color.r + color.g + color.b)/3);
      vis_ptr[i] = color;
    }
  }
}

void PluginVisualize::DrawThresholdedImage(
    FrameData* data, VisualizationFrame* vis_frame) {
  if (_threshold_lut != 0) {
    Image<raw8>* img_thresholded =
        reinterpret_cast<Image<raw8>*>(data->map.get("cmv_threshold"));
    if (img_thresholded != 0) {
      int n = vis_frame->data.getNumPixels();
      if (img_thresholded->getNumPixels() == n) {
        rgb * vis_ptr = vis_frame->data.getPixelData();
        raw8 * seg_ptr = img_thresholded->getPixelData();
        for (int i = 0; i < n; i++) {
          if (seg_ptr[i].getIntensity() != 0) {
            vis_ptr[i] = _threshold_lut->getChannel(
                seg_ptr[i].getIntensity()).draw_color;
          }
        }
      }
    }
  }
}

void PluginVisualize::DrawBlobs(
    FrameData* data, VisualizationFrame* vis_frame) {
  CMVision::ColorRegionList* colorlist =
      reinterpret_cast<CMVision::ColorRegionList*>(
          data->map.get("cmv_colorlist"));
  if (colorlist != 0) {
    CMVision::RegionLinkedList * regionlist;
    regionlist = colorlist->getColorRegionArrayPointer();
    for (int i = 0; i < colorlist->getNumColorRegions(); i++) {
      rgb blob_draw_color;
      if (_threshold_lut != 0) {
        blob_draw_color = _threshold_lut->getChannel(i).draw_color;
      } else {
        blob_draw_color.set(255, 255, 255);
      }
      CMVision::Region * blob=regionlist[i].getInitialElement();
      while (blob != 0) {
        vis_frame->data.drawLine(
            blob->x1,blob->y1,blob->x2,blob->y1,blob_draw_color);
        vis_frame->data.drawLine(
            blob->x1,blob->y1,blob->x1,blob->y2,blob_draw_color);
        vis_frame->data.drawLine(
            blob->x1,blob->y2,blob->x2,blob->y2,blob_draw_color);
        vis_frame->data.drawLine(
            blob->x2,blob->y1,blob->x2,blob->y2,blob_draw_color);
        blob = blob->next;
      }
    }
  }
}

void PluginVisualize::DrawCameraCalibration(
    FrameData* data, VisualizationFrame* vis_frame) {
  // Principal point
  rgb ppoint_draw_color;
  ppoint_draw_color.set(255, 0, 0);
  int x = camera_parameters.principal_point_x->getDouble();
  int y = camera_parameters.principal_point_y->getDouble();
  vis_frame->data.drawFatLine(x-15, y-15, x+15, y+15, ppoint_draw_color);
  vis_frame->data.drawFatLine(x+15, y-15, x-15, y+15, ppoint_draw_color);
  // Calibration points
  rgb cpoint_draw_color;
  cpoint_draw_color.set(0,255,255);
  for (int i = 0; i < AddnlCalibInfo::kNumControlPoints; ++i) {
    const int bx = camera_parameters.additional_calibration_information->
        control_point_image_xs[i]->getDouble();
    const int by = camera_parameters.additional_calibration_information->
        control_point_image_ys[i]->getDouble();
    vis_frame->data.drawFatBox(bx - 5, by - 5, 11, 11, cpoint_draw_color);
    const string label =
        camera_parameters.additional_calibration_information->
        control_point_names[i]->getString();
    vis_frame->data.drawString(bx - 5, by + 15, label, cpoint_draw_color);

    char buff[20];
    snprintf(buff, sizeof(buff), "(%.0f,%.0f)",
             camera_parameters.additional_calibration_information->control_point_field_xs[i]->getDouble(),
             camera_parameters.additional_calibration_information->control_point_field_ys[i]->getDouble());
    std::string description = buff;
    vis_frame->data.drawString(bx + 10, by - 2, description, cpoint_draw_color);
  }
}

void PluginVisualize::DrawCalibrationResult(
    FrameData* data, VisualizationFrame* vis_frame) {
  int steps_per_line(20);
  real_field.field_markings_mutex.lockForRead();
  for (size_t i = 0; i < real_field.field_lines.size(); ++i) {
    const FieldLine& line_segment =
        *(real_field.field_lines[i]);
    const GVector::vector3d<double> p1(
        line_segment.p1_x->getDouble(), line_segment.p1_y->getDouble(), 0.0);
    const GVector::vector3d<double> p2(
        line_segment.p2_x->getDouble(), line_segment.p2_y->getDouble(), 0.0);
    drawFieldLine(p1, p2, steps_per_line, vis_frame);
  }
  for (size_t i = 0; i < real_field.field_arcs.size(); ++i) {
    const FieldCircularArc& arc = *(real_field.field_arcs[i]);
    const GVector::vector3d<double> center(
        arc.center_x->getDouble(), arc.center_y->getDouble(), 0.0);
    drawFieldArc(center, arc.radius->getDouble(), arc.a1->getDouble(),
                 arc.a2->getDouble(), steps_per_line, vis_frame);
  }
  real_field.field_markings_mutex.unlock();
}

void PluginVisualize::DrawSobelImage(
    FrameData* data, VisualizationFrame* vis_frame) {
  if (edge_image == 0) {
    edge_image =
        new greyImage(data->video.getWidth(),data->video.getHeight());
    temp_grey_image =
        new greyImage(data->video.getWidth(),data->video.getHeight());
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
      if (dMax > bMax)
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
    if (col == 0)
    {
      color.r=color.g=color.b=col;
    }
    else if (col == 1)
    {
      color.r=0; color.g=255; color.b=0;
    }
    else if (col == 2)
    {
      color.r=255; color.g=255; color.b=255;
    }
    vis_ptr[i]=color;
  }
}

void PluginVisualize::DrawEdgeTangent(
    const GVector::vector2d<double>& image_point,
    const GVector::vector3d<double>& field_point,
    const GVector::vector3d<double>& field_tangent,
    VisualizationFrame* vis_frame, rgb edge_draw_color) {
  const GVector::vector3d<double> field_point_plus_tangent =
      field_point + 1000.0 * field_tangent;
  GVector::vector2d<double> image_point_plus_tangent(0.0, 0.0);
  camera_parameters.field2image(
      field_point_plus_tangent, image_point_plus_tangent);
  const GVector::vector2d<double> edge_p1 =
      image_point + (image_point_plus_tangent - image_point).norm(6.0);
  const GVector::vector2d<double> edge_p2 =
      image_point - (image_point_plus_tangent - image_point).norm(6.0);
  vis_frame->data.drawLine(edge_p1.x, edge_p1.y,
                            edge_p2.x, edge_p2.y, edge_draw_color);
}

void PluginVisualize::DrawDetectedEdges(
    FrameData* data, VisualizationFrame* vis_frame) {
  // The edges:
  const rgb edge_draw_color = RGB::Red;
  for (size_t ls = 0; ls < camera_parameters.calibrationSegments.size(); ++ls) {
    const CameraParameters::CalibrationData& segment =
        camera_parameters.calibrationSegments[ls];
    for(unsigned int edge=0; edge<segment.imgPts.size(); ++edge) {
      if (!(segment.imgPts[edge].second)) continue;
      const GVector::vector2d<double>& image_point = segment.imgPts[edge].first;
      vis_frame->data.drawBox(
          image_point.x - 5, image_point.y - 5, 11, 11, edge_draw_color);
      const double alpha = segment.alphas[edge];
      if (segment.straightLine) {
        const GVector::vector3d<double> field_point =
            alpha * segment.p1 + (1.0 - alpha) * segment.p2;
        const GVector::vector3d<double> field_tangent =
            (segment.p2 - segment.p1).norm();
        DrawEdgeTangent(image_point, field_point, field_tangent, vis_frame,
                        edge_draw_color);
      } else {
        const double angle =
            alpha * segment.theta1 + (1.0 - alpha) * segment.theta2;
        const GVector::vector3d<double> field_radius_vector(
            cos(angle), sin(angle), 0.0);
        const GVector::vector3d<double> field_point =
            segment.center + segment.radius * field_radius_vector;
        const GVector::vector3d<double> field_tangent(
            -sin(angle), cos(angle), 0.0);
        DrawEdgeTangent(image_point, field_point, field_tangent, vis_frame,
                        edge_draw_color);
      }
    }
  }
  DrawSearchCorridors(data, vis_frame);
}

void PluginVisualize::DrawSearchCorridors(
    FrameData* data, VisualizationFrame* vis_frame) {
  static const int steps_per_line = 20;
  const double half_corridor_width = camera_parameters.
      additional_calibration_information->line_search_corridor_width->
      getDouble() * 0.5;
  real_field.field_markings_mutex.lockForRead();
  for (size_t i = 0; i < real_field.field_lines.size(); ++i) {
    const FieldLine& line_segment =
        *(real_field.field_lines[i]);
    const GVector::vector3d<double> p1(
        line_segment.p1_x->getDouble(), line_segment.p1_y->getDouble(), 0.0);
    const GVector::vector3d<double> p2(
        line_segment.p2_x->getDouble(), line_segment.p2_y->getDouble(), 0.0);
    const GVector::vector3d<double> line_dir = (p2 - p1).norm();
    const GVector::vector3d<double> line_perp(-line_dir.y, line_dir.x, 0.0);
    drawFieldLine(p1 + half_corridor_width * line_perp,
                  p2 + half_corridor_width * line_perp,
                  steps_per_line, vis_frame, 180, 180, 255);
    drawFieldLine(p1 - half_corridor_width * line_perp,
                  p2 - half_corridor_width * line_perp,
                  steps_per_line, vis_frame, 180, 180, 255);
  }
  for (size_t i = 0; i < real_field.field_arcs.size(); ++i) {
    const FieldCircularArc& arc = *(real_field.field_arcs[i]);
    const GVector::vector3d<double> center(
        arc.center_x->getDouble(), arc.center_y->getDouble(), 0.0);
    drawFieldArc(center, arc.radius->getDouble() + half_corridor_width,
                 arc.a1->getDouble(), arc.a2->getDouble(), steps_per_line,
                 vis_frame, 180, 180, 255);
    drawFieldArc(center,
                 max(0.0, arc.radius->getDouble() - half_corridor_width),
                 arc.a1->getDouble(), arc.a2->getDouble(), steps_per_line,
                 vis_frame, 180, 180, 255);
  }
  real_field.field_markings_mutex.unlock();
}

void PluginVisualize::DrawMaskHull(
    FrameData* data, VisualizationFrame* vis_frame) {
  _image_mask.lock();
  auto mask_ch = _image_mask.getConvexHull();
  for (auto it = mask_ch.begin(); it != mask_ch.end(); ++it) {
    const GVector::vector2d<int> &a = *it;
    vis_frame->data.drawFatBox(a.x - 3, a.y - 3, 7, 7, RGB::Orange);

    const GVector::vector2d<int> &b =
      std::next(it) != mask_ch.end() ? *std::next(it) : *mask_ch.begin();
    vis_frame->data.drawLine(a.x, a.y, b.x, b.y, RGB::Orange);
  }
  _image_mask.unlock();
}

ProcessResult PluginVisualize::process(
    FrameData* data, RenderOptions* options) {
  if (data == 0) return ProcessingFailed;

  VisualizationFrame* vis_frame =
      reinterpret_cast<VisualizationFrame*>(data->map.get("vis_frame"));
  if (vis_frame == 0) {
    vis_frame = reinterpret_cast<VisualizationFrame*>(
        data->map.insert("vis_frame",new VisualizationFrame()));
  }

  if (_v_enabled->getBool()) {
    //check video data...
    if (data->video.getWidth() == 0 || data->video.getHeight()==0) {
      //there is no valid video data
      //mark visualization data as invalid
      vis_frame->valid = false;
      return ProcessingOk;
    } else {
      //allocate visualization frame accordingly:
      vis_frame->data.allocate(data->video.getWidth(), data->video.getHeight());
    }

    // Draw camera image
    if (_v_image->getBool()) {
      DrawCameraImage(data, vis_frame);
    } else {
      vis_frame->data.fillBlack();
    }

    // Draw color-thresholded image.
    if (_v_thresholded->getBool()) {
      DrawThresholdedImage(data, vis_frame);
    }

    //draw blob finding results:
    if (_v_blobs->getBool()) {
      DrawBlobs(data, vis_frame);
    }

    // Camera calibration
    if (_v_camera_calibration->getBool()) {
      DrawCameraCalibration(data, vis_frame);
    }

    // Result of camera calibration, draws field to image
    if (_v_calibration_result->getBool()) {
      DrawCalibrationResult(data, vis_frame);
    }

    // Test edge detection for calibration
    if (_v_complete_sobel->getBool()) {
      DrawSobelImage(data, vis_frame);
    }

    // Result of edge detection for second calibration step
    if (_v_detected_edges->getBool()) {
      DrawDetectedEdges(data, vis_frame);
    }
    if(_v_mask_hull->getBool()) {
      DrawMaskHull(data, vis_frame);
    }
    vis_frame->valid = true;
  } else {
    vis_frame->valid = false;
  }
  return ProcessingOk;
}

void PluginVisualize::setThresholdingLUT(LUT3D * threshold_lut) {
  _threshold_lut=threshold_lut;
}

void PluginVisualize::drawFieldArc(
    const GVector::vector3d<double>& center,
    double radius, double theta1, double theta2, int steps,
    VisualizationFrame* vis_frame,
    unsigned char r, unsigned char g, unsigned char b) {
  const double delta = (theta2 - theta1) / static_cast<double>(steps);
  GVector::vector2d<double> lastInImage(0.0, 0.0);
  GVector::vector3d<double> lastInWorld =  center +
      radius * GVector::vector3d<double>(cos(theta1), sin(theta1), 0.0);
  camera_parameters.field2image(lastInWorld, lastInImage);
  for (int i = 1; i <= steps; ++i) {
    const double theta = theta1 + static_cast<double>(i) * delta;
    GVector::vector3d<double> nextInWorld =  center +
        radius * GVector::vector3d<double>(cos(theta), sin(theta), 0.0);
    GVector::vector2d<double> nextInImage(0.0, 0.0);
    camera_parameters.field2image(nextInWorld, nextInImage);
    rgb draw_color;
    draw_color.set(r,g,b);
    vis_frame->data.drawFatLine(
        lastInImage.x,lastInImage.y,nextInImage.x,nextInImage.y,draw_color);
    lastInWorld = nextInWorld;
    lastInImage = nextInImage;
  }
}

void PluginVisualize::drawFieldLine(
    const GVector::vector3d<double>& start,
    const GVector::vector3d<double>& end, int steps,
    VisualizationFrame* vis_frame,
    unsigned char r, unsigned char g, unsigned char b) {
  const GVector::vector3d<double> delta =
      (end - start) / static_cast<double>(steps);
  GVector::vector2d<double> lastInImage(0.0, 0.0);
  GVector::vector3d<double> lastInWorld(start);
  camera_parameters.field2image(lastInWorld, lastInImage);
  for (int i = 0; i < steps; ++i) {
    GVector::vector3d<double> nextInWorld = lastInWorld + delta;
    GVector::vector2d<double> nextInImage;
    camera_parameters.field2image(nextInWorld, nextInImage);
    rgb draw_color;
    draw_color.set(r,g,b);
    vis_frame->data.drawFatLine(
        lastInImage.x,lastInImage.y,nextInImage.x,nextInImage.y,draw_color);

    lastInWorld = nextInWorld;
    lastInImage = nextInImage;
  }
}

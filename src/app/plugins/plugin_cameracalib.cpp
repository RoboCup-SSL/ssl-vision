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
  \file    plugin_cameracalib.cpp
  \brief   C++ Implementation: plugin_cameracalib
  \author  Tim Laue, 2009
*/
//========================================================================

#include "plugin_cameracalib.h"
#include "conversions.h"
#include "sobel.h"
#include <algorithm>
#include <QTabWidget>
#include <QStackedWidget>

using std::swap;

PluginCameraCalibration::PluginCameraCalibration(
    FrameBuffer* _buffer, CameraParameters& camera_params,
    RoboCupField& _field) :
    VisionPlugin(_buffer), camera_parameters(camera_params),
    field(_field),
    ccw(0), grey_image(0), rgb_image(0), doing_drag(false), drag_x(0),
    drag_y(0) {
  video_width=video_height=0;
  settings=new VarList("Camera Calibrator");
  settings->addChild(camera_settings = new VarList("Camera Parameters"));
  camera_params.addSettingsToList(*camera_settings);
  settings->addChild(calibration_settings =
      new VarList("Calibration Parameters"));
  camera_parameters.additional_calibration_information->addSettingsToList(
      *calibration_settings);
}

PluginCameraCalibration::~PluginCameraCalibration() {
  delete camera_settings;
  delete calibration_settings;
  if(grey_image)
    delete grey_image;
  if(rgb_image)
    delete rgb_image;
}

void PluginCameraCalibration::detectEdges(FrameData* data) {
  double point_separation(camera_parameters.additional_calibration_information->
      pointSeparation->getDouble());
  // Reset list:
  camera_parameters.calibrationSegments.clear();
  // Get a greyscale image:
  if(grey_image == 0) {
    grey_image = new greyImage(data->video.getWidth(),data->video.getHeight());
    rgb_image = new rgbImage(data->video.getWidth(),data->video.getHeight());
  }
  if (data->video.getColorFormat()==COLOR_YUV422_UYVY) {
    Conversions::uyvy2rgb(
        data->video.getData(),
        reinterpret_cast<unsigned char*>(rgb_image->getData()),
        data->video.getWidth(),data->video.getHeight());
    Images::convert(*rgb_image, *grey_image);
  } else if (data->video.getColorFormat()==COLOR_RGB8) {
    Images::convert(data->video, *grey_image);
  } else {
    fprintf(stderr, "ColorThresholding needs YUV422 or RGB8 as input image, "
            "but found: %s\n",
            Colors::colorFormatToString(data->video.getColorFormat()).c_str());
    return;
  }

  field.field_markings_mutex.lockForRead();
  for (size_t i = 0; i < field.field_lines.size(); ++i) {
    const FieldLine& line = *(field.field_lines[i]);
    const GVector::vector3d<double> p1(
        line.p1_x->getDouble(), line.p1_y->getDouble(), 0.0);
    const GVector::vector3d<double> p2(
        line.p2_x->getDouble(), line.p2_y->getDouble(), 0.0);
    detectEdgesOnSingleLine(
        p1, p2, line.thickness->getDouble(), point_separation);
  }

  for (size_t i = 0; i < field.field_arcs.size(); ++i) {
    const FieldCircularArc& arc = *(field.field_arcs[i]);
    const GVector::vector3d<double> center(
        arc.center_x->getDouble(), arc.center_y->getDouble(), 0.0);
    detectEdgesOnSingleArc(center, arc.radius->getDouble(), arc.a1->getDouble(),
        arc.a2->getDouble(), arc.thickness->getDouble(), point_separation);
  }
  field.field_markings_mutex.unlock();
}

ProcessResult PluginCameraCalibration::process(
    FrameData* data, RenderOptions* options) {
  video_width=data->video.getWidth();
  video_height=data->video.getHeight();
  if(camera_parameters.additional_calibration_information->imageWidth->getInt() != video_width ||
     camera_parameters.additional_calibration_information->imageHeight->getInt() != video_height){
      camera_parameters.additional_calibration_information->imageWidth->setInt(video_width);
      camera_parameters.additional_calibration_information->imageHeight->setInt(video_height);
  }
  (void)options;
  if(ccw) {
    if(ccw->getDetectEdges()) {
      detectEdges(data);
      // detectEdges2(data);
      ccw->resetDetectEdges();
    }
    ccw->set_slider_from_vars();
  }
  return ProcessingOk;
}

VarList * PluginCameraCalibration::getSettings() {
  return settings;
}

string PluginCameraCalibration::getName() {
  return "Camera Calibration";
}

QWidget * PluginCameraCalibration::getControlWidget() {
  if (ccw==0)
    ccw = new CameraCalibrationWidget(camera_parameters);

  return (QWidget *)ccw;
}

void PluginCameraCalibration::sanitizeSobel(
    greyImage* img, GVector::vector2d<double>& val, int sobel_border) {
  val.x = bound<double>(val.x, sobel_border, img->getWidth() - sobel_border);
  val.y = bound<double>(val.y, sobel_border, img->getHeight() - sobel_border);
}

void PluginCameraCalibration::detectEdgesOnSingleLine(
    const GVector::vector3d<double>& p1,
    const GVector::vector3d<double>& p2,
    double thickness, double point_separation) {
  static const int kSobelThreshold = 20;
  const double search_distance =
      camera_parameters.additional_calibration_information->
      line_search_corridor_width->getDouble() / 2.0;
  const double image_boundary =
      camera_parameters.additional_calibration_information->
      image_boundary->getDouble();
  const double max_feature_distance =
      camera_parameters.additional_calibration_information->
      max_feature_distance->getDouble();
  CameraParameters::CalibrationData calibration_data;
  calibration_data.straightLine = true;
  calibration_data.p1 = p1;
  calibration_data.p2 = p2;
  const double line_length = (p2 - p1).length();
  const GVector::vector3d<double> line_dir = (p2 - p1) / line_length;
  const GVector::vector3d<double> line_perp(-line_dir.y, line_dir.x, 0.0);
  const int num_points = floor(line_length / point_separation - 1.0);
  for (int i = 1; i <= num_points; ++i) {
    const double alpha =
        1.0 - static_cast<double>(i) / (static_cast<double>(num_points) + 1.0);
    const GVector::vector3d<double> p_world = alpha * p1 + (1.0 - alpha) * p2;
    GVector::vector2d<double> p_image(0.0, 0.0);
        camera_parameters.field2image(p_world, p_image);
    if (p_image.x < image_boundary ||
        p_image.x > grey_image->getWidth() - image_boundary ||
        p_image.y < image_boundary ||
        p_image.y > grey_image->getHeight() - image_boundary) {
      // This edge feature is outside the image boundary, so ignore it since
      // the edge detection might not be accurate.
      continue;
    }
    if ((p_world - camera_parameters.getWorldLocation()).length() >
       max_feature_distance) {
      // This edge feature too far away from the camera, so ignore it since the
      // line feature is likely to be too small to detect accurately.
      continue;
    }
    const GVector::vector3d<double> start_world =
        p_world + line_perp * (0.5 * thickness + search_distance);
    const GVector::vector3d<double> end_world =
        p_world - line_perp * (0.5 * thickness + search_distance);
    GVector::vector2d<double> start_image(0.0, 0.0), end_image(0.0, 0.0);
    camera_parameters.field2image(start_world, start_image);
    camera_parameters.field2image(end_world, end_image);
    sanitizeSobel(grey_image, start_image);
    sanitizeSobel(grey_image, end_image);

    GVector::vector2d<double> point_image;
    bool center_found = false;
    Sobel::centerOfLine(
        *grey_image, start_image.x, end_image.x, start_image.y,
        end_image.y, point_image, center_found, kSobelThreshold);
    calibration_data.imgPts.push_back(std::make_pair(point_image,center_found));
    calibration_data.alphas.push_back(alpha);
  }
  if (calibration_data.imgPts.size() > 2) {
    // Need at least two points to uniquely define a line segment.
    camera_parameters.calibrationSegments.push_back(calibration_data);
  }
}

void PluginCameraCalibration::detectEdgesOnSingleArc(
    const GVector::vector3d<double>& center, double radius, double theta1,
    double theta2, double thickness, double point_separation) {
  static const int kSobelThreshold = 20;
  const double search_distance =
      camera_parameters.additional_calibration_information->
      line_search_corridor_width->getDouble() / 2.0;
  const double image_boundary =
      camera_parameters.additional_calibration_information->
      image_boundary->getDouble();
  const double max_feature_distance =
      camera_parameters.additional_calibration_information->
      max_feature_distance->getDouble();
  CameraParameters::CalibrationData calibration_data;
  calibration_data.straightLine = false;
  calibration_data.radius = radius;
  calibration_data.center = center;
  calibration_data.theta1 = theta1;
  calibration_data.theta2 = theta2;
  const int num_points =
      floor((theta2 - theta1) * radius / point_separation - 1.0);
  for (int i = 1; i <= num_points; ++i) {
    const double alpha =
        1.0 - static_cast<double>(i) / (static_cast<double>(num_points) + 1.0);
    const double theta = alpha * theta1 + (1.0 - alpha) * theta2;
    const GVector::vector3d<double> radius_vector(cos(theta), sin(theta), 0.0);
    const GVector::vector3d<double> p_world = center + radius * radius_vector;
    GVector::vector2d<double> p_image(0.0, 0.0);
    camera_parameters.field2image(p_world, p_image);
    if (p_image.x < image_boundary ||
        p_image.x > grey_image->getWidth() - image_boundary ||
        p_image.y < image_boundary ||
        p_image.y > grey_image->getHeight() - image_boundary) {
      // This edge feature is outside the image boundary, so ignore it since
      // the edge detection might not be accurate.
      continue;
    }
    if ((p_world - camera_parameters.getWorldLocation()).length() >
       max_feature_distance) {
      // This edge feature too far away from the camera, so ignore it since the
      // line feature is likely to be too small to detect accurately.
      continue;
    }
    const GVector::vector3d<double> start_world =
        center + (radius - 0.5 * thickness - search_distance) * radius_vector;
    const GVector::vector3d<double> end_world =
        center + (radius + 0.5 * thickness + search_distance) * radius_vector;
    GVector::vector2d<double> start_image(0.0, 0.0), end_image(0.0, 0.0);
    camera_parameters.field2image(start_world, start_image);
    camera_parameters.field2image(end_world, end_image);
    sanitizeSobel(grey_image, start_image);
    sanitizeSobel(grey_image, end_image);

    GVector::vector2d<double> point_image;
    bool center_found = false;
    Sobel::centerOfLine(
        *grey_image, start_image.x, end_image.x, start_image.y,
        end_image.y, point_image, center_found, kSobelThreshold);
    calibration_data.imgPts.push_back(std::make_pair(point_image,center_found));
    calibration_data.alphas.push_back(alpha);
  }
  if (calibration_data.imgPts.size() > 3) {
    // Need at least three points to uniquely define a circular arc.
    camera_parameters.calibrationSegments.push_back(calibration_data);
  }
}

void PluginCameraCalibration::mouseEvent( QMouseEvent * event, pixelloc loc)
{
  (void) event;
  (void) loc;
}

void PluginCameraCalibration::keyPressEvent ( QKeyEvent * event )
{
  (void) event;
}

void PluginCameraCalibration::mousePressEvent ( QMouseEvent * event, pixelloc loc )
{
  QTabWidget* tabw = (QTabWidget*) ccw->parentWidget()->parentWidget();
  double drag_threshold = 20; //in px
  if (tabw->currentWidget() == ccw && (event->buttons() & Qt::LeftButton)!=0) {
    drag_x = 0;
    drag_y = 0;
    for (int i = 0;
        i < CameraParameters::AdditionalCalibrationInformation::kNumControlPoints;
        ++i) {
      const double x_diff =
          camera_parameters.additional_calibration_information->
              control_point_image_xs[i]->getDouble() - loc.x;
      const double y_diff =
          camera_parameters.additional_calibration_information->
              control_point_image_ys[i]->getDouble() - loc.y;
      if (sqrt(x_diff*x_diff + y_diff*y_diff) < drag_threshold) {
        drag_x = camera_parameters.additional_calibration_information->
            control_point_image_xs[i];
        drag_y = camera_parameters.additional_calibration_information->
            control_point_image_ys[i];
        break;
      }
    }
    if (drag_x != 0 && drag_y != 0)
    {
      event->accept();
      doing_drag = true;
    }
    else
      event->ignore();
  } else {
    event->ignore();
  }

}

void PluginCameraCalibration::mouseReleaseEvent ( QMouseEvent * event, pixelloc loc )
{
  (void)loc;
  QTabWidget* tabw = (QTabWidget*) ccw->parentWidget()->parentWidget();
  if (tabw->currentWidget() == ccw)
  {
    doing_drag =false;
    event->accept();
  }
  else
    event->ignore();
}

void PluginCameraCalibration::mouseMoveEvent ( QMouseEvent * event, pixelloc loc )
{
  QTabWidget* tabw = (QTabWidget*) ccw->parentWidget()->parentWidget();
  if (doing_drag && tabw->currentWidget() == ccw && (event->buttons() & Qt::LeftButton)!=0)
  {
    if (loc.x < 0) loc.x=0;
    if (loc.y < 0) loc.y=0;
    if (video_width > 0 && loc.x >= video_width) loc.x=video_width-1;
    if (video_height > 0 && loc.y >= video_height) loc.y=video_height-1;
    drag_x->setDouble(loc.x);
    drag_y->setDouble(loc.y);
    event->accept();
  }
  else
    event->ignore();
}

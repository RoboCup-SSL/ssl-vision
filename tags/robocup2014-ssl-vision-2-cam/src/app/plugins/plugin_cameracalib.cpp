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
#include <QTabWidget>
#include <QStackedWidget>

PluginCameraCalibration::PluginCameraCalibration(FrameBuffer * _buffer, 
                                                 CameraParameters& camera_params,
                                                 RoboCupCalibrationHalfField& _field) 
  : VisionPlugin(_buffer), camera_parameters(camera_params), field(_field), ccw(0),
     grey_image(0), rgb_image(0), doing_drag(false), drag_x(0), drag_y(0)
{
  video_width=video_height=0;
  settings=new VarList("Camera Calibrator");
  settings->addChild(camera_settings = new VarList("Camera Parameters"));
  camera_params.addSettingsToList(*camera_settings);
  settings->addChild(field_settings = new VarList("Field Half Configuration"));
  field.addSettingsToList(*field_settings);
  settings->addChild(calibration_settings = new VarList("Calibration Parameters"));
  camera_parameters.additional_calibration_information->addSettingsToList(*calibration_settings);
}

PluginCameraCalibration::~PluginCameraCalibration()
{
  delete camera_settings;
  delete field_settings;
  delete calibration_settings;
  if(grey_image)
    delete grey_image;
  if(rgb_image)
    delete rgb_image;
}

ProcessResult PluginCameraCalibration::process(FrameData * data, RenderOptions * options) 
{
  video_width=data->video.getWidth();
  video_height=data->video.getHeight();
  (void)options;
  if(ccw)
  {
    if(ccw->getDetectEdges())
    {
      detectEdges(data);
      ccw->resetDetectEdges();
    }
    ccw->set_slider_from_vars();
  }
  return ProcessingOk;
}

VarList * PluginCameraCalibration::getSettings() 
{
  return settings;
}

string PluginCameraCalibration::getName() 
{
  return "Camera Calibration";
}

QWidget * PluginCameraCalibration::getControlWidget() 
{
  if (ccw==0) 
    ccw = new CameraCalibrationWidget(camera_parameters);

  return (QWidget *)ccw;
}

void PluginCameraCalibration::detectEdges(FrameData * data)
{
  //std::cout<<"Detect Edges"<<std::endl;
  // Set config value:
  int pointsPerLine(camera_parameters.additional_calibration_information->pointsPerLine->getInt());
  int pointsInsideGoal(camera_parameters.additional_calibration_information->pointsInsideGoal->getInt());
  int pointsInsideCenterCircle(camera_parameters.additional_calibration_information->pointsInsideCenterCircle->getInt());
  int pointsOnCenterCircle(camera_parameters.additional_calibration_information->pointsOnCenterCircle->getInt());
  int pointsOnDefenseAreaArc(camera_parameters.additional_calibration_information->pointsOnDefenseAreaArc->getInt());
  int pointsOnDefenseStretch(camera_parameters.additional_calibration_information->pointsOnDefenseStretch->getInt());
  // Reset list:
  camera_parameters.calibrationSegments.clear();
  // Get a greyscale image:
  if(grey_image == 0)
  {
    grey_image = new greyImage(data->video.getWidth(),data->video.getHeight());
    rgb_image = new rgbImage(data->video.getWidth(),data->video.getHeight());
  }
  if (data->video.getColorFormat()==COLOR_YUV422_UYVY) 
  {
    Conversions::uyvy2rgb(data->video.getData(),(unsigned char*)(rgb_image->getData()),data->video.getWidth(),data->video.getHeight());
    Images::convert(*rgb_image, *grey_image);
    //std::cout<<"Hmmm. YUV422."<<std::endl;
  } 
  else if (data->video.getColorFormat()==COLOR_RGB8) 
  {
    Images::convert(data->video, *grey_image);
    //std::cout<<"OK. RGB8."<<std::endl;
  } 
  else 
  {
    fprintf(stderr,"ColorThresholding needs YUV422 or RGB8 as input image, but found: %s\n",Colors::colorFormatToString(data->video.getColorFormat()).c_str());
    return;
  }
  
  const RoboCupCalibrationHalfField *field = &camera_parameters.field;
  // Find edges on left field side:
  GVector::vector3d<double> start;
  start.x = field->left_corner_x->getDouble();
  start.y = field->left_corner_y->getDouble();
  start.z = 0;
  GVector::vector3d<double> end;
  end.x = field->left_centerline_x->getDouble();
  end.y = field->left_centerline_y->getDouble();
  end.z = 0;
  detectEdgesOnSingleLine(start,end,pointsPerLine);
  
  // Find edges on right field side:
  start.x = field->right_corner_x->getDouble();
  start.y = field->right_corner_y->getDouble();
  end.x = field->right_centerline_x->getDouble();
  end.y = field->right_centerline_y->getDouble();
  detectEdgesOnSingleLine(start,end,pointsPerLine);
  
  // Find edges along goal line:
  start.x = field->left_corner_x->getDouble();
  start.y = field->left_corner_y->getDouble();
  end.x = field->left_goal_area_x->getDouble();
  end.y = field->left_goal_area_y->getDouble();
  detectEdgesOnSingleLine(start,end,(pointsPerLine-pointsInsideGoal)/2);
  start.x = field->right_goal_area_x->getDouble();
  start.y = field->right_goal_area_y->getDouble();
  end.x = field->right_corner_x->getDouble();
  end.y = field->right_corner_y->getDouble();
  detectEdgesOnSingleLine(start,end,(pointsPerLine-pointsInsideGoal)/2);
  if(field->useFeaturesInGoal->getBool()){
    start.x = field->left_goal_post_x->getDouble();
    start.y = field->left_goal_post_y->getDouble();
    end.x = field->right_goal_post_x->getDouble();
    end.y = field->right_goal_post_y->getDouble();
    detectEdgesOnSingleLine(start,end,pointsInsideGoal);
  }
  
  // Find edges along center line:
  start.x = field->left_centerline_x->getDouble();
  start.y = field->left_centerline_y->getDouble();
  end.x = field->left_centercircle_x->getDouble();
  end.y = field->left_centercircle_y->getDouble();
  detectEdgesOnSingleLine(start,end,(pointsPerLine-pointsInsideCenterCircle)/2, true);
  start.x = field->right_centercircle_x->getDouble();
  start.y = field->right_centercircle_y->getDouble();
  end.x = field->right_centerline_x->getDouble();
  end.y = field->right_centerline_y->getDouble();
  detectEdgesOnSingleLine(start,end,(pointsPerLine-pointsInsideCenterCircle)/2, true);
  start.x = field->left_centercircle_x->getDouble();
  start.y = field->left_centercircle_y->getDouble();
  end.x = field->right_centercircle_x->getDouble();
  end.y = field->right_centercircle_y->getDouble();
  detectEdgesOnSingleLine(start,end,pointsInsideCenterCircle, true);
  
  if(field->useFeaturesOnCenterCircle->getBool()){
    //Find edges along center circle:
    GVector::vector3d<double> center(field->centerpoint_x->getDouble(),field->centerpoint_y->getDouble(),0.0);
    double radius = field->centercircle_radius->getDouble();
    double theta1 = atan2(field->right_centerline_y->getDouble()-field->centerpoint_y->getDouble(), 
                          field->right_centerline_x->getDouble()-field->centerpoint_x->getDouble());
    double theta2 = atan2(field->left_centerline_y->getDouble()-field->centerpoint_y->getDouble(), 
                          field->left_centerline_x->getDouble()-field->centerpoint_x->getDouble());
    if(field->right_centerline_y->getDouble()>0.0){
      theta1+=M_PI;
      theta2+=M_PI;
    }
    detectEdgesOnSingleArc(center,radius,theta1,theta2,pointsOnCenterCircle);
  }
  if(field->useFeaturesInDefenseArea->getBool()){
    //Find edges along border of defense area:
    GVector::vector3d<double> center;
    double radius = field->defense_area_radius->getDouble();
    double theta1, theta2;
    
    //Detect edges on the arc on the left side of defense area
    center.x = 0.5*(field->right_goal_post_x->getDouble() + field->left_goal_post_x->getDouble());
    center.y = 0.5*(field->right_goal_post_y->getDouble() + field->left_goal_post_y->getDouble()) - 0.5*field->defense_stretch->getDouble();
    theta1 = 2.0*M_PI + atan2(field->left_goal_area_y->getDouble()-field->left_goal_post_y->getDouble(),
                   field->left_goal_area_x->getDouble()-field->left_goal_post_x->getDouble());
    theta2 = 2.0*M_PI;
    if(field->right_centerline_y->getDouble()<0.0){
      theta1+=M_PI;
      theta2+=M_PI;
    }
    detectEdgesOnSingleArc(center,radius,theta1,theta2,pointsOnDefenseAreaArc);
    
    //Detect edges on the arc on the right side of defense area
    center.x = 0.5*(field->right_goal_post_x->getDouble() + field->left_goal_post_x->getDouble());
    center.y = 0.5*(field->right_goal_post_y->getDouble() + field->left_goal_post_y->getDouble()) + 0.5*field->defense_stretch->getDouble();
    theta1 = 0.0;
    theta2 = atan2(field->right_goal_area_y->getDouble()-field->right_goal_post_y->getDouble(),
                   field->right_goal_area_x->getDouble()-field->right_goal_post_x->getDouble());
    if(field->right_centerline_y->getDouble()<0.0){
      theta1+=M_PI;
      theta2+=M_PI;
    }
    detectEdgesOnSingleArc(center,radius,theta1,theta2,pointsOnDefenseAreaArc);
    
    //Detect edges on Defense Stretch
    if(field->right_centerline_y->getDouble()<0.0){
      start.x = 0.5*(field->right_goal_post_x->getDouble() + field->left_goal_post_x->getDouble()) - radius;
    }else{
      start.x = 0.5*(field->right_goal_post_x->getDouble() + field->left_goal_post_x->getDouble()) + radius;
    }
    start.y = 0.5*(field->right_goal_post_y->getDouble() + field->left_goal_post_y->getDouble()) - 0.5*field->defense_stretch->getDouble();
    end.x = start.x;
    end.y = 0.5*(field->right_goal_post_y->getDouble() + field->left_goal_post_y->getDouble()) + 0.5*field->defense_stretch->getDouble();
    detectEdgesOnSingleLine(start,end,pointsOnDefenseStretch, true);
  }
}


void PluginCameraCalibration::sanitizeSobel(greyImage * img, GVector::vector2d<double> & val,int sobel_border) {
  if (val.x < (sobel_border)) val.x=sobel_border;
  if (val.x > (img->getWidth()-sobel_border)) val.x=(img->getWidth()-sobel_border);
  if (val.y < (sobel_border)) val.y=sobel_border;
  if (val.y > (img->getHeight()-sobel_border)) val.y=(img->getHeight()-sobel_border);
}

void PluginCameraCalibration::detectEdgesOnSingleArc(
    const GVector::vector3d<double>& center,
    double radius, double theta1, double theta2,
    int numPoints)
{
  int threshold(20);
  double distToLine = camera_parameters.additional_calibration_information->line_search_corridor_width->getDouble() / 2.0;
  CameraParameters::CalibrationData calData;
  calData.straightLine = false;
  calData.radius = radius;
  calData.center = center;
  calData.theta1 = theta1;
  calData.theta2 = theta2;
  
  for(int i=1;i<=numPoints;i++){
    double alpha = 1.0 - ((double) i)/((double) numPoints + 1.0);
    double theta = alpha*theta1+(1.0-alpha)*theta2;
    GVector::vector3d<double> radiusVector(cos(theta),sin(theta),0.0);
    GVector::vector3d<double> posInWorld = center+radius*radiusVector;
    GVector::vector3d<double> worldStart = center+(radius-distToLine)*radiusVector;
    GVector::vector3d<double> worldEnd = center+(radius+distToLine)*radiusVector;
    GVector::vector2d<double> imgStart, imgEnd;
    
    camera_parameters.field2image(worldStart,imgStart);  
    camera_parameters.field2image(worldEnd,imgEnd);
    sanitizeSobel(grey_image,imgStart);
    sanitizeSobel(grey_image,imgEnd);
    
    GVector::vector2d<double> arcPoint;
    bool centerFound;
    Sobel::centerOfLine(*grey_image, imgStart.x, imgEnd.x, imgStart.y, imgEnd.y, arcPoint, centerFound, threshold);
    calData.imgPts.push_back(std::make_pair(arcPoint,centerFound));
    calData.alphas.push_back(alpha);
  }
  camera_parameters.calibrationSegments.push_back(calData);
}

void PluginCameraCalibration::detectEdgesOnSingleLine(
    const GVector::vector3d<double>& start,
    const GVector::vector3d<double>& end,
    int pointsPerLine, bool detectCenter)
{
  int threshold(20);
  double distToLine = camera_parameters.additional_calibration_information->line_search_corridor_width->getDouble() / 2.0;
  ImageSide imageSide = getImageSide(start,end);
  GVector::vector3d<double> offset = (end - start) / ((double)pointsPerLine+1.0);
  CameraParameters::CalibrationData calData;
  calData.straightLine = true;
  calData.p1 = start;
  calData.p2 = end;
  for(int i=1; i<=pointsPerLine; ++i)
  {
    GVector::vector3d<double> posInWorld = start + (offset*i);
    GVector::vector3d<double> worldStart(posInWorld), worldEnd(posInWorld);
    GVector::vector2d<double> imgStart, imgEnd;
    int x,y;
    double dummy;
    switch(imageSide)
    {
      case IMG_LEFT:
        worldStart.y -= distToLine;
        worldEnd.y += distToLine;
        camera_parameters.field2image(worldStart,imgStart);  
        camera_parameters.field2image(worldEnd,imgEnd);
        sanitizeSobel(grey_image,imgStart);
        sanitizeSobel(grey_image,imgEnd);
        y = (imgStart.y + imgEnd.y) / 2;
        if(imgStart.x > imgEnd.x){
          dummy = imgEnd.x;
          imgEnd.x = imgStart.x;
          imgStart.x = dummy;
        }
        if(detectCenter)
          x = Sobel::centerOfVerticalLine(*grey_image, y, imgStart.x, imgEnd.x, threshold);
        else
          x = Sobel::maximumHorizontalEdge(*grey_image, y, imgStart.x, imgEnd.x,
                                          threshold, Sobel::horizontalBrighter);
        if(x!=-1){
          calData.imgPts.push_back(std::make_pair(GVector::vector2d<double>(x,y),true));
          calData.alphas.push_back(1.0-((double)i)/((double)pointsPerLine+1.0));
        }else{
          x = (imgStart.x + imgEnd.x) / 2;
          calData.imgPts.push_back(std::make_pair(GVector::vector2d<double>(x,y),false));
          calData.alphas.push_back(1.0-((double)i)/((double)pointsPerLine+1.0));
        }
        calData.horizontal = true;
        break;
      case IMG_RIGHT:
        worldStart.y -= distToLine;
        worldEnd.y += distToLine;
        camera_parameters.field2image(worldStart,imgStart);  
        camera_parameters.field2image(worldEnd,imgEnd);
        sanitizeSobel(grey_image,imgStart);
        sanitizeSobel(grey_image,imgEnd);
        y = (imgStart.y + imgEnd.y) / 2;
        if(imgStart.x > imgEnd.x){
          dummy = imgEnd.x;
          imgEnd.x = imgStart.x;
          imgStart.x = dummy;
        }
        
        if(detectCenter)
          x = Sobel::centerOfVerticalLine(*grey_image, y, imgStart.x, imgEnd.x, threshold);
        else
          x = Sobel::maximumHorizontalEdge(*grey_image, y, imgStart.x, imgEnd.x,
                                          threshold, Sobel::horizontalDarker);
        if(x!=-1){
          calData.imgPts.push_back(std::make_pair(GVector::vector2d<double>(x,y),true));
          calData.alphas.push_back(1.0-((double)i)/((double)pointsPerLine+1.0));
        }else{
          x = (imgStart.x + imgEnd.x) / 2;
          calData.imgPts.push_back(std::make_pair(GVector::vector2d<double>(x,y),false));
          calData.alphas.push_back(1.0-((double)i)/((double)pointsPerLine+1.0));
        }
        calData.horizontal = true;
        break;
      case IMG_TOP:
        worldStart.x -= distToLine;
        worldEnd.x += distToLine;
        camera_parameters.field2image(worldStart,imgStart);  
        camera_parameters.field2image(worldEnd,imgEnd);
        sanitizeSobel(grey_image,imgStart);
        sanitizeSobel(grey_image,imgEnd);
        x = (imgStart.x + imgEnd.x) / 2;
        if(imgStart.y > imgEnd.y){
          dummy = imgEnd.y;
          imgEnd.y = imgStart.y;
          imgStart.y = dummy;
        }
        if(detectCenter)
          y = Sobel::centerOfHorizontalLine(*grey_image, x, imgStart.y, imgEnd.y, threshold);
        else
          y = Sobel::maximumVerticalEdge(*grey_image, x, imgStart.y, imgEnd.y,
                                          threshold, Sobel::verticalBrighter);
        if(y > 0){
          calData.imgPts.push_back(std::make_pair(GVector::vector2d<double>(x,y),true));
          calData.alphas.push_back(1.0-((double)i)/((double)pointsPerLine+1.0));
        }else{
          y = (imgStart.y + imgEnd.y) / 2;
          calData.imgPts.push_back(std::make_pair(GVector::vector2d<double>(x,y),false));
          calData.alphas.push_back(1.0-((double)i)/((double)pointsPerLine+1.0));
        }
        calData.horizontal = false;
        break;
      case IMG_BOTTOM:
        worldStart.x -= distToLine;
        worldEnd.x += distToLine;
        camera_parameters.field2image(worldStart,imgStart);  
        camera_parameters.field2image(worldEnd,imgEnd);
        sanitizeSobel(grey_image,imgStart);
        sanitizeSobel(grey_image,imgEnd);
        x = (imgStart.x + imgEnd.x) / 2;
        if(imgStart.y > imgEnd.y){
          dummy = imgEnd.y;
          imgEnd.y = imgStart.y;
          imgStart.y = dummy;
        }
        if(detectCenter)
          y = Sobel::centerOfHorizontalLine(*grey_image, x, imgStart.y, imgEnd.y, threshold);
        else
          y = Sobel::maximumVerticalEdge(*grey_image, x, imgStart.y, imgEnd.y,
                                          threshold, Sobel::verticalDarker);
        if(y > 0){
          calData.imgPts.push_back(std::make_pair(GVector::vector2d<double>(x,y),true));
          calData.alphas.push_back(1.0-((double)i)/((double)pointsPerLine+1.0));
        }else{
          y = (imgStart.y + imgEnd.y) / 2;
          calData.imgPts.push_back(std::make_pair(GVector::vector2d<double>(x,y),false));
          calData.alphas.push_back(1.0-((double)i)/((double)pointsPerLine+1.0));
        }
        calData.horizontal = false;
        break;
    };
  }
  camera_parameters.calibrationSegments.push_back(calData);
}

PluginCameraCalibration::ImageSide PluginCameraCalibration::getImageSide(
    const GVector::vector3d<double>& start,
    const GVector::vector3d<double>& end)
{
  GVector::vector2d<double> imgStart, imgEnd;
  camera_parameters.field2image(start,imgStart);
  camera_parameters.field2image(end,imgEnd);
  const double centerX = camera_parameters.principal_point_x->getDouble();
  const double centerY = camera_parameters.principal_point_y->getDouble();
  GVector::vector2d<double> vec(imgEnd-imgStart);
  if(fabs(vec.x) > fabs(vec.y)) // Horizontal line
  {
    if(imgStart.y < centerY)
      return IMG_TOP;
    else
      return IMG_BOTTOM;
  }
  else // Vertical line
  {
    if(imgStart.x < centerX)
      return IMG_LEFT;
    else
      return IMG_RIGHT;
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
    double x_diff = camera_parameters.additional_calibration_information->left_corner_image_x->getDouble() - loc.x;
    double y_diff = camera_parameters.additional_calibration_information->left_corner_image_y->getDouble() - loc.y;
    if (sqrt(x_diff*x_diff + y_diff*y_diff) < drag_threshold)
    {
      drag_x = camera_parameters.additional_calibration_information->left_corner_image_x;
      drag_y = camera_parameters.additional_calibration_information->left_corner_image_y;
    }
    x_diff = camera_parameters.additional_calibration_information->right_corner_image_x->getDouble() - loc.x;
    y_diff = camera_parameters.additional_calibration_information->right_corner_image_y->getDouble() - loc.y;
    if (sqrt(x_diff*x_diff + y_diff*y_diff) < drag_threshold)
    {
      drag_x = camera_parameters.additional_calibration_information->right_corner_image_x;
      drag_y = camera_parameters.additional_calibration_information->right_corner_image_y;
    }
    x_diff = camera_parameters.additional_calibration_information->left_centerline_image_x->getDouble() - loc.x;
    y_diff = camera_parameters.additional_calibration_information->left_centerline_image_y->getDouble() - loc.y;
    if (sqrt(x_diff*x_diff + y_diff*y_diff) < drag_threshold)
    {
      drag_x = camera_parameters.additional_calibration_information->left_centerline_image_x;
      drag_y = camera_parameters.additional_calibration_information->left_centerline_image_y;
    }
    x_diff = camera_parameters.additional_calibration_information->right_centerline_image_x->getDouble() - loc.x;
    y_diff = camera_parameters.additional_calibration_information->right_centerline_image_y->getDouble() - loc.y;
    if (sqrt(x_diff*x_diff + y_diff*y_diff) < drag_threshold)
    {
      drag_x = camera_parameters.additional_calibration_information->right_centerline_image_x;
      drag_y = camera_parameters.additional_calibration_information->right_centerline_image_y;
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

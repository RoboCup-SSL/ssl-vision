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
  \file    camera_calibration.h
  \brief   Datatypes for camera parameters, functions for perspective transformations
  \author  Tim Laue, (C) 2009
*/
//========================================================================

#include <VarDouble.h>
#include <VarList.h>
#include <quaternion.h>
#include <Eigen/Core>
#include "field.h"
#include "timer.h"

#ifndef CAMERA_CALIBRATION_H
#define CAMERA_CALIBRATION_H
#include "messages_robocup_ssl_geometry.pb.h"


//using namespace Eigen;
//USING_PART_OF_NAMESPACE_EIGEN

/*!
  \class CameraParameters

  \brief Intrinsic and extrinsic camera parameters.

  \author Tim Laue, (C) 2009
**/
class CameraParameters
{
public:

  class AdditionalCalibrationInformation;
  class CalibrationData;

  CameraParameters(int camera_index_, RoboCupField * field);
  ~CameraParameters();
  void addSettingsToList(VarList& list);

  VarDouble* focal_length;
  VarDouble* principal_point_x;
  VarDouble* principal_point_y;
  VarDouble* distortion;

  // Just for automatic saving purposes
  VarDouble* q0;
  VarDouble* q1;
  VarDouble* q2;
  VarDouble* q3;

  VarDouble* tx;
  VarDouble* ty;
  VarDouble* tz;

  std::vector<CalibrationData> calibrationSegments;

  Eigen::VectorXd p_alpha;
  Quaternion<double> q_rotate180;

  //Quaternion<double> q_field2cam;
  //GVector::vector3d<double> translation;

  AdditionalCalibrationInformation* additional_calibration_information;

  GVector::vector3d<double> getWorldLocation() const;
  void field2image(const GVector::vector3d<double> &p_f, GVector::vector2d<double> &p_i) const;
  void image2field(GVector::vector3d< double >& p_f, const GVector::vector2d< double >& p_i, double z) const;
  void calibrate(std::vector<GVector::vector3d<double> > &p_f, std::vector<GVector::vector2d<double> > &p_i, int cal_type);

  double radialDistortion(double ru) const;  //apply radial distortion to (undistorted) radius ru and return distorted radius
  double radialDistortionInv(double rd) const;  //invert radial distortion from (distorted) radius rd and return undistorted radius
  void radialDistortionInv(GVector::vector2d<double> &pu, const GVector::vector2d<double> &pd) const;
  void radialDistortion(const GVector::vector2d<double> pu, GVector::vector2d<double> &pd) const;
  double radialDistortion(double ru, double dist) const;
  void radialDistortion(const GVector::vector2d<double> pu, GVector::vector2d<double> &pd, double dist) const;

  double calc_chisqr(std::vector<GVector::vector3d<double> > &p_f, std::vector<GVector::vector2d<double> > &p_i, Eigen::VectorXd &p, int);
  void field2image(GVector::vector3d<double> &p_f, GVector::vector2d<double> &p_i, Eigen::VectorXd &p);

  void toProtoBuffer(SSL_GeometryCameraCalibration &buffer) const;

  enum
  {
    FOCAL_LENGTH=0,
    PP_X,
    PP_Y,
    DIST,
    Q_1,
    Q_2,
    Q_3,
    T_1,
    T_2,
    T_3,
    STATE_SPACE_DIMENSION
  };

  enum
  {
    FOUR_POINT_INITIAL = 1,
    FULL_ESTIMATION = 2
    // ... = 4 etc.
  };

  std::vector<int> p_to_est;

  /*!
  \class AdditionalCalibrationInformation
  \brief Some additional data used for calibration
  \author Tim Laue, (C) 2009
   **/
  class AdditionalCalibrationInformation
  {
    public:
      AdditionalCalibrationInformation(int camera_index_, RoboCupField* field);
      ~AdditionalCalibrationInformation();
      void addSettingsToList(VarList& list);
      void updateControlPoints();

      static const int kNumControlPoints = 4;
      VarInt* camera_index;
      VarList* control_point_set[kNumControlPoints];
      VarString* control_point_names[kNumControlPoints];
      VarDouble* control_point_image_xs[kNumControlPoints];
      VarDouble* control_point_image_ys[kNumControlPoints];
      VarDouble* control_point_field_xs[kNumControlPoints];
      VarDouble* control_point_field_ys[kNumControlPoints];

      VarDouble* initial_distortion;
      VarDouble* line_search_corridor_width;
      VarDouble* image_boundary;
      VarDouble* max_feature_distance;
      VarDouble* convergence_timeout;
      VarDouble* cov_corner_x;
      VarDouble* cov_corner_y;

      VarDouble* cov_ls_x;
      VarDouble* cov_ls_y;

      VarDouble* pointSeparation;

  private:
      RoboCupField* field;
      std::vector<GVector::vector2d<double> >
      generateCameraControlPoints(int cameraId, int numCamerasTotal, double fieldHeight, double fieldWidth);
  };

  /*!
  \class CalibrationData
  \brief Additional structure for holding information about
  image points on line segments
  \author OB, (C) 2009
   **/
  class CalibrationData {
   public:
    // False implies that it is an arc segment.
    bool straightLine;

    //Parameters for straight line segments
    //Start point of the straight line segment in world space coords
    GVector::vector3d<double> p1;
    //End point of the straight line segment in world space coords
    GVector::vector3d<double> p2;

    //Parameters for arc segments
    //center point of the arc segment in world space coords
    GVector::vector3d<double> center;
    //Start angle (Counter-clockwise, right-handed system) of the arc in world space
    double theta1;
    //End angle (Counter-clockwise, right-handed system) of the arc in world space
    double theta2;
    //Radius of the arc in world space
    double radius;

    //Image points, paired with a bool indicating whether the point was correctly detected
    std::vector<std::pair<GVector::vector2d<double>,bool> > imgPts;

    // Denotes the position along the line or arc. For each point, the
    // location of the point is "alpha * start + (1.0 - alpha) * end" where
    // "start" and "end" indicate the start and end points of the line / arc
    // segment. During calibration, this is varied to move the point along the
    // line / arc to minimize the fitting error - see how p_alpha varies.
    std::vector<double> alphas;

    CalibrationData() {
      straightLine = true;
    }
  };


public:
  void do_calibration(int cal_type);
  void reset();
};

#endif

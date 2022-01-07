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

#ifndef CAMERA_CALIBRATION_H
#define CAMERA_CALIBRATION_H

#include <VarDouble.h>
#include <VarList.h>
#include <quaternion.h>

#include <Eigen/Core>
#include <opencv2/opencv.hpp>

#include "camera_parameters.h"
#include "field.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "timer.h"

/*!
  \class CameraParameters

  \brief Intrinsic and extrinsic camera parameters.

  \author Tim Laue, (C) 2009
**/
class CameraParameters {
 public:
  class AdditionalCalibrationInformation;
  class CalibrationData;

  CameraParameters(int camera_index_, RoboCupField* field);
  ~CameraParameters();
  void addSettingsToList(VarList& list) const;

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

  AdditionalCalibrationInformation* additional_calibration_information;

  VarBool* use_opencv_model;
  CameraIntrinsicParameters* intrinsic_parameters;
  CameraExtrinsicParameters* extrinsic_parameters;

  void quaternionFromOpenCVCalibration(double Q[]) const;
  GVector::vector3d<double> getWorldLocation() const;
  void field2image(const GVector::vector3d<double>& p_f, GVector::vector2d<double>& p_i) const;
  void image2field(GVector::vector3d<double>& p_f, const GVector::vector2d<double>& p_i, double z) const;
  void calibrate(std::vector<GVector::vector3d<double> >& p_f,
                 std::vector<GVector::vector2d<double> >& p_i,
                 int cal_type);
  void calibrateExtrinsicModel(std::vector<GVector::vector3d<double> >& p_f,
                               std::vector<GVector::vector2d<double> >& p_i,
                               int cal_type) const;
  double calculateFourPointRmse(std::vector<GVector::vector3d<double> > &p_f,
                                std::vector<GVector::vector2d<double> > &p_i) const;
  void updateCalibrationDataPoints();
  double calculateCalibrationDataPointsRmse();

  /** apply radial distortion to (undistorted) radius ru and return distorted radius */
  double radialDistortion(double ru) const;
  /** invert radial distortion from (distorted) radius rd and return undistorted radius */
  double radialDistortionInv(double rd) const;
  void radialDistortionInv(GVector::vector2d<double>& pu, const GVector::vector2d<double>& pd) const;
  void radialDistortion(GVector::vector2d<double> pu, GVector::vector2d<double>& pd) const;
  static double radialDistortion(double ru, double dist);
  static void radialDistortion(GVector::vector2d<double> pu, GVector::vector2d<double>& pd, double dist);

  double calc_chisqr(std::vector<GVector::vector3d<double> >& p_f,
                     std::vector<GVector::vector2d<double> >& p_i,
                     Eigen::VectorXd& p,
                     int);
  void field2image(GVector::vector3d<double>& p_f, GVector::vector2d<double>& p_i, Eigen::VectorXd& p) const;

  void toProtoBuffer(SSL_GeometryCameraCalibration& buffer) const;

  enum { FOCAL_LENGTH = 0, PP_X, PP_Y, DIST, Q_1, Q_2, Q_3, T_1, T_2, T_3, STATE_SPACE_DIMENSION };

  enum {
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
  class AdditionalCalibrationInformation {
   public:
    AdditionalCalibrationInformation(int camera_index_, RoboCupField* field);
    ~AdditionalCalibrationInformation();
    void addSettingsToList(VarList& list);
    void updateControlPoints();

    static const int kNumControlPoints = 4;
    VarInt* camera_index;
    VarList* control_point_set[kNumControlPoints]{};
    VarString* control_point_names[kNumControlPoints]{};
    VarDouble* control_point_image_xs[kNumControlPoints]{};
    VarDouble* control_point_image_ys[kNumControlPoints]{};
    VarDouble* control_point_field_xs[kNumControlPoints]{};
    VarDouble* control_point_field_ys[kNumControlPoints]{};

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

    VarInt* imageWidth;
    VarInt* imageHeight;
    VarInt* grid_width;
    VarInt* grid_height;
    VarInt* global_camera_id;

   private:
    RoboCupField* field;
    static std::vector<GVector::vector2d<double> > generateCameraControlPoints(int cameraId,
                                                                               int numCamerasTotal,
                                                                               double fieldHeight,
                                                                               double fieldWidth);
  };

  class CalibrationDataPoint {
   public:
    CalibrationDataPoint(GVector::vector2d<double> img_point, bool detected);
    bool detected;
    GVector::vector2d<double> img_point = {};
    GVector::vector2d<double> img_closestPointToSegment = {};
    GVector::vector3d<double> world_point = {};
    GVector::vector3d<double> world_closestPointToSegment = {};
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
    bool straightLine = true;

    // Parameters for straight line segments
    // Start point of the straight line segment in world space coords
    GVector::vector3d<double> p1;
    // End point of the straight line segment in world space coords
    GVector::vector3d<double> p2;

    // Parameters for arc segments
    // center point of the arc segment in world space coords
    GVector::vector3d<double> center;
    // Start angle (Counter-clockwise, right-handed system) of the arc in world space
    double theta1;
    // End angle (Counter-clockwise, right-handed system) of the arc in world space
    double theta2;
    // Radius of the arc in world space
    double radius;

    // Image points, paired with a bool indicating whether the point was correctly detected
    std::vector<CalibrationDataPoint> points;

    // Denotes the position along the line or arc. For each point, the
    // location of the point is "alpha * start + (1.0 - alpha) * end" where
    // "start" and "end" indicate the start and end points of the line / arc
    // segment. During calibration, this is varied to move the point along the
    // line / arc to minimize the fitting error - see how p_alpha varies.
    std::vector<double> alphas;
  };

 public:
  double do_calibration(int cal_type);
  void reset() const;
  void detectCalibrationCorners();
};

#endif

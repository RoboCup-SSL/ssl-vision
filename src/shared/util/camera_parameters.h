#pragma once

#include <VarTypes.h>

#include <QObject>
#include <opencv2/opencv.hpp>

using namespace VarTypes;

class CameraIntrinsicParameters : public QObject {
  Q_OBJECT

 public:
  CameraIntrinsicParameters();
  ~CameraIntrinsicParameters() override;

  void updateConfigValues() const;
  void reset();

  VarList *settings;

  cv::Mat camera_mat;
  cv::Mat dist_coeffs;

  // derived matrices for faster computation
  cv::Mat camera_mat_inv;

  VarDouble *focal_length_x;
  VarDouble *focal_length_y;
  VarDouble *principal_point_x;
  VarDouble *principal_point_y;

  VarDouble *dist_coeff_k1;
  VarDouble *dist_coeff_k2;
  VarDouble *dist_coeff_p1;
  VarDouble *dist_coeff_p2;
  VarDouble *dist_coeff_k3;
 public slots:
  void updateCameraMat();
  void updateDistCoeffs();
};

class CameraExtrinsicParameters : public QObject {
  Q_OBJECT

 private:
  VarDouble *rvec_x;
  VarDouble *rvec_y;
  VarDouble *rvec_z;

  VarDouble *tvec_x;
  VarDouble *tvec_y;
  VarDouble *tvec_z;

  VarList* calibrationPoints;

 public:
  CameraExtrinsicParameters();
  ~CameraExtrinsicParameters() override;

  void updateConfigValues();
  void reset();

  VarList *settings;

  VarBool *fixFocalLength;
  VarBool *fixPrinciplePoint;
  VarBool *fixTangentialDistortion;
  VarBool *fixK1;
  VarBool *fixK2;
  VarBool *fixK3;

  cv::Mat rvec;
  cv::Mat tvec;

  // derived matrices for faster computation
  cv::Mat rotation_mat_inv;
  cv::Mat right_side_mat;

  void addCalibrationPointSet(cv::Point2d image, cv::Point3d field);
  void clearCalibrationPoints();
  vector<cv::Point2d> getCalibImagePoints();
  vector<cv::Point3d> getCalibFieldPoints();
  int getCalibrationPointSize();
  VarDouble* getCalibImageValueX(int index);
  VarDouble* getCalibImageValueY(int index);

 public slots:
  void updateRVec();
  void updateTVec();
  void addNewCalibrationPointSet();
};

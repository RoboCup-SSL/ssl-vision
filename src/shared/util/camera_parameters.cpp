#include "camera_parameters.h"

#include <VarList.h>

CameraIntrinsicParameters::CameraIntrinsicParameters() {
  settings = new VarList("Intrinsic Parameters");

  focal_length_x = new VarDouble("focal_length_x", 1.0);
  focal_length_y = new VarDouble("focal_length_y", 1.0);
  principal_point_x = new VarDouble("principal_point_x", 0.0);
  principal_point_y = new VarDouble("principal_point_y", 0.0);

  dist_coeff_k1 = new VarDouble("dist_coeff_k1", 0.0);
  dist_coeff_k2 = new VarDouble("dist_coeff_k2", 0.0);
  dist_coeff_p1 = new VarDouble("dist_coeff_p1", 0.0);
  dist_coeff_p2 = new VarDouble("dist_coeff_p2", 0.0);
  dist_coeff_k3 = new VarDouble("dist_coeff_k3", 0.0);

  settings->addChild(focal_length_x);
  settings->addChild(focal_length_y);
  settings->addChild(principal_point_x);
  settings->addChild(principal_point_y);
  settings->addChild(dist_coeff_k1);
  settings->addChild(dist_coeff_k2);
  settings->addChild(dist_coeff_p1);
  settings->addChild(dist_coeff_p2);
  settings->addChild(dist_coeff_k3);

  connect(focal_length_x, SIGNAL(hasChanged(VarType*)), this, SLOT(updateCameraMat()));
  connect(focal_length_y, SIGNAL(hasChanged(VarType*)), this, SLOT(updateCameraMat()));
  connect(principal_point_x, SIGNAL(hasChanged(VarType*)), this, SLOT(updateCameraMat()));
  connect(principal_point_y, SIGNAL(hasChanged(VarType*)), this, SLOT(updateCameraMat()));

  connect(dist_coeff_k1, SIGNAL(hasChanged(VarType*)), this, SLOT(updateDistCoeffs()));
  connect(dist_coeff_k2, SIGNAL(hasChanged(VarType*)), this, SLOT(updateDistCoeffs()));
  connect(dist_coeff_p1, SIGNAL(hasChanged(VarType*)), this, SLOT(updateDistCoeffs()));
  connect(dist_coeff_p2, SIGNAL(hasChanged(VarType*)), this, SLOT(updateDistCoeffs()));
  connect(dist_coeff_k3, SIGNAL(hasChanged(VarType*)), this, SLOT(updateDistCoeffs()));

  camera_mat = cv::Mat::eye(3, 3, CV_64FC1);
  dist_coeffs = cv::Mat(5, 1, CV_64FC1, cv::Scalar(0));
  updateCameraMat();
  updateDistCoeffs();
}

CameraIntrinsicParameters::~CameraIntrinsicParameters() {
  delete focal_length_x;
  delete focal_length_y;
  delete principal_point_x;
  delete principal_point_y;
  delete dist_coeff_k1;
  delete dist_coeff_k2;
  delete dist_coeff_p1;
  delete dist_coeff_p2;
  delete dist_coeff_k3;
  delete settings;
}

void CameraIntrinsicParameters::updateCameraMat() {
  camera_mat.at<double>(0, 0) = focal_length_x->getDouble();
  camera_mat.at<double>(1, 1) = focal_length_y->getDouble();
  camera_mat.at<double>(0, 2) = principal_point_x->getDouble();
  camera_mat.at<double>(1, 2) = principal_point_y->getDouble();

  camera_mat_inv = camera_mat.inv();
}

void CameraIntrinsicParameters::updateDistCoeffs() {
  dist_coeffs.at<double>(0, 0) = dist_coeff_k1->getDouble();
  dist_coeffs.at<double>(1, 0) = dist_coeff_k2->getDouble();
  dist_coeffs.at<double>(2, 0) = dist_coeff_p1->getDouble();
  dist_coeffs.at<double>(3, 0) = dist_coeff_p2->getDouble();
  dist_coeffs.at<double>(4, 0) = dist_coeff_k3->getDouble();
}

void CameraIntrinsicParameters::updateConfigValues() const {
  cv::Mat new_camera_mat(camera_mat);
  focal_length_x->setDouble(new_camera_mat.at<double>(0, 0));
  focal_length_y->setDouble(new_camera_mat.at<double>(1, 1));
  principal_point_x->setDouble(new_camera_mat.at<double>(0, 2));
  principal_point_y->setDouble(new_camera_mat.at<double>(1, 2));

  cv::Mat new_dist_coeffs(dist_coeffs);
  dist_coeff_k1->setDouble(new_dist_coeffs.at<double>(0, 0));
  dist_coeff_k2->setDouble(new_dist_coeffs.at<double>(1, 0));
  dist_coeff_p1->setDouble(new_dist_coeffs.at<double>(2, 0));
  dist_coeff_p2->setDouble(new_dist_coeffs.at<double>(3, 0));
  dist_coeff_k3->setDouble(new_dist_coeffs.at<double>(4, 0));
}

void CameraIntrinsicParameters::reset() {
  focal_length_x->resetToDefault();
  focal_length_y->resetToDefault();
  principal_point_x->resetToDefault();
  principal_point_y->resetToDefault();
  updateCameraMat();

  dist_coeff_k1->resetToDefault();
  dist_coeff_k2->resetToDefault();
  dist_coeff_p1->resetToDefault();
  dist_coeff_p2->resetToDefault();
  dist_coeff_k3->resetToDefault();
  updateDistCoeffs();
}

CameraExtrinsicParameters::CameraExtrinsicParameters() {
  settings = new VarList("Extrinsic Parameters");
  rvec_x = new VarDouble("rvec x", 0.0);
  rvec_y = new VarDouble("rvec y", 0.0);
  rvec_z = new VarDouble("rvec z", 0.0);
  tvec_x = new VarDouble("tvec x", 0.0);
  tvec_y = new VarDouble("tvec y", 0.0);
  tvec_z = new VarDouble("tvec z", 0.0);

  settings->addChild(rvec_x);
  settings->addChild(rvec_y);
  settings->addChild(rvec_z);
  settings->addChild(tvec_x);
  settings->addChild(tvec_y);
  settings->addChild(tvec_z);

  connect(rvec_x, SIGNAL(hasChanged(VarType*)), this, SLOT(updateRVec()));
  connect(rvec_y, SIGNAL(hasChanged(VarType*)), this, SLOT(updateRVec()));
  connect(rvec_z, SIGNAL(hasChanged(VarType*)), this, SLOT(updateRVec()));
  connect(tvec_x, SIGNAL(hasChanged(VarType*)), this, SLOT(updateTVec()));
  connect(tvec_y, SIGNAL(hasChanged(VarType*)), this, SLOT(updateTVec()));
  connect(tvec_z, SIGNAL(hasChanged(VarType*)), this, SLOT(updateTVec()));

  rvec = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
  tvec = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
  rotation_mat_inv = cv::Mat(3, 3, CV_64FC1, cv::Scalar(0));
  right_side_mat = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
}

CameraExtrinsicParameters::~CameraExtrinsicParameters() {
  delete rvec_x;
  delete rvec_y;
  delete rvec_z;
  delete tvec_x;
  delete tvec_y;
  delete tvec_z;
  delete settings;
}

void CameraExtrinsicParameters::updateTVec() {
  tvec.at<double>(0, 0) = tvec_x->getDouble();
  tvec.at<double>(0, 1) = tvec_y->getDouble();
  tvec.at<double>(0, 2) = tvec_z->getDouble();

  right_side_mat = rotation_mat_inv * tvec;
}

void CameraExtrinsicParameters::updateRVec() {
  rvec.at<double>(0, 0) = rvec_x->getDouble();
  rvec.at<double>(0, 1) = rvec_y->getDouble();
  rvec.at<double>(0, 2) = rvec_z->getDouble();

  cv::Mat rotation_mat(3, 3, cv::DataType<double>::type);
  cv::Rodrigues(rvec, rotation_mat);
  rotation_mat_inv = rotation_mat.inv();
  right_side_mat = rotation_mat_inv * tvec;
}

void CameraExtrinsicParameters::updateConfigValues() {
  cv::Mat new_tvec = tvec.clone();
  tvec_x->setDouble(new_tvec.at<double>(0, 0));
  tvec_y->setDouble(new_tvec.at<double>(0, 1));
  tvec_z->setDouble(new_tvec.at<double>(0, 2));

  cv::Mat new_rvec = rvec.clone();
  rvec_x->setDouble(new_rvec.at<double>(0, 0));
  rvec_y->setDouble(new_rvec.at<double>(0, 1));
  rvec_z->setDouble(new_rvec.at<double>(0, 2));
}

void CameraExtrinsicParameters::reset() {
  tvec_x->resetToDefault();
  tvec_y->resetToDefault();
  tvec_z->resetToDefault();
  updateTVec();

  rvec_x->resetToDefault();
  rvec_y->resetToDefault();
  rvec_z->resetToDefault();
  updateRVec();
}

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
  cv::Mat tmp_camera_mat = cv::Mat::eye(3, 3, CV_64F);
  tmp_camera_mat.at<double>(0, 0) = focal_length_x->getDouble();
  tmp_camera_mat.at<double>(1, 1) = focal_length_y->getDouble();
  tmp_camera_mat.at<double>(0, 2) = principal_point_x->getDouble();
  tmp_camera_mat.at<double>(1, 2) = principal_point_y->getDouble();
  tmp_camera_mat.copyTo(camera_mat);

  camera_mat_inv = camera_mat.inv();
}

void CameraIntrinsicParameters::updateDistCoeffs() {
  cv::Mat tmp_dist_coeffs(5, 1, CV_64F);
  tmp_dist_coeffs.at<double>(0, 0) = dist_coeff_k1->getDouble();
  tmp_dist_coeffs.at<double>(1, 0) = dist_coeff_k2->getDouble();
  tmp_dist_coeffs.at<double>(2, 0) = dist_coeff_p1->getDouble();
  tmp_dist_coeffs.at<double>(3, 0) = dist_coeff_p2->getDouble();
  tmp_dist_coeffs.at<double>(4, 0) = dist_coeff_k3->getDouble();
  tmp_dist_coeffs.copyTo(dist_coeffs);
}

void CameraIntrinsicParameters::updateConfigValues() const {
  cv::Mat tmp_camera_mat(3, 3, CV_64F);
  camera_mat.copyTo(tmp_camera_mat);
  focal_length_x->setDouble(tmp_camera_mat.at<double>(0, 0));
  focal_length_y->setDouble(tmp_camera_mat.at<double>(1, 1));
  principal_point_x->setDouble(tmp_camera_mat.at<double>(0, 2));
  principal_point_y->setDouble(tmp_camera_mat.at<double>(1, 2));

  cv::Mat tmp_dist_coeffs(5, 1, CV_64F);
  dist_coeffs.copyTo(tmp_dist_coeffs);
  dist_coeff_k1->setDouble(tmp_dist_coeffs.at<double>(0));
  dist_coeff_k2->setDouble(tmp_dist_coeffs.at<double>(1));
  dist_coeff_p1->setDouble(tmp_dist_coeffs.at<double>(2));
  dist_coeff_p2->setDouble(tmp_dist_coeffs.at<double>(3));
  dist_coeff_k3->setDouble(tmp_dist_coeffs.at<double>(4));
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

  fixFocalLength = new VarBool("Fix focal length", false);
  fixPrinciplePoint = new VarBool("Fix principle point", false);
  fixTangentialDistortion = new VarBool("Fix tangential distortion", true);
  fixK1 = new VarBool("Fix k1", false);
  fixK2 = new VarBool("Fix k2", false);
  fixK3 = new VarBool("Fix k3", false);

  calibrationPoints = new VarList("Calibration points");
  auto addPoint = new VarTrigger("Calibration point", "Add");

  settings->addChild(rvec_x);
  settings->addChild(rvec_y);
  settings->addChild(rvec_z);
  settings->addChild(tvec_x);
  settings->addChild(tvec_y);
  settings->addChild(tvec_z);
  settings->addChild(fixFocalLength);
  settings->addChild(fixPrinciplePoint);
  settings->addChild(fixTangentialDistortion);
  settings->addChild(fixK1);
  settings->addChild(fixK2);
  settings->addChild(fixK3);
  settings->addChild(addPoint);
  settings->addChild(calibrationPoints);

  connect(rvec_x, SIGNAL(hasChanged(VarType*)), this, SLOT(updateRVec()));
  connect(rvec_y, SIGNAL(hasChanged(VarType*)), this, SLOT(updateRVec()));
  connect(rvec_z, SIGNAL(hasChanged(VarType*)), this, SLOT(updateRVec()));
  connect(tvec_x, SIGNAL(hasChanged(VarType*)), this, SLOT(updateTVec()));
  connect(tvec_y, SIGNAL(hasChanged(VarType*)), this, SLOT(updateTVec()));
  connect(tvec_z, SIGNAL(hasChanged(VarType*)), this, SLOT(updateTVec()));
  connect(addPoint, SIGNAL(wasEdited(VarType*)), this, SLOT(addNewCalibrationPointSet()));

  rvec = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
  tvec = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
  rotation_mat_inv = cv::Mat(3, 3, CV_64FC1, cv::Scalar(0));
  right_side_mat = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
}

CameraExtrinsicParameters::~CameraExtrinsicParameters() {
  settings->deleteAllChildren();
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

  clearCalibrationPoints();
}

void CameraExtrinsicParameters::addNewCalibrationPointSet() {
  cv::Point2d image(50, 50);
  cv::Point3d field(0, 0, 0);
  addCalibrationPointSet(image, field);
}

void CameraExtrinsicParameters::addCalibrationPointSet(cv::Point2d image, cv::Point3d field) {
  auto set = new VarList("point");
  set->addChild(new VarDouble("image_x", image.x));
  set->addChild(new VarDouble("image_y", image.y));
  set->addChild(new VarDouble("field_x", field.x));
  set->addChild(new VarDouble("field_y", field.y));
  set->addChild(new VarDouble("field_z", field.z));
  calibrationPoints->addChild(set);
}

void CameraExtrinsicParameters::clearCalibrationPoints() {
  settings->removeChild(calibrationPoints);
  // Note: The old points should be deleted, but deleting them here causes a seg fault due to some
  // update handlers that are called on the var types afterwards...
  // So for now, they are not deleted at all
//  calibrationPoints->deleteAllChildren();
  calibrationPoints = new VarList("Calibration points");
  settings->addChild(calibrationPoints);
}

std::vector<cv::Point2d> CameraExtrinsicParameters::getCalibImagePoints() {
  std::vector<cv::Point2d> points;
  for (auto& pointSet : calibrationPoints->getChildren()) {
    auto image_x = (VarDouble*)pointSet->findChild("image_x");
    auto image_y = (VarDouble*)pointSet->findChild("image_y");
    points.emplace_back(image_x->getDouble(), image_y->getDouble());
  }
  return points;
}

std::vector<cv::Point3d> CameraExtrinsicParameters::getCalibFieldPoints() {
  std::vector<cv::Point3d> points;
  for (auto& pointSet : calibrationPoints->getChildren()) {
    auto field_x = (VarDouble*)pointSet->findChild("field_x");
    auto field_y = (VarDouble*)pointSet->findChild("field_y");
    auto field_z = (VarDouble*)pointSet->findChild("field_z");
    points.emplace_back(field_x->getDouble(), field_y->getDouble(), field_z->getDouble());
  }
  return points;
}

int CameraExtrinsicParameters::getCalibrationPointSize() {
  return calibrationPoints->getChildrenCount();
}

VarDouble* CameraExtrinsicParameters::getCalibImageValueX(int index) {
  return (VarDouble*)calibrationPoints->getChildren()[index]->findChild("image_x");
}

VarDouble* CameraExtrinsicParameters::getCalibImageValueY(int index) {
  return (VarDouble*)calibrationPoints->getChildren()[index]->findChild("image_y");
}

#include "camera_calibration.h"
#include <Eigen/Cholesky>
#include <iostream>
#include <algorithm>
#include <limits>
#include "field.h"
#include "field_default_constants.h"
#include "geomalgo.h"

CameraParameters::CameraParameters(int camera_index_, RoboCupField * field_) :
        p_alpha(Eigen::VectorXd(1)) {
  focal_length = new VarDouble("focal length", 500.0);
  principal_point_x = new VarDouble("principal point x", 390.0);
  principal_point_y = new VarDouble("principal point y", 290.0);
  distortion = new VarDouble("distortion", 0.0, -1.0, 2.0);
  //do not overwrite min/max ranges with values from config file
  distortion->addFlags(VARTYPE_FLAG_NOLOAD_ATTRIBUTES);

  q0 = new VarDouble("q0", 0.7);
  q1 = new VarDouble("q1", -0.7);
  q2 = new VarDouble("q2", .0);
  q3 = new VarDouble("q3", .0);

  tx = new VarDouble("tx", 0);
  ty = new VarDouble("ty", 1250);

  tz = new VarDouble("tz", 3500, 0, 6500);
  //do not overwrite min/max ranges with values from config file
  tz->addFlags(VARTYPE_FLAG_NOLOAD_ATTRIBUTES);

  additional_calibration_information =
      new AdditionalCalibrationInformation(camera_index_, field_);

  q_rotate180 = Quaternion<double>(0, 0, 1.0,0);

  intrinsic_parameters = new CameraIntrinsicParameters();
  extrinsic_parameters = new CameraExtrinsicParameters();
  use_opencv_model = new VarBool("use openCV camera model", false);
}

CameraParameters::~CameraParameters() {
  delete focal_length;
  delete principal_point_x;
  delete principal_point_y;
  delete distortion;
  delete q0;
  delete q1;
  delete q2;
  delete q3;
  delete tx;
  delete ty;
  delete tz;
  delete additional_calibration_information;
}

/**
 * Convert the rotational vector from extrinsic parameters to a Quaternion.
 * Taken from: https://gist.github.com/shubh-agrawal/76754b9bfb0f4143819dbd146d15d4c8
 */
void CameraParameters::quaternionFromOpenCVCalibration(double Q[]) const
{
  cv::Mat R;
  cv::Rodrigues(extrinsic_parameters->rvec, R);
  double trace = R.at<double>(0,0) + R.at<double>(1,1) + R.at<double>(2,2);

  if (trace > 0.0)
  {
    double s = sqrt(trace + 1.0);
    Q[3] = (s * 0.5);
    s = 0.5 / s;
    Q[0] = ((R.at<double>(2,1) - R.at<double>(1,2)) * s);
    Q[1] = ((R.at<double>(0,2) - R.at<double>(2,0)) * s);
    Q[2] = ((R.at<double>(1,0) - R.at<double>(0,1)) * s);
  }
  else
  {
    int i = R.at<double>(0,0) < R.at<double>(1,1) ? (R.at<double>(1,1) < R.at<double>(2,2) ? 2 : 1) : (R.at<double>(0,0) < R.at<double>(2,2) ? 2 : 0);
    int j = (i + 1) % 3;
    int k = (i + 2) % 3;

    double s = sqrt(R.at<double>(i, i) - R.at<double>(j,j) - R.at<double>(k,k) + 1.0);
    Q[i] = s * 0.5;
    s = 0.5 / s;

    Q[3] = (R.at<double>(k,j) - R.at<double>(j,k)) * s;
    Q[j] = (R.at<double>(j,i) + R.at<double>(i,j)) * s;
    Q[k] = (R.at<double>(k,i) + R.at<double>(i,k)) * s;
  }
}

void CameraParameters::toProtoBuffer(SSL_GeometryCameraCalibration &buffer) const {
  buffer.set_camera_id(additional_calibration_information->camera_index->getInt());
  if (use_opencv_model->getBool()) {
    buffer.set_focal_length((float) (intrinsic_parameters->focal_length_x->getDouble() + intrinsic_parameters->focal_length_y->getDouble()) / 2.0f);
    buffer.set_principal_point_x((float) intrinsic_parameters->principal_point_x->getDouble());
    buffer.set_principal_point_y((float) intrinsic_parameters->principal_point_y->getDouble());
    buffer.set_distortion((float) intrinsic_parameters->dist_coeff_k1->getDouble());
    double Q[4];
    quaternionFromOpenCVCalibration(Q);
    buffer.set_q0((float) Q[0]);
    buffer.set_q1((float) Q[1]);
    buffer.set_q2((float) Q[2]);
    buffer.set_q3((float) Q[3]);
    buffer.set_tx((float) extrinsic_parameters->tvec.at<double>(0, 0));
    buffer.set_ty((float) extrinsic_parameters->tvec.at<double>(0, 1));
    buffer.set_tz((float) extrinsic_parameters->tvec.at<double>(0, 2));
  } else {
    buffer.set_focal_length((float) focal_length->getDouble());
    buffer.set_principal_point_x((float) principal_point_x->getDouble());
    buffer.set_principal_point_y((float) principal_point_y->getDouble());
    buffer.set_distortion((float) distortion->getDouble());
    buffer.set_q0((float) q0->getDouble());
    buffer.set_q1((float) q1->getDouble());
    buffer.set_q2((float) q2->getDouble());
    buffer.set_q3((float) q3->getDouble());
    buffer.set_tx((float) tx->getDouble());
    buffer.set_ty((float) ty->getDouble());
    buffer.set_tz((float) tz->getDouble());
  }

  GVector::vector3d< double > world = getWorldLocation();
  buffer.set_derived_camera_world_tx((float) world.x);
  buffer.set_derived_camera_world_ty((float) world.y);
  buffer.set_derived_camera_world_tz((float) world.z);

  buffer.set_pixel_image_width(additional_calibration_information->imageWidth->getInt());
  buffer.set_pixel_image_height(additional_calibration_information->imageHeight->getInt());
}

GVector::vector3d< double > CameraParameters::getWorldLocation() const {
  if (use_opencv_model->getBool()) {
    cv::Mat R;
    cv::Rodrigues(extrinsic_parameters->rvec, R);
    R = R.t();
    cv::Mat t = -R * extrinsic_parameters->tvec;
    GVector::vector3d<double> v_out;
    v_out.x = t.at<double>(0, 0);
    v_out.y = t.at<double>(0, 1);
    v_out.z = t.at<double>(0, 2);
    return v_out;
  }
  Quaternion<double> q;
  q.set(q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
  q.invert();
  GVector::vector3d<double> v_in(tx->getDouble(),ty->getDouble(),tz->getDouble());
  v_in = (-(v_in));
  GVector::vector3d<double> v_out = q.rotateVectorByQuaternion(v_in);
  return v_out;
}

void CameraParameters::addSettingsToList(VarList& list) const {
  list.addChild(intrinsic_parameters->settings);
  list.addChild(extrinsic_parameters->settings);
  list.addChild(use_opencv_model);
  list.addChild(focal_length);
  list.addChild(principal_point_x);
  list.addChild(principal_point_y);
  list.addChild(distortion);
  list.addChild(q0);
  list.addChild(q1);
  list.addChild(q2);
  list.addChild(q3);
  list.addChild(tx);
  list.addChild(ty);
  list.addChild(tz);
}

double CameraParameters::radialDistortion(double ru) const {
  if ((distortion->getDouble())<=DBL_MIN)
    return ru;
  double rd = 0;
  double a = distortion->getDouble();
  double b = -9.0*a*a*ru + a*sqrt(a*(12.0 + 81.0*a*ru*ru));
  b = (b < 0.0) ? (-pow(b, 1.0 / 3.0)) : pow(b, 1.0 / 3.0);
  rd = pow(2.0 / 3.0, 1.0 / 3.0) / b -
      b / (pow(2.0 * 3.0 * 3.0, 1.0 / 3.0) * a);
  return rd;
}

double CameraParameters::radialDistortion(double ru, double dist) {
  if (dist<=DBL_MIN)
    return ru;
  double rd = 0;
  double a = dist;
  double b = -9.0*a*a*ru + a*sqrt(a*(12.0 + 81.0*a*ru*ru));
  b = (b < 0.0) ? (-pow(b, 1.0 / 3.0)) : pow(b, 1.0 / 3.0);
  rd = pow(2.0 / 3.0, 1.0 / 3.0) / b -
      b / (pow(2.0 * 3.0 * 3.0, 1.0 / 3.0) * a);
  return rd;
}

double CameraParameters::radialDistortionInv(double rd) const {
  double ru = rd*(1.0+rd*rd*distortion->getDouble());
  return ru;
}

void CameraParameters::radialDistortionInv(
    GVector::vector2d<double> &pu, const GVector::vector2d<double> &pd) const {
  double ru = radialDistortionInv(pd.length());
  pu = pd;
  pu = pu.norm(ru);
}

void CameraParameters::radialDistortion(
    const GVector::vector2d<double> pu, GVector::vector2d<double> &pd) const {
  double rd = radialDistortion(pu.length());
  pd = pu;
  pd = pd.norm(rd);
}

void CameraParameters::radialDistortion(
    const GVector::vector2d<double> pu,
    GVector::vector2d<double> &pd, double dist) {
  double rd = radialDistortion(pu.length(), dist);
  pd = pu;
  pd = pd.norm(rd);
}

void CameraParameters::field2image(
    const GVector::vector3d<double> &p_f,
    GVector::vector2d<double> &p_i) const {

  if(use_opencv_model->getBool()) {
    cv::Mat src(1, 3, CV_64FC1);
    cv::Mat dst(1, 2, CV_64FC1);
    src.ptr<cv::Point3d>(0)->x = p_f.x;
    src.ptr<cv::Point3d>(0)->y = p_f.y;
    src.ptr<cv::Point3d>(0)->z = p_f.z;
    cv::projectPoints(src, extrinsic_parameters->rvec, extrinsic_parameters->tvec, intrinsic_parameters->camera_mat, intrinsic_parameters->dist_coeffs, dst);
    p_i.x = dst.at<double>(0);
    p_i.y = dst.at<double>(1);
  } else {
    Quaternion<double> q_field2cam = Quaternion<double>(
        q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
    q_field2cam.norm();
    GVector::vector3d<double> translation = GVector::vector3d<double>(
        tx->getDouble(),ty->getDouble(),tz->getDouble());

    // First transform the point from the field into the coordinate system of the
    // camera
    GVector::vector3d<double> p_c =
        q_field2cam.rotateVectorByQuaternion(p_f) + translation;
    GVector::vector2d<double> p_un =
        GVector::vector2d<double>(p_c.x/p_c.z, p_c.y/p_c.z);

    // Apply distortion
    GVector::vector2d<double> p_d;
    radialDistortion(p_un,p_d);

    // Then project from the camera coordinate system onto the image plane using
    // the instrinsic parameters
    p_i = focal_length->getDouble() * p_d +
          GVector::vector2d<double>(principal_point_x->getDouble(),
                                    principal_point_y->getDouble());
  }
}

void CameraParameters::field2image(
    GVector::vector3d<double> &p_f, GVector::vector2d<double> &p_i,
    Eigen::VectorXd &p) const {
  Quaternion<double> q_field2cam = Quaternion<double>(
      q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
  q_field2cam.norm();
  GVector::vector3d<double> translation = GVector::vector3d<double>(
      tx->getDouble(),ty->getDouble(),tz->getDouble());

  GVector::vector3d<double> aa_diff(p[Q_1], p[Q_2], p[Q_3]);
  GVector::vector3d<double> t_diff(p[T_1], p[T_2], p[T_3]);

  // Create a quaternion out of the 3D rotational representation
  Quaternion<double> q_diff;
  q_diff.setAxis(aa_diff.norm(), aa_diff.length());

  // First transform the point from the field into the coordinate system of the
  // camera
  GVector::vector3d<double> p_c =
      (q_diff * q_field2cam ).rotateVectorByQuaternion(p_f) + translation +
      t_diff;
  GVector::vector2d<double> p_un =
      GVector::vector2d<double>(p_c.x/p_c.z, p_c.y/p_c.z);

  // Apply distortion
  GVector::vector2d<double> p_d;
  radialDistortion(p_un,p_d,distortion->getDouble() + p[DIST]);

  // Then project from the camera coordinate system onto the image plane using
  // the instrinsic parameters
  p_i = (focal_length->getDouble() + p[FOCAL_LENGTH]) * p_d +
      GVector::vector2d<double>(principal_point_x->getDouble() + p[PP_X],
                                principal_point_y->getDouble() + p[PP_Y]);
}

void CameraParameters::image2field(
    GVector::vector3d<double> &p_f, const GVector::vector2d<double> &p_i,
    double z) const {
  if(use_opencv_model->getBool()) {
    cv::Mat p_i_mat(3, 1, CV_64FC1);

    p_i_mat.ptr<cv::Point3d>(0)->x = p_i.x;
    p_i_mat.ptr<cv::Point3d>(0)->y = p_i.y;
    p_i_mat.ptr<cv::Point3d>(0)->z = 1;

    /**
     * Calculation is based on:
     * https://stackoverflow.com/questions/12299870/computing-x-y-coordinate-3d-from-image-point
     */

    const cv::Mat &tvec = extrinsic_parameters->tvec;
    const cv::Mat &rotation_mat_inv = extrinsic_parameters->rotation_mat_inv;
    const cv::Mat &camera_mat_inv = intrinsic_parameters->camera_mat_inv;
    const cv::Mat &right_side_mat = extrinsic_parameters->right_side_mat;
    const cv::Mat left_side_mat = rotation_mat_inv * camera_mat_inv * p_i_mat;

    double right_side_mat_z = right_side_mat.at<double>(2, 0);
    double left_side_mat_z = left_side_mat.at<double>(2, 0);
    double s = (z + right_side_mat_z) / left_side_mat_z;

    cv::Mat p_f_mat = rotation_mat_inv * (s * camera_mat_inv * p_i_mat - tvec);

    p_f.x = p_f_mat.at<cv::Point3d>(0).x;
    p_f.y = p_f_mat.at<cv::Point3d>(0).y;
    p_f.z = p_f_mat.at<cv::Point3d>(0).z;
  } else {
    // Undo scaling and offset
    GVector::vector2d<double> p_d(
        (p_i.x - principal_point_x->getDouble()) / focal_length->getDouble(),
        (p_i.y - principal_point_y->getDouble()) / focal_length->getDouble());

    // Compensate for distortion (undistort)
    GVector::vector2d<double> p_un;
    radialDistortionInv(p_un, p_d);

    // Now we got a ray on the z axis
    GVector::vector3d<double> v(p_un.x, p_un.y, 1);

    // Transform this ray into world coordinates
    Quaternion<double> q_field2cam = Quaternion<double>(
        q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
    q_field2cam.norm();
    GVector::vector3d<double> translation =
        GVector::vector3d<double>(tx->getDouble(),ty->getDouble(),tz->getDouble());

    Quaternion<double> q_field2cam_inv = q_field2cam;
    q_field2cam_inv.invert();
    GVector::vector3d<double> v_in_w =
        q_field2cam_inv.rotateVectorByQuaternion(v);
    GVector::vector3d<double> zero_in_w =
        q_field2cam_inv.rotateVectorByQuaternion(
            GVector::vector3d<double>(0,0,0) - translation);

    // Compute the the point where the rays intersects the field
    double t = GVector::ray_plane_intersect(
        GVector::vector3d<double>(0,0,z), GVector::vector3d<double>(0,0,1).norm(),
        zero_in_w, v_in_w.norm());

    // Set p_f
    p_f = zero_in_w + v_in_w.norm() * t;
  }
}


double CameraParameters::calc_chisqr(
    std::vector<GVector::vector3d<double> > &p_f,
    std::vector<GVector::vector2d<double> > &p_i, Eigen::VectorXd &p,
    int cal_type) {
  assert(p_f.size() == p_i.size());

  double cov_cx_inv =
      1.0 / additional_calibration_information->cov_corner_x->getDouble();
  double cov_cy_inv =
      1.0 / additional_calibration_information->cov_corner_y->getDouble();

  double cov_lsx_inv =
      1.0 / additional_calibration_information->cov_ls_x->getDouble();
  double cov_lsy_inv =
      1.0 / additional_calibration_information->cov_ls_y->getDouble();

  double chisqr(0);

  // Iterate over manual points
  auto it_p_f  = p_f.begin();
  auto it_p_i  = p_i.begin();

  for (; it_p_f != p_f.end(); it_p_f++, it_p_i++)
  {
    GVector::vector2d<double> proj_p;
    field2image(*it_p_f, proj_p, p);
    chisqr += (proj_p.x - it_p_i->x) * (proj_p.x - it_p_i->x) * cov_cx_inv +
        (proj_p.y - it_p_i->y) * (proj_p.y - it_p_i->y) * cov_cy_inv;
  }

  // Iterate of line edge points, but only when performing a full estimation
  if (cal_type & FULL_ESTIMATION)
  {
    auto ls_it = calibrationSegments.begin();

    int i = 0;
    for (; ls_it != calibrationSegments.end(); ls_it++) {
      auto imgPts_it = (*ls_it).points.begin();
      for (; imgPts_it != (*ls_it).points.end(); imgPts_it++) {
        // Integrate only if a valid point on line
        if (imgPts_it->detected) {
          GVector::vector2d<double> proj_p;
          double alpha = p_alpha(i) + p(STATE_SPACE_DIMENSION + i);

          // Calculate point on segment
          GVector::vector3d<double> alpha_point;
          if (ls_it->straightLine) {
            alpha_point = alpha * (*ls_it).p1 + (1.0 - alpha) * (*ls_it).p2;
          } else {
            double theta = alpha * (*ls_it).theta1 + (1.0 - alpha) * (*ls_it).theta2;
            alpha_point = ls_it->center +
                ls_it->radius*GVector::vector3d<double>(
                    cos(theta),sin(theta),0.0);
          }

          // Project into image plane
          field2image(alpha_point, proj_p, p);

          chisqr += (proj_p.x-imgPts_it->img_point.x) *
              (proj_p.x-imgPts_it->img_point.x) * cov_lsx_inv +
              (proj_p.y - imgPts_it->img_point.y) *
              (proj_p.y - imgPts_it->img_point.y) * cov_lsy_inv;
          i++;
        }
      }
    }
  }
  return chisqr;
}

double CameraParameters::do_calibration(int cal_type) {
  std::vector<GVector::vector3d<double> > p_f;
  std::vector<GVector::vector2d<double> > p_i;

  AdditionalCalibrationInformation* aci = additional_calibration_information;
  for (int i = 0; i < AdditionalCalibrationInformation::kNumControlPoints;
      ++i) {
    p_i.emplace_back(
      aci->control_point_image_xs[i]->getDouble(),
      aci->control_point_image_ys[i]->getDouble());
    p_f.emplace_back(
      aci->control_point_field_xs[i]->getDouble(),
      aci->control_point_field_ys[i]->getDouble(), 0.0);
  }

  if(use_opencv_model->getBool()) {
    calibrateExtrinsicModel(p_f, p_i, cal_type);
  } else {
    calibrate(p_f, p_i, cal_type);
  }

  updateCalibrationDataPoints();
  if (cal_type & FOUR_POINT_INITIAL) {
    return calculateFourPointRmse(p_f, p_i);
  }
  return calculateCalibrationDataPointsRmse();
}

double CameraParameters::calculateFourPointRmse(std::vector<GVector::vector3d<double> > &p_f,
                                                std::vector<GVector::vector2d<double> > &p_i) const {
  auto it_p_f  = p_f.begin();
  auto it_p_i  = p_i.begin();
  double sum = 0;
  for (; it_p_f != p_f.end(); it_p_f++, it_p_i++) {
    GVector::vector2d<double> proj_p;
    field2image(*it_p_f, proj_p);
    GVector::vector3d<double> some_point;
    image2field(some_point,proj_p, 0);

    double diff_x = proj_p.x - it_p_i->x;
    double diff_y = proj_p.y - it_p_i->y;
    sum += diff_x * diff_x + diff_y * diff_y;
  }
  return sqrt(sum / (double) p_f.size());
}

void CameraParameters::updateCalibrationDataPoints() {
  for (auto & segment : calibrationSegments) {
    if (!segment.straightLine) {
      continue;
    }

    for (auto & imgPt : segment.points) {
      if (imgPt.detected) {
        image2field(imgPt.world_point, imgPt.img_point, 0.0);

        GVector::vector2d<double> line0(segment.p1.x, segment.p1.y);
        GVector::vector2d<double> line1(segment.p2.x, segment.p2.y);
        GVector::vector2d<double> point(imgPt.world_point.x, imgPt.world_point.y);
        GVector::vector2d<double> closestPoint = GVector::closest_point_on_line(line0, line1, point);
        imgPt.world_closestPointToSegment.x = closestPoint.x;
        imgPt.world_closestPointToSegment.y = closestPoint.y;
        field2image(imgPt.world_closestPointToSegment, imgPt.img_closestPointToSegment);
      }
    }
  }
}

double CameraParameters::calculateCalibrationDataPointsRmse() {
  double rmse_sum_all = 0;
  int rmse_cnt_all = 0;
  for (auto & segment : calibrationSegments) {
    if (!segment.straightLine) {
      continue;
    }

    double rmse_sum_segment = 0;
    int rmse_cnt_segment = 0;

    for (auto imgPt : segment.points) {
      if (imgPt.detected) {
        double distance = (imgPt.img_closestPointToSegment - imgPt.img_point).length();
        rmse_sum_segment += distance*distance;
        rmse_cnt_segment++;
      }
    }
    std::cout << "segment ["
              << segment.p1.x << "," << segment.p1.y
              << "] -> ["
              << segment.p2.x << "," << segment.p2.y
              << "] RMSE: " << sqrt(rmse_sum_segment / rmse_cnt_segment)
              << std::endl;
    rmse_sum_all += rmse_sum_segment;
    rmse_cnt_all += rmse_cnt_segment;
  }
  double rmse = sqrt(rmse_sum_all / rmse_cnt_all);
  std::cout << "Calibration segment RMSE: " << rmse << std::endl;
  return rmse;
}

CameraParameters::CalibrationDataPoint::CalibrationDataPoint(GVector::vector2d<double> img_point, bool detected)
    : detected(detected),
      img_point(img_point),
      img_closestPointToSegment(0,0),
      world_point(0,0,0),
      world_closestPointToSegment(0,0,0)
{}

void CameraParameters::reset() const {
  focal_length->resetToDefault();
  principal_point_x->resetToDefault();
  principal_point_y->resetToDefault();
  distortion->resetToDefault();
  tx->resetToDefault();
  ty->resetToDefault();
  // note: Do not reset camera height, as this is set manually by user
  q0->resetToDefault();
  q1->resetToDefault();
  q2->resetToDefault();
  q3->resetToDefault();
}

bool intersection(cv::Vec4d l1, cv::Vec4d l2, cv::Point2d &r)
{
  cv::Point2d o1;
  cv::Point2d x;
  cv::Point2d d1;
  cv::Point2d d2;

  o1.x = l1[2];
  o1.y = l1[3];
  x.x = l2[2] - l1[2];
  x.y = l2[3] - l1[3];
  d1.x = l1[0];
  d1.y = l1[1];
  d2.x = l2[0];
  d2.y = l2[1];

  double cross = d1.x*d2.y - d1.y*d2.x;
  if (abs(cross) < 1e-8)
    return false;

  double t1 = (x.x * d2.y - x.y * d2.x)/cross;
  r = o1 + d1 * t1;
  return true;
}

bool contains(const cv::Rect& rect, const cv::Point2d& pt, double margin) {
  return rect.x - margin <= pt.x && pt.x < rect.x + rect.width + margin * 2
         && rect.y - margin <= pt.y && pt.y < rect.y + rect.height + margin * 2;
}

void CameraParameters::calibrateExtrinsicModel(
    std::vector<GVector::vector3d<double>> &p_f,
    std::vector<GVector::vector2d<double>> &p_i,
    int cal_type) const {

  std::vector<cv::Point3d> calib_field_points = extrinsic_parameters->getCalibFieldPoints();
  std::vector<cv::Point2d> calib_image_points = extrinsic_parameters->getCalibImagePoints();

  if (cal_type & FULL_ESTIMATION && calib_field_points.size() < 4) {
    std::cerr << "Not enough calibration points for full estimation: " << calib_field_points.size() << std::endl;
  }

  if (cal_type & FOUR_POINT_INITIAL || calib_field_points.size() < 4) {
    // Use calibration points
    for (uint i = 0; i < p_f.size(); i++) {
      calib_field_points.emplace_back(p_f[i].x, p_f[i].y, p_f[i].z);
      calib_image_points.emplace_back(p_i[i].x, p_i[i].y);
    }
  }

  std::vector<std::vector<cv::Point3f>> object_points(1);
  std::vector<std::vector<cv::Point2f>> image_points(1);
  cv::Size imageSize(
      additional_calibration_information->imageWidth->getInt(),
      additional_calibration_information->imageHeight->getInt());
  std::vector<cv::Mat> rvecs;
  std::vector<cv::Mat> tvecs;

  for (auto &point : calib_field_points) {
    object_points[0].emplace_back((float) point.x, (float) point.y, (float) point.z);
  }
  for (auto &point : calib_image_points) {
    image_points[0].emplace_back((float) point.x, (float) point.y);
  }

  int flags = cv::CALIB_USE_INTRINSIC_GUESS;
  if (cal_type & FULL_ESTIMATION) {
    if (extrinsic_parameters->fixFocalLength->getBool()) {
      flags |= cv::CALIB_FIX_FOCAL_LENGTH;
    }
    if (extrinsic_parameters->fixPrinciplePoint->getBool()) {
      flags |= cv::CALIB_FIX_PRINCIPAL_POINT;
    }
    if (extrinsic_parameters->fixTangentialDistortion->getBool()) {
      flags |=cv::CALIB_FIX_TANGENT_DIST;
    }
    if (extrinsic_parameters->fixK1->getBool()) {
      flags |=cv::CALIB_FIX_K1;
    }
    if (extrinsic_parameters->fixK2->getBool()) {
      flags |=cv::CALIB_FIX_K2;
    }
    if (extrinsic_parameters->fixK3->getBool()) {
      flags |=cv::CALIB_FIX_K3;
    }
  } else if (cal_type & FOUR_POINT_INITIAL) {
    flags |= cv::CALIB_FIX_TANGENT_DIST
             | cv::CALIB_FIX_K1
             | cv::CALIB_FIX_K2
             | cv::CALIB_FIX_K3
             | cv::CALIB_FIX_FOCAL_LENGTH
             | cv::CALIB_FIX_PRINCIPAL_POINT;
  }

  double rms = cv::calibrateCamera(
        object_points,
        image_points,
        imageSize,
        intrinsic_parameters->camera_mat,
        intrinsic_parameters->dist_coeffs,
        rvecs,
        tvecs,
        flags,
        cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, DBL_EPSILON)
      );
  extrinsic_parameters->rvec = rvecs[0];
  extrinsic_parameters->tvec = tvecs[0];

  std::cout << "Calibration finished with RMS=" << rms << std::endl;

  // Update parameters
  intrinsic_parameters->updateConfigValues();
  extrinsic_parameters->updateConfigValues();
}

void CameraParameters::detectCalibrationCorners() {

  extrinsic_parameters->clearCalibrationPoints();

  // fitted lines through undistorted calibration points
  vector<cv::Vec4d> lines_img_undistorted;
  // corresponding lines in field coordinates
  vector<cv::Vec4d> lines_field;
  // corresponding line starting points
  vector<cv::Point2d> lines_start;
  // corresponding line ending points
  vector<cv::Point2d> lines_end;

  for (const auto &calibrationData : calibrationSegments) {
    if (!calibrationData.straightLine) {
      // only straight lines are supported
      continue;
    }

    // collect all valid image points
    vector<cv::Point2d> points_img;
    for (auto point : calibrationData.points) {
      if (point.detected) {
        auto p_img = point.img_point;
        points_img.emplace_back(p_img.x, p_img.y);
      }
    }

    if (points_img.empty()) {
      continue;
    }

    // undistort image points (without considering extrinsic calibration)
    vector<cv::Point2d> points_img_normalized;
    cv::undistortPoints(
        points_img, points_img_normalized, intrinsic_parameters->camera_mat, intrinsic_parameters->dist_coeffs);

    // fit a line through the detected calibration image points
    cv::Vec4d line_img;
    cv::fitLine(points_img_normalized, line_img, cv::DIST_L2, 0, 0.01, 0.01);
    lines_img_undistorted.push_back(line_img);
    // create a corresponding line in world coordinates
    auto dir = (calibrationData.p2 - calibrationData.p1).norm();
    cv::Vec4d line_field(dir.x, dir.y, calibrationData.p1.x, calibrationData.p1.y);
    lines_field.push_back(line_field);
    // add corresponding start and end points
    lines_start.emplace_back(calibrationData.p1.x, calibrationData.p1.y);
    lines_end.emplace_back(calibrationData.p2.x, calibrationData.p2.y);
  }

  vector<cv::Point3d> points_intersection_img_undistorted;
  vector<cv::Point3d> points_intersection_field;
  for (uint i = 0; i < lines_field.size(); i++) {
    for (uint j = i + 1; j < lines_field.size(); j++) {
      auto line_img1 = lines_img_undistorted[i];
      auto line_img2 = lines_img_undistorted[j];
      auto line_field1 = lines_field[i];
      auto line_field2 = lines_field[j];

      cv::Point2d p_intersection_img_undistorted;
      cv::Point2d p_intersection_field;
      bool found_img = intersection(line_img1, line_img2, p_intersection_img_undistorted);
      bool found_field = intersection(line_field1, line_field2, p_intersection_field);

      cv::Rect2d rect_line1(lines_start[i], lines_end[i]);
      cv::Rect2d rect_line2(lines_start[j], lines_end[j]);
      if (found_img
          && found_field
          // check if intersection is on line segment
          && contains(rect_line1, p_intersection_field, 10)
          && contains(rect_line2, p_intersection_field, 10)) {
        cv::Point3d p_intersection_img_undistorted_3d(
            p_intersection_img_undistorted.x, p_intersection_img_undistorted.y, 0);
        points_intersection_img_undistorted.push_back(p_intersection_img_undistorted_3d);
        cv::Point3d p_intersection_field_3d(p_intersection_field.x, p_intersection_field.y, 0);
        points_intersection_field.push_back(p_intersection_field_3d);
      } else {
        std::cout << "No calibration points found for "
            << line_field1 << " -> " << line_field2
            << " ( " << line_img1 << " -> " << line_img2 << " )"
            << std::endl
            << "found_img: " << found_img
            << " found_field: " << found_field
            << std::endl
            << "intersection img: " << p_intersection_img_undistorted
            << " intersection_field: " << p_intersection_field
            << std::endl;
      }
    }
  }
  if (!points_intersection_img_undistorted.empty()) {
    vector<double> zero(3);
    vector<cv::Point2d> points_img_intersect;
    // project undistorted image points back to image (distort)
    cv::projectPoints(points_intersection_img_undistorted,
                      zero,
                      zero,
                      intrinsic_parameters->camera_mat,
                      intrinsic_parameters->dist_coeffs,
                      points_img_intersect);
    for (int i = 0; i < (int) points_img_intersect.size(); i++) {
      if (points_img_intersect[i].x >= 0
         && points_img_intersect[i].y >= 0
         && points_img_intersect[i].x < additional_calibration_information->imageWidth->getInt()
         && points_img_intersect[i].y < additional_calibration_information->imageHeight->getInt()) {
        extrinsic_parameters->addCalibrationPointSet(points_img_intersect[i], points_intersection_field[i]);
      } else {
        std::cout << "Calibration point outside image: "
          << points_img_intersect[i]
          << std::endl;
      }
    }
  }
}

void CameraParameters::calibrate(
    std::vector<GVector::vector3d<double> > &p_f,
    std::vector<GVector::vector2d<double> > &p_i, int cal_type) {
  assert(p_f.size() == p_i.size());
  assert(cal_type != 0);

  p_to_est.clear();
  p_to_est.push_back(FOCAL_LENGTH);
  p_to_est.push_back(Q_1);
  p_to_est.push_back(Q_2);
  p_to_est.push_back(Q_3);
  p_to_est.push_back(T_1);
  p_to_est.push_back(T_2);

  int num_alpha(0);

  if (cal_type & FULL_ESTIMATION) {
    int count_alpha(0); //The number of well detected line segment points
    auto ls_it = calibrationSegments.begin();
    for (; ls_it != calibrationSegments.end(); ls_it++) {
      auto pts_it = (*ls_it).points.begin();
      for (; pts_it != (*ls_it).points.end(); pts_it++) {
        if (pts_it->detected) count_alpha ++;
      }
    }


    if (count_alpha > 0) {
      p_alpha = Eigen::VectorXd(count_alpha);

      count_alpha = 0;
      ls_it = calibrationSegments.begin();
      for (; ls_it != calibrationSegments.end(); ls_it++) {
        auto pts_it = (*ls_it).points.begin();
        auto alphas_it = (*ls_it).alphas.begin();
        for (; pts_it != (*ls_it).points.end() &&
            alphas_it != (*ls_it).alphas.end(); pts_it++, alphas_it++) {
          if (pts_it->detected) {
            p_alpha(count_alpha++) = (*alphas_it);
          }
        }
      }
    }

    p_to_est.push_back(PP_X);
    p_to_est.push_back(PP_Y);
    p_to_est.push_back(DIST);

    num_alpha = count_alpha;
  }

  double lambda(0.01);

  Eigen::VectorXd p(STATE_SPACE_DIMENSION + num_alpha);
  p.setZero();

  // Calculate first chisqr for all points using the start parameters
  double old_chisqr = calc_chisqr(p_f, p_i, p, cal_type);

#ifndef NDEBUG
  std::cerr << "Chi-square: "<< old_chisqr << std::endl;
#endif

  // Create and fill corner measurement covariance matrix
  Eigen::Matrix2d cov_corner_inv;
  cov_corner_inv <<
      1 / additional_calibration_information->cov_corner_x->getDouble(), 0 , 0 ,
      1 / additional_calibration_information->cov_corner_y->getDouble();

  // Create and fill line segment measurement covariance matrix
  Eigen::Matrix2d cov_ls_inv;
  cov_ls_inv << 1 / additional_calibration_information->cov_ls_x->getDouble(),
      0 , 0 , 1 / additional_calibration_information->cov_ls_y->getDouble();

  // Matrices for A, b and the Jacobian J
  Eigen::MatrixXd alpha(STATE_SPACE_DIMENSION + num_alpha,
                        STATE_SPACE_DIMENSION + num_alpha);
  Eigen::VectorXd beta(STATE_SPACE_DIMENSION + num_alpha, 1);
  Eigen::MatrixXd J(2, STATE_SPACE_DIMENSION + num_alpha);

  bool stop_optimization(false);
  int convergence_counter(0);
  double t_start=GetTimeSec();
  while (!stop_optimization) {
    // Calculate Jacobi-Matrix, alpha and beta
    // Iterate over alle point pairs
    auto it_p_f  = p_f.begin();
    auto it_p_i  = p_i.begin();

    double epsilon = sqrt(std::numeric_limits<double>::epsilon());

    alpha.setZero();
    beta.setZero();

    for (; it_p_f != p_f.end(); it_p_f++, it_p_i++) {
      J.setZero();

      GVector::vector2d<double> proj_p;
      field2image(*it_p_f, proj_p, p);
      proj_p = proj_p - *it_p_i;

      auto it = p_to_est.begin();
      for (; it != p_to_est.end(); it++) {
        int i = *it;

        Eigen::VectorXd p_diff = p;
        p_diff(i) = p_diff(i) + epsilon;

        GVector::vector2d<double> proj_p_diff;
        field2image(*it_p_f, proj_p_diff, p_diff);
        J(0,i) = ((proj_p_diff.x - (*it_p_i).x) - proj_p.x) / epsilon;
        J(1,i) = ((proj_p_diff.y - (*it_p_i).y) - proj_p.y) / epsilon;
      }

      alpha += J.transpose() * cov_corner_inv * J;
      beta += J.transpose() * cov_corner_inv *
          Eigen::Vector2d(proj_p.x, proj_p.y);
    }

    if (cal_type & FULL_ESTIMATION) {
      // First, calculate how many alpha we need to estimate
      auto ls_it = calibrationSegments.begin();

      int i = 0;
      for (; ls_it != calibrationSegments.end(); ls_it++) {
        auto pts_it = (*ls_it).points.begin();
        for (; pts_it != (*ls_it).points.end(); pts_it++) {
          if (pts_it->detected) {
            GVector::vector2d<double> proj_p;
            GVector::vector3d<double >alpha_point;
            if (ls_it->straightLine) {
              alpha_point = p_alpha(i) * (*ls_it).p1 + (1 - p_alpha(i)) * (*ls_it).p2;
            } else {
              double theta = p_alpha(i) * (*ls_it).theta1 + (1.0 - p_alpha(i)) * (*ls_it).theta2;
              alpha_point = ls_it->center + ls_it->radius*GVector::vector3d<double>(cos(theta),sin(theta),0.0);
            }
            field2image(alpha_point, proj_p, p);
            proj_p = proj_p - (*pts_it).img_point;

            J.setZero();

            auto it = p_to_est.begin();
            for (; it != p_to_est.end(); it++) {
              int j = *it;
              Eigen::VectorXd p_diff = p;
              p_diff(j) = p_diff(j) + epsilon;
              GVector::vector2d<double> proj_p_diff;
              field2image(alpha_point, proj_p_diff, p_diff);
              J(0,j) = ((proj_p_diff.x - (*pts_it).img_point.x) - proj_p.x) /
                  epsilon;
              J(1,j) = ((proj_p_diff.y - (*pts_it).img_point.y) - proj_p.y) /
                  epsilon;
            }

            double my_alpha = p_alpha(i) + epsilon;
            if (ls_it->straightLine) {
              alpha_point = my_alpha * (*ls_it).p1 + (1 - my_alpha) *
                  (*ls_it).p2;
            } else {
              double theta = my_alpha * (*ls_it).theta1 + (1.0 - my_alpha) *
                  (*ls_it).theta2;
              alpha_point = ls_it->center +
                  ls_it->radius*GVector::vector3d<double>(cos(theta),
                                                          sin(theta),0.0);
            }


            GVector::vector2d<double> proj_p_diff;
            field2image(alpha_point, proj_p_diff);
            J(0,STATE_SPACE_DIMENSION + i) =
                ((proj_p_diff.x - (*pts_it).img_point.x) - proj_p.x) / epsilon;
            J(1,STATE_SPACE_DIMENSION + i) =
                ((proj_p_diff.y - (*pts_it).img_point.y) - proj_p.y) / epsilon;

            alpha += J.transpose() * cov_ls_inv * J;
            beta += J.transpose() * cov_ls_inv *
                Eigen::Vector2d(proj_p.x, proj_p.y);
            i++;
          }
        }
      }
    }

    // Augment alpha
    alpha += Eigen::MatrixXd::Identity(
        STATE_SPACE_DIMENSION + num_alpha, STATE_SPACE_DIMENSION + num_alpha)
        * lambda;

    // Solve for x
    Eigen::VectorXd new_p(STATE_SPACE_DIMENSION + num_alpha);

    // Due to an API change we need to check for
    // the right call at compile time
#ifdef EIGEN_WORLD_VERSION
    // alpha.llt().solve(-beta, &new_p); -- modify 1/15/16
    //  -- move to Eigen3 structure -
    //  -- http://eigen.tuxfamily.org/dox/Eigen2ToEigen3.html
    new_p = alpha.llt().solve(-beta);
#else
    Eigen::Cholesky<Eigen::MatrixXd> c(alpha);
    new_p = c.solve(-beta);
#endif

    // Calculate chisqr again
    double chisqr = calc_chisqr(p_f, p_i, new_p, cal_type);

    if (chisqr < old_chisqr) {
      focal_length->setDouble(focal_length->getDouble() + new_p[FOCAL_LENGTH]);
      principal_point_x->setDouble(
          principal_point_x->getDouble() + new_p[PP_X]);
      principal_point_y->setDouble(
          principal_point_y->getDouble() + new_p[PP_Y]);
      distortion->setDouble(distortion->getDouble() + new_p[DIST]);
      tx->setDouble(tx->getDouble() + new_p[T_1]);
      ty->setDouble(ty->getDouble() + new_p[T_2]);
      tz->setDouble(tz->getDouble() + new_p[T_3]);

      Quaternion<double> q_diff;
      GVector::vector3d<double> aa_diff(new_p[Q_1], new_p[Q_2], new_p[Q_3]);
      q_diff.setAxis(aa_diff.norm(), aa_diff.length());
      Quaternion<double> q_field2cam = Quaternion<double>(
          q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
      q_field2cam.norm();
      q_field2cam = q_diff * q_field2cam ;
      q0->setDouble(q_field2cam.x);
      q1->setDouble(q_field2cam.y);
      q2->setDouble(q_field2cam.z);
      q3->setDouble(q_field2cam.w);

      for (int i=0; i < num_alpha; i++)
        p_alpha[i] += new_p[STATE_SPACE_DIMENSION + i];

      // Normalize focal length an orientation when the optimization tends to go into the wrong
      // of both possible projections
      if (focal_length->getDouble() < 0) {
        focal_length->setDouble(-focal_length->getDouble());
        q_field2cam = q_rotate180 * q_field2cam;
        q0->setDouble(q_field2cam.x);
        q1->setDouble(q_field2cam.y);
        q2->setDouble(q_field2cam.z);
        q3->setDouble(q_field2cam.w);
      }

      if (old_chisqr - chisqr < 0.001) {
        stop_optimization = true;
      } else {
        lambda /= 10;
        convergence_counter = 0;
      }

      old_chisqr = chisqr;
#ifndef NDEBUG
      std::cerr << "Chi-square: "<< old_chisqr << std::endl;
#endif
    } else {
      lambda *= 10;
      if (convergence_counter++ > 10) stop_optimization = true;
    }
    if ((GetTimeSec() - t_start) >
        additional_calibration_information->convergence_timeout->getDouble()) {
      stop_optimization=true;
    }
  }

// Debug output starts here
#ifndef NDEBUG

  // Estimated parameters
  std::cerr << "Estimated parameters: " << std::endl;
  std::cerr << focal_length->getDouble() << " "
            << principal_point_x->getDouble() << " "
            << principal_point_y->getDouble() << " " << distortion->getDouble()
            << std::endl;
  std::cerr << q0->getDouble() << " " << q1->getDouble() << " "
            << q2->getDouble() << " " << q3->getDouble() << std::endl;
  std::cerr << tx->getDouble() << " " << ty->getDouble() << " "
            << tz->getDouble() <<  std::endl;
  std::cerr << "alphas: " << p_alpha << std::endl;

  // Testing calibration by projecting the four field points into the image
  // plane and calculate MSE
  auto it_p_f  = p_f.begin();
  auto it_p_i  = p_i.begin();

  double corner_x(0);
  double corner_y(0);
  double corner_sum(0);
  for (; it_p_f != p_f.end(); it_p_f++, it_p_i++) {
    GVector::vector2d<double> proj_p;
    field2image(*it_p_f, proj_p);
    GVector::vector3d<double> some_point;
    image2field(some_point,proj_p, 0);
    std::cerr << "Point in world: ("<< it_p_f->x << "," << it_p_f->y << ","
              << it_p_f->z  << ")" << std::endl;
    std::cerr << "Point should be at (" << it_p_i->x << "," << it_p_i->y
              << ") and is projected at (" << proj_p.x << "," << proj_p.y <<")"
              << std::endl;

    double diff_x = proj_p.x - it_p_i->x;
    double diff_y = proj_p.y - it_p_i->y;
    corner_x += diff_x * diff_x;
    corner_y += diff_y * diff_y;
    corner_sum += diff_x * diff_x + diff_y * diff_y;
  }

  std::cerr << "RESIDUAL CORNER POINTS: "
            << sqrt(corner_x/4) << " "
            << sqrt(corner_y/4) << " "
            << sqrt(corner_sum/4) << std::endl;

 if (cal_type & FULL_ESTIMATION) {
  // Testing calibration by projecting the points on the lines into the image
  // plane and calculate MSE
  double line_x(0);
  double line_y(0);

  auto ls_it = calibrationSegments.begin();

  int i = 0;
  for (; ls_it != calibrationSegments.end(); ls_it++) {
    auto pts_it = (*ls_it).points.begin();
    for (; pts_it != (*ls_it).points.end(); pts_it++) {
      if (pts_it->detected) {
        GVector::vector2d<double> proj_p;
        double pts_alpha = p_alpha(i);
        GVector::vector3d<double >alpha_point;
        if (ls_it->straightLine) {
          alpha_point = pts_alpha * (*ls_it).p1 + (1 - pts_alpha) * (*ls_it).p2;
        } else {
          double theta = pts_alpha * (*ls_it).theta1 + (1.0 - pts_alpha) *
              (*ls_it).theta2;
          alpha_point = ls_it->center +
              ls_it->radius*GVector::vector3d<double>(
                  cos(theta),sin(theta),0.0);
        }

        field2image(alpha_point, proj_p, p);

        line_x += (proj_p.x - pts_it->img_point.x) * (proj_p.x - pts_it->img_point.x);
        line_y += (proj_p.y - pts_it->img_point.y) * (proj_p.y - pts_it->img_point.y);

        i++;
      }
    }
  }

  if (i != 0) {
    std::cerr << "RESIDUAL LINE POINTS: " << sqrt(line_x/(double)i) << " "
              << sqrt(line_y/(double)i) << std::endl;
  }
 }
#else
  return;
#endif
}


CameraParameters::AdditionalCalibrationInformation::
    AdditionalCalibrationInformation(int camera_index_, RoboCupField* field_) :
        field(field_) {

  camera_index = new VarInt("camera index", camera_index_);

    for (int i = 0; i < kNumControlPoints; ++i) {
        ostringstream convert;
        convert << i;
        const string i_str = convert.str();
        control_point_set[i] = new VarList("Control Point " + i_str);
        control_point_names[i] = new VarString(
                "Control point " + i_str + " name", "CP " + i_str);
        control_point_set[i]->addChild(control_point_names[i]);
        control_point_image_xs[i] = new VarDouble(
                "Control point " + i_str + " image x", 50.0);
        control_point_set[i]->addChild(control_point_image_xs[i]);
        control_point_image_ys[i] = new VarDouble(
                "Control point " + i_str + " image y", 50.0 * (i+1));
        control_point_set[i]->addChild(control_point_image_ys[i]);
        control_point_field_xs[i] = new VarDouble(
                "Control point " + i_str + " field x", 0.0);
        control_point_set[i]->addChild(control_point_field_xs[i]);
        control_point_field_ys[i] = new VarDouble(
                "Control point " + i_str + " field y", 0.0);
        control_point_set[i]->addChild(control_point_field_ys[i]);
    }

  initial_distortion = new VarDouble("initial distortion", 1.0);
  line_search_corridor_width = new VarDouble("line search corridor width", 280.0);
  image_boundary = new VarDouble("Image boundary for edge detection", 10.0);
  max_feature_distance = new VarDouble("Max distance of edge from camera", 20000.0);
  convergence_timeout = new VarDouble("convergence timeout (s)", 10.0);
  cov_corner_x = new VarDouble("Cov corner measurement x", 1.0);
  cov_corner_y = new VarDouble("Cov corner measurement y", 1.0);
  cov_ls_x = new VarDouble("Cov line segment measurement x", 1.0);
  cov_ls_y = new VarDouble("Cov line segment measurement y", 1.0);
  pointSeparation = new VarDouble("Points separation", 150);

  imageWidth = new VarInt("Image width",0,0);
  imageWidth->addFlags(VARTYPE_FLAG_NOLOAD_ATTRIBUTES);
  imageHeight = new VarInt("Image height",0,0);
  imageHeight->addFlags(VARTYPE_FLAG_NOLOAD_ATTRIBUTES);
  grid_width = new VarInt("grid width", 7);
  grid_height = new VarInt("grid height", 9);
  global_camera_id = new VarInt("global camera id", camera_index_, 0, 7);
  global_camera_id->setFlags(VARTYPE_FLAG_HIDDEN);
}

void CameraParameters::AdditionalCalibrationInformation::updateControlPoints() {

    std::vector<GVector::vector2d<double> > control_points = generateCameraControlPoints(
            camera_index->getInt(), field->num_cameras_total->getInt(), field->field_length->getDouble(), field->field_width->getDouble());

    if(control_points.empty())
    {
        return;
    }

    for (int i = 0; i < kNumControlPoints; ++i) {
        control_point_field_xs[i]->setDouble(control_points[i].x);
        control_point_field_ys[i]->setDouble(control_points[i].y);
    }
}

std::vector<GVector::vector2d<double> > CameraParameters::AdditionalCalibrationInformation::
generateCameraControlPoints(int cameraId, int numCamerasTotal, double fieldHeight, double fieldWidth) {

  std::vector<GVector::vector2d<double> > points;

  int nCamerasX;
  int nCamerasY;
  switch (numCamerasTotal) {
    case 1:
      nCamerasX = 1;
      nCamerasY = 1;
      break;
    case 2:
      nCamerasX = 2;
      nCamerasY = 1;
      break;
    case 4:
      nCamerasX = 2;
      nCamerasY = 2;
      break;
    case 6:
      nCamerasX = 3;
      nCamerasY = 2;
      break;
    case 8:
      nCamerasX = 4;
      nCamerasY = 2;
      break;
    default:
      std::cerr << "Invalid camera id: " << cameraId << " for " << numCamerasTotal << " total cameras"
                << std::endl;
      return points;
  }
  double xStep = fieldHeight / nCamerasX;
  double yStep = fieldWidth / nCamerasY;

  int xIdx = cameraId / nCamerasY;
  int yIdx = cameraId % nCamerasY;
  double x1 = -fieldHeight / 2 + xIdx * xStep;
  double x2 = x1 + xStep;
  double y1 = -fieldWidth / 2 + yIdx * yStep;
  double y2 = y1 + yStep;

  points.emplace_back(x1, y1);
  points.emplace_back(x1, y2);
  points.emplace_back(x2, y2);
  points.emplace_back(x2, y1);
  return points;
}

CameraParameters::AdditionalCalibrationInformation::~AdditionalCalibrationInformation() {
  for (int i = 0; i < kNumControlPoints; ++i) {
    delete control_point_names[i];
    delete control_point_image_xs[i];
    delete control_point_image_ys[i];
    delete control_point_field_xs[i];
    delete control_point_field_ys[i];
  }

  delete initial_distortion;
  delete line_search_corridor_width;
  delete image_boundary;
  delete max_feature_distance;
  delete convergence_timeout;
  delete cov_corner_x;
  delete cov_corner_y;
  delete cov_ls_x;
  delete cov_ls_y;
  delete pointSeparation;
  delete imageWidth;
  delete imageHeight;
}

void CameraParameters::AdditionalCalibrationInformation::addSettingsToList(
    VarList& list) {
  for (auto & i : control_point_set) {
    list.addChild(i);
  }

  list.addChild(camera_index);
  list.addChild(initial_distortion);
  list.addChild(line_search_corridor_width);
  list.addChild(image_boundary);
  list.addChild(max_feature_distance);
  list.addChild(convergence_timeout);
  list.addChild(cov_corner_x);
  list.addChild(cov_corner_y);
  list.addChild(cov_ls_x);
  list.addChild(cov_ls_y);
  list.addChild(pointSeparation);
  list.addChild(imageWidth);
  list.addChild(imageHeight);
  list.addChild(grid_height);
  list.addChild(grid_width);
  list.addChild(global_camera_id);
}

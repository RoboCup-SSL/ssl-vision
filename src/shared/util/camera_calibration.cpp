#include "camera_calibration.h"
#include <Eigen/Cholesky>
#include <Eigen/Dense>
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

void CameraParameters::toProtoBuffer(SSL_GeometryCameraCalibration &buffer) const {
  buffer.set_focal_length(focal_length->getDouble());
  buffer.set_principal_point_x(principal_point_x->getDouble());
  buffer.set_principal_point_y(principal_point_y->getDouble());
  buffer.set_distortion(distortion->getDouble());
  buffer.set_q0(q0->getDouble());
  buffer.set_q1(q1->getDouble());
  buffer.set_q2(q2->getDouble());
  buffer.set_q3(q3->getDouble());
  buffer.set_tx(tx->getDouble());
  buffer.set_ty(ty->getDouble());
  buffer.set_tz(tz->getDouble());
  buffer.set_camera_id(additional_calibration_information->camera_index->getInt());

  //--Set derived parameters:
  //compute camera world coordinates:
  Quaternion<double> q;
  q.set(q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
  q.invert();

  GVector::vector3d<double> v_in(tx->getDouble(),ty->getDouble(),tz->getDouble());
  v_in=(-(v_in));

  GVector::vector3d<double> v_out = q.rotateVectorByQuaternion(v_in);
  buffer.set_derived_camera_world_tx(v_out.x);
  buffer.set_derived_camera_world_ty(v_out.y);
  buffer.set_derived_camera_world_tz(v_out.z);

  buffer.set_pixel_image_width(additional_calibration_information->imageWidth->getInt());
  buffer.set_pixel_image_height(additional_calibration_information->imageHeight->getInt());

}

GVector::vector3d< double > CameraParameters::getWorldLocation() {
  Quaternion<double> q;
  q.set(q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
  q.invert();
  GVector::vector3d<double> v_in(tx->getDouble(),ty->getDouble(),tz->getDouble());
  v_in = (-(v_in));
  GVector::vector3d<double> v_out = q.rotateVectorByQuaternion(v_in);
  return v_out;
}

void CameraParameters::addSettingsToList(VarList& list) {
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

double CameraParameters::radialDistortion(double ru, double dist) const {
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
    GVector::vector2d<double> &pd, double dist) const {
  double rd = radialDistortion(pu.length(), dist);
  pd = pu;
  pd = pd.norm(rd);
}

void CameraParameters::field2image(
    const GVector::vector3d<double> &p_f,
    GVector::vector2d<double> &p_i) const {
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

void CameraParameters::field2image(
    GVector::vector3d<double> &p_f, GVector::vector2d<double> &p_i,
    Eigen::VectorXd &p) {
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
  // Undo scaling and offset
  GVector::vector2d<double> p_d(
      (p_i.x - principal_point_x->getDouble()) / focal_length->getDouble(),
      (p_i.y - principal_point_y->getDouble()) / focal_length->getDouble());

  // Compensate for distortion (undistort)
  GVector::vector2d<double> p_un;
  radialDistortionInv(p_un,p_d);

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
  std::vector<GVector::vector3d<double> >::iterator it_p_f  = p_f.begin();
  std::vector<GVector::vector2d<double> >::iterator it_p_i  = p_i.begin();

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
    std::vector<CalibrationData>::iterator ls_it = calibrationSegments.begin();

    int i = 0;
    for (; ls_it != calibrationSegments.end(); ls_it++) {
      std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator
          imgPts_it = (*ls_it).imgPts.begin();
      for (; imgPts_it != (*ls_it).imgPts.end(); imgPts_it++) {
        // Integrate only if a valid point on line
        if (imgPts_it->second) {
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

          chisqr += (proj_p.x-imgPts_it->first.x) *
              (proj_p.x-imgPts_it->first.x) * cov_lsx_inv +
              (proj_p.y - imgPts_it->first.y) *
              (proj_p.y - imgPts_it->first.y) * cov_lsy_inv;
          i++;
        }
      }
    }
  }
  return chisqr;
}

void CameraParameters::do_calibration(int cal_type) {
  std::vector<GVector::vector3d<double> > p_f;
  std::vector<GVector::vector2d<double> > p_i;

  AdditionalCalibrationInformation* aci = additional_calibration_information;

  for (int i = 0; i < AdditionalCalibrationInformation::kNumControlPoints;
      ++i) {
    p_i.push_back(GVector::vector2d<double>(
      aci->control_point_image_xs[i]->getDouble(),
      aci->control_point_image_ys[i]->getDouble()));
    p_f.push_back(GVector::vector3d<double>(
      aci->control_point_field_xs[i]->getDouble(),
      aci->control_point_field_ys[i]->getDouble(), 0.0));
  }

  calibrate(p_f, p_i, cal_type);
}

void CameraParameters::reset() {
  focal_length->resetToDefault();
  principal_point_x->resetToDefault();
  principal_point_y->resetToDefault();
  distortion->resetToDefault();
  tx->resetToDefault();
  ty->resetToDefault();
  tz->resetToDefault();
  q0->resetToDefault();
  q1->resetToDefault();
  q2->resetToDefault();
  q3->resetToDefault();
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
    std::vector<CalibrationData>::iterator ls_it = calibrationSegments.begin();
    for (; ls_it != calibrationSegments.end(); ls_it++) {
      std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator
          pts_it = (*ls_it).imgPts.begin();
      for (; pts_it != (*ls_it).imgPts.end(); pts_it++) {
        if (pts_it->second) count_alpha ++;
      }
    }


    if (count_alpha > 0) {
      p_alpha = Eigen::VectorXd(count_alpha);

      count_alpha = 0;
      ls_it = calibrationSegments.begin();
      for (; ls_it != calibrationSegments.end(); ls_it++) {
        std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator
            pts_it = (*ls_it).imgPts.begin();
        std::vector< double >::iterator alphas_it = (*ls_it).alphas.begin();
        for (; pts_it != (*ls_it).imgPts.end() &&
            alphas_it != (*ls_it).alphas.end(); pts_it++, alphas_it++) {
          if (pts_it->second) {
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
    std::vector<GVector::vector3d<double> >::iterator it_p_f  = p_f.begin();
    std::vector<GVector::vector2d<double> >::iterator it_p_i  = p_i.begin();

    double epsilon = sqrt(std::numeric_limits<double>::epsilon());

    alpha.setZero();
    beta.setZero();

    for (; it_p_f != p_f.end(); it_p_f++, it_p_i++) {
      J.setZero();

      GVector::vector2d<double> proj_p;
      field2image(*it_p_f, proj_p, p);
      proj_p = proj_p - *it_p_i;

      std::vector<int>::iterator it = p_to_est.begin();
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
      std::vector<CalibrationData>::iterator ls_it =
          calibrationSegments.begin();

      int i = 0;
      for (; ls_it != calibrationSegments.end(); ls_it++) {
        std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator
            pts_it = (*ls_it).imgPts.begin();
        for (; pts_it != (*ls_it).imgPts.end(); pts_it++) {
          if (pts_it->second) {
            GVector::vector2d<double> proj_p;
            GVector::vector3d<double >alpha_point;
            if (ls_it->straightLine) {
              alpha_point = p_alpha(i) * (*ls_it).p1 + (1 - p_alpha(i)) * (*ls_it).p2;
            } else {
              double theta = p_alpha(i) * (*ls_it).theta1 + (1.0 - p_alpha(i)) * (*ls_it).theta2;
              alpha_point = ls_it->center + ls_it->radius*GVector::vector3d<double>(cos(theta),sin(theta),0.0);
            }
            field2image(alpha_point, proj_p, p);
            proj_p = proj_p - (*pts_it).first;

            J.setZero();

            std::vector<int>::iterator it = p_to_est.begin();
            for (; it != p_to_est.end(); it++) {
              int j = *it;
              Eigen::VectorXd p_diff = p;
              p_diff(j) = p_diff(j) + epsilon;
              GVector::vector2d<double> proj_p_diff;
              field2image(alpha_point, proj_p_diff, p_diff);
              J(0,j) = ((proj_p_diff.x - (*pts_it).first.x) - proj_p.x) /
                  epsilon;
              J(1,j) = ((proj_p_diff.y - (*pts_it).first.y) - proj_p.y) /
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
                ((proj_p_diff.x - (*pts_it).first.x) - proj_p.x) / epsilon;
            J(1,STATE_SPACE_DIMENSION + i) =
                ((proj_p_diff.y - (*pts_it).first.y) - proj_p.y) / epsilon;

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
  std::vector<GVector::vector3d<double> >::iterator it_p_f  = p_f.begin();
  std::vector<GVector::vector2d<double> >::iterator it_p_i  = p_i.begin();

  double corner_x(0);
  double corner_y(0);
  for (; it_p_f != p_f.end(); it_p_f++, it_p_i++) {
    GVector::vector2d<double> proj_p;
    field2image(*it_p_f, proj_p);
    GVector::vector3d<double> some_point;
    image2field(some_point,proj_p, 150);
    std::cerr << "Point in world: ("<< it_p_f->x << "," << it_p_f->y << ","
              << it_p_f->z  << ")" << std::endl;
    std::cerr << "Point should be at (" << it_p_i->x << "," << it_p_i->y
              << ") and is projected at (" << proj_p.x << "," << proj_p.y <<")"
              << std::endl;

    corner_x += (proj_p.x - it_p_i->x) * (proj_p.x - it_p_i->x);
    corner_y += (proj_p.y - it_p_i->y) * (proj_p.y - it_p_i->y);
  }

  std::cerr << "RESIDUAL CORNER POINTS: " << sqrt(corner_x/4) << " "
            << sqrt(corner_y/4) << std::endl;

 if (cal_type & FULL_ESTIMATION) {
  // Testing calibration by projecting the points on the lines into the image
  // plane and calculate MSE
  double line_x(0);
  double line_y(0);

  std::vector<CalibrationData>::iterator ls_it = calibrationSegments.begin();

  int i = 0;
  for (; ls_it != calibrationSegments.end(); ls_it++) {
    std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator pts_it =
        (*ls_it).imgPts.begin();
    for (; pts_it != (*ls_it).imgPts.end(); pts_it++) {
      if (pts_it->second) {
        GVector::vector2d<double> proj_p;
        double alpha = p_alpha(i);
        GVector::vector3d<double >alpha_point;
        if (ls_it->straightLine) {
          alpha_point = alpha * (*ls_it).p1 + (1 - alpha) * (*ls_it).p2;
        } else {
          double theta = alpha * (*ls_it).theta1 + (1.0 - alpha) *
              (*ls_it).theta2;
          alpha_point = ls_it->center +
              ls_it->radius*GVector::vector3d<double>(
                  cos(theta),sin(theta),0.0);
        }

        field2image(alpha_point, proj_p, p);

        line_x += (proj_p.x - pts_it->first.x) * (proj_p.x - pts_it->first.x);
        line_y += (proj_p.y - pts_it->first.y) * (proj_p.y - pts_it->first.y);

        i++;
      }
    }
  }

  if (i != 0) {
    std::cerr << "RESIDUAL LINE POINTS: " << sqrt(line_x/(double)i) << " "
              << sqrt(line_y/(double)i) << std::endl;
  }
 }

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

  points.push_back(GVector::vector2d<double>(x1, y1));
  points.push_back(GVector::vector2d<double>(x1, y2));
  points.push_back(GVector::vector2d<double>(x2, y2));
  points.push_back(GVector::vector2d<double>(x2, y1));
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
  for (int i = 0; i < kNumControlPoints; ++i) {
    list.addChild(control_point_set[i]);
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
}

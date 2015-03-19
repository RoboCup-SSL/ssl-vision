#include "camera_calibration.h"
#include <Eigen/Cholesky>
#include <iostream>
#include <algorithm>
#include <limits>
#include "field.h"
#include "geomalgo.h"

CameraParameters::CameraParameters(RoboCupCalibrationHalfField & _field) : field(_field), p_alpha(Eigen::VectorXd(1))
{
  focal_length = new VarDouble("focal length", 500.0);
  principal_point_x = new VarDouble("principal point x", 390.0);
  principal_point_y = new VarDouble("principal point y", 290.0);
  distortion = new VarDouble("distortion", 0.0, 0.0, 2.0);
  //do not overwrite min/max ranges with values from config file
  distortion->addFlags(VARTYPE_FLAG_NOLOAD_ATTRIBUTES);

  q0 = new VarDouble("q0", 0.7);
  q1 = new VarDouble("q1", -0.7);
  q2 = new VarDouble("q2", .0);
  q3 = new VarDouble("q3", .0);
  
  tx = new VarDouble("tx", 0);
  ty = new VarDouble("ty", 1250);
  
  tz = new VarDouble("tz", 3500, 0, 5000);
  //do not overwrite min/max ranges with values from config file
  tz->addFlags(VARTYPE_FLAG_NOLOAD_ATTRIBUTES);

  additional_calibration_information = new AdditionalCalibrationInformation();

  q_rotate180 = Quaternion<double>(0, 0, 1.0,0);
}

CameraParameters::~CameraParameters()
{
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

#ifndef NO_PROTOBUFFERS
void CameraParameters::toProtoBuffer(SSL_GeometryCameraCalibration & buffer, int camera_id) const {
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
  buffer.set_camera_id(camera_id);

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

}

void CameraParameters::fromProtoBuffer(const SSL_GeometryCameraCalibration & buffer) {
  focal_length->setDouble(buffer.focal_length());
  principal_point_x->setDouble(buffer.principal_point_x());
  principal_point_y->setDouble(buffer.principal_point_y());
  distortion->setDouble(buffer.distortion());
  q0->setDouble(buffer.q0());
  q1->setDouble(buffer.q1());
  q2->setDouble(buffer.q2());
  q3->setDouble(buffer.q3());
  tx->setDouble(buffer.tx());
  ty->setDouble(buffer.ty());
  tz->setDouble(buffer.tz());
}
#endif

void CameraParameters::addSettingsToList(VarList& list) 
{
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

double CameraParameters::radialDistortion(double ru) const
{
  if((distortion->getDouble())<=DBL_MIN)
    return ru;
  double rd = 0;
  double a = distortion->getDouble();
  double b = -9.0*a*a*ru + a*sqrt(a*(12.0 + 81.0*a*ru*ru));
  //Pre-computing numerical constants to optimize for speed...
  //b = b<0.0?-pow(b,1.0/3.0):pow(b,1.0/3.0);
  //rd = pow(2.0/3.0,1.0/3.0)/b - b/(pow(2.0*3.0*3.0,1.0/3.0)*a);
  b = b<0.0?-pow(-b,0.33333333333333333333333333333333333333333333333333):pow(b,0.33333333333333333333333333333333333333333333333333);
  rd = 0.87358046473629886904722042681399875674647588190788/b - b/(2.62074139420889660714166128044199627023942764572363*a);
  return rd;
}

double CameraParameters::radialDistortion(double ru, double dist) const
{
  if(dist<=DBL_MIN)
    return ru;
  double rd = 0;
  double a = dist;
  double b = -9.0*a*a*ru + a*sqrt(a*(12.0 + 81.0*a*ru*ru));
  //Pre-computing numerical constants to optimize for speed...
  //b = b<0.0?-pow(b,1.0/3.0):pow(b,1.0/3.0);
  //rd = pow(2.0/3.0,1.0/3.0)/b - b/(pow(2.0*3.0*3.0,1.0/3.0)*a);
  b = b<0.0?-pow(-b,0.33333333333333333333333333333333333333333333333333):pow(b,0.33333333333333333333333333333333333333333333333333);
  rd = 0.87358046473629886904722042681399875674647588190788/b - b/(2.62074139420889660714166128044199627023942764572363*a);
  return rd;
}

double CameraParameters::radialDistortionInv(double rd) const
{
  double ru = rd*(1.0+rd*rd*distortion->getDouble());
  return ru;
}
    
void CameraParameters::radialDistortionInv(GVector::vector2d<double> &pu, const GVector::vector2d<double> &pd) const
{
  double ru = radialDistortionInv(pd.length());
  pu = pd;
  pu = pu.norm(ru);
}
      
void CameraParameters::radialDistortion(const GVector::vector2d<double> pu, GVector::vector2d<double> &pd) const
{
  double rd = radialDistortion(pu.length());
  pd = pu;
  pd = pd.norm(rd);
}

void CameraParameters::radialDistortion(const GVector::vector2d<double> pu, GVector::vector2d<double> &pd, double dist) const
{
  double rd = radialDistortion(pu.length(), dist);
  pd = pu;
  pd = pd.norm(rd);
}

void CameraParameters::field2image(const GVector::vector3d<double> &p_f, GVector::vector2d<double> &p_i) const
{
  Quaternion<double> q_field2cam = Quaternion<double>(q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
  q_field2cam.norm();
  GVector::vector3d<double> translation = GVector::vector3d<double>(tx->getDouble(),ty->getDouble(),tz->getDouble());

  // First transform the point from the field into the coordinate system of the camera
  GVector::vector3d<double> p_c = q_field2cam.rotateVectorByQuaternion(p_f) + translation;
  GVector::vector2d<double> p_un = GVector::vector2d<double>(p_c.x/p_c.z, p_c.y/p_c.z);

  // Apply distortion
  GVector::vector2d<double> p_d;
  radialDistortion(p_un,p_d);
  
  // Then project from the camera coordinate system onto the image plane using the instrinsic parameters
  p_i = focal_length->getDouble() * p_d +
      GVector::vector2d<double>(principal_point_x->getDouble(), principal_point_y->getDouble());
}

void CameraParameters::field2image(GVector::vector3d<double> &p_f, GVector::vector2d<double> &p_i, Eigen::VectorXd &p)
{
  Quaternion<double> q_field2cam = Quaternion<double>(q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
  q_field2cam.norm();
  GVector::vector3d<double> translation = GVector::vector3d<double>(tx->getDouble(),ty->getDouble(),tz->getDouble());

  GVector::vector3d<double> aa_diff(p[Q_1], p[Q_2], p[Q_3]);
  GVector::vector3d<double> t_diff(p[T_1], p[T_2], p[T_3]);

  // Create a quaternion out of the 3D rotational representation
  Quaternion<double> q_diff;
  q_diff.setAxis(aa_diff.norm(), aa_diff.length());

  // First transform the point from the field into the coordinate system of the camera
  GVector::vector3d<double> p_c = (q_diff * q_field2cam ).rotateVectorByQuaternion(p_f) + translation + t_diff;
  GVector::vector2d<double> p_un = GVector::vector2d<double>(p_c.x/p_c.z, p_c.y/p_c.z);

  // Apply distortion
  GVector::vector2d<double> p_d;
  radialDistortion(p_un,p_d,distortion->getDouble() + p[DIST]);

  // Then project from the camera coordinate system onto the image plane using the instrinsic parameters
  p_i = (focal_length->getDouble() + p[FOCAL_LENGTH]) * p_d +
      GVector::vector2d<double>(principal_point_x->getDouble() + p[PP_X], principal_point_y->getDouble() + p[PP_Y]);
}

void CameraParameters::image2field(GVector::vector3d<double> &p_f, GVector::vector2d<double> &p_i, double z) const
{
  // Undo scaling and offset
  GVector::vector2d<double> p_d((p_i.x - principal_point_x->getDouble()) / focal_length->getDouble(),
                                  (p_i.y - principal_point_y->getDouble()) / focal_length->getDouble());
  
  // Compensate for distortion (undistort)
  GVector::vector2d<double> p_un;
  radialDistortionInv(p_un,p_d);
  
  // Now we got a ray on the z axis
  GVector::vector3d<double> v(p_un.x, p_un.y, 1);

  // Transform this ray into world coordinates
  Quaternion<double> q_field2cam = Quaternion<double>(q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
  q_field2cam.norm();
  GVector::vector3d<double> translation = GVector::vector3d<double>(tx->getDouble(),ty->getDouble(),tz->getDouble());

  Quaternion<double> q_field2cam_inv = q_field2cam;
  q_field2cam_inv.invert();
  GVector::vector3d<double> v_in_w = q_field2cam_inv.rotateVectorByQuaternion(v);
  GVector::vector3d<double> zero_in_w = q_field2cam_inv.rotateVectorByQuaternion(GVector::vector3d<double>(0,0,0) - translation);

  // Compute the the point where the rays intersects the field
  double t = GVector::ray_plane_intersect(GVector::vector3d<double>(0,0,z), GVector::vector3d<double>(0,0,1).norm(), zero_in_w, v_in_w.norm());

  // Set p_f
  p_f = zero_in_w + v_in_w.norm() * t;
}


double CameraParameters::calc_chisqr(std::vector<GVector::vector3d<double> > &p_f, std::vector<GVector::vector2d<double> > &p_i, Eigen::VectorXd &p, int cal_type)
{
  assert(p_f.size() == p_i.size());
  
  double cov_cx_inv = 1 / additional_calibration_information->cov_corner_x->getDouble();
  double cov_cy_inv = 1 / additional_calibration_information->cov_corner_y->getDouble();

  double cov_lsx_inv = 1 / additional_calibration_information->cov_ls_x->getDouble();
  double cov_lsy_inv = 1 / additional_calibration_information->cov_ls_y->getDouble();

  double chisqr(0);

  // Iterate over manual points
  std::vector<GVector::vector3d<double> >::iterator it_p_f  = p_f.begin();
  std::vector<GVector::vector2d<double> >::iterator it_p_i  = p_i.begin();

  for(; it_p_f != p_f.end(); it_p_f++, it_p_i++)
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
    for (; ls_it != calibrationSegments.end(); ls_it++)
    {
      std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator imgPts_it = (*ls_it).imgPts.begin();
      for(; imgPts_it != (*ls_it).imgPts.end(); imgPts_it++)
      {
        // Integrate only if a valid point on line
        if (imgPts_it->second)
        { 
          GVector::vector2d<double> proj_p; 
          double alpha = p_alpha(i) + p(STATE_SPACE_DIMENSION + i);
          
          // Calculate point on segment
          GVector::vector3d<double> alpha_point;
          if(ls_it->straightLine){
            alpha_point = alpha * (*ls_it).p1 + (1.0 - alpha) * (*ls_it).p2;
          }else{
            double theta = alpha * (*ls_it).theta1 + (1.0 - alpha) * (*ls_it).theta2;
            alpha_point = ls_it->center + ls_it->radius*GVector::vector3d<double>(cos(theta),sin(theta),0.0);
          }
          
          // Project into image plane
          field2image(alpha_point, proj_p, p);
          
          chisqr += (proj_p.x-imgPts_it->first.x) * (proj_p.x-imgPts_it->first.x) * cov_lsx_inv +
              (proj_p.y - imgPts_it->first.y) * (proj_p.y - imgPts_it->first.y) * cov_lsy_inv;
          i++;
        }
      }
    }
  }

  return chisqr;
}

void CameraParameters::do_calibration(int cal_type)
{
  std::vector<GVector::vector3d<double> > p_f;
  std::vector<GVector::vector2d<double> > p_i;
  
  AdditionalCalibrationInformation* aci = additional_calibration_information;
  
  p_i.push_back(GVector::vector2d<double>(aci->left_corner_image_x->getDouble(), aci->left_corner_image_y->getDouble()));
  p_i.push_back(GVector::vector2d<double>(aci->right_corner_image_x->getDouble(), aci->right_corner_image_y->getDouble()));
  p_i.push_back(GVector::vector2d<double>(aci->left_centerline_image_x->getDouble(), aci->left_centerline_image_y->getDouble()));
  p_i.push_back(GVector::vector2d<double>(aci->right_centerline_image_x->getDouble(), aci->right_centerline_image_y->getDouble()));

  p_f.push_back(GVector::vector3d<double>(field.left_corner_x->getDouble(), field.left_corner_y->getDouble(),0));
  p_f.push_back(GVector::vector3d<double>(field.right_corner_x->getDouble(), field.right_corner_y->getDouble(),0));
  p_f.push_back(GVector::vector3d<double>(field.left_centerline_x->getDouble(), field.left_centerline_y->getDouble(),0));
  p_f.push_back(GVector::vector3d<double>(field.right_centerline_x->getDouble(), field.right_centerline_y->getDouble(),0));

  calibrate(p_f, p_i, cal_type);
}

void CameraParameters::reset()
{
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

void CameraParameters::calibrate(std::vector<GVector::vector3d<double> > &p_f, std::vector<GVector::vector2d<double> > &p_i, int cal_type)
{
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

  if (cal_type & FULL_ESTIMATION)
  {
    int count_alpha(0); //The number of well detected line segment points
    std::vector<CalibrationData>::iterator ls_it = calibrationSegments.begin();
    for (; ls_it != calibrationSegments.end(); ls_it++)
    {
      std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator pts_it = (*ls_it).imgPts.begin();
      for(; pts_it != (*ls_it).imgPts.end(); pts_it++)
      {
        if (pts_it->second)
          count_alpha ++;
      }
    }
    
    
    if (count_alpha > 0)
    {
      p_alpha = Eigen::VectorXd(count_alpha);

      count_alpha = 0;
      ls_it = calibrationSegments.begin();
      for (; ls_it != calibrationSegments.end(); ls_it++)
      {
        std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator pts_it = (*ls_it).imgPts.begin();
        std::vector< double >::iterator alphas_it = (*ls_it).alphas.begin();
        for(; pts_it != (*ls_it).imgPts.end() && alphas_it != (*ls_it).alphas.end(); pts_it++, alphas_it++)
        {
          if (pts_it->second)
          {
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
  cov_corner_inv << 1 / additional_calibration_information->cov_corner_x->getDouble(), 0 , 0 , 1 / additional_calibration_information->cov_corner_y->getDouble();

  // Create and fill line segment measurement covariance matrix
  Eigen::Matrix2d cov_ls_inv;
  cov_ls_inv << 1 / additional_calibration_information->cov_ls_x->getDouble(), 0 , 0 , 1 / additional_calibration_information->cov_ls_y->getDouble();

  // Matrices for A, b and the Jacobian J
  Eigen::MatrixXd alpha(STATE_SPACE_DIMENSION + num_alpha, STATE_SPACE_DIMENSION + num_alpha);
  Eigen::VectorXd beta(STATE_SPACE_DIMENSION + num_alpha, 1);
  Eigen::MatrixXd J(2, STATE_SPACE_DIMENSION + num_alpha);

  bool stop_optimization(false);
  int convergence_counter(0);
  double t_start=GetTimeSec();
  while(!stop_optimization) {
    // Calculate Jacobi-Matrix, alpha and beta
    // Iterate over alle point pairs
    std::vector<GVector::vector3d<double> >::iterator it_p_f  = p_f.begin();
    std::vector<GVector::vector2d<double> >::iterator it_p_i  = p_i.begin();

    double epsilon = sqrt(std::numeric_limits<double>::epsilon());
    
    alpha.setZero();
    beta.setZero();
 
    for(; it_p_f != p_f.end(); it_p_f++, it_p_i++)
    {
      J.setZero();

      GVector::vector2d<double> proj_p;
      field2image(*it_p_f, proj_p, p);
      proj_p = proj_p - *it_p_i;

      std::vector<int>::iterator it = p_to_est.begin();
      for(; it != p_to_est.end(); it++)
      {
        int i = *it;

        Eigen::VectorXd p_diff = p;
        p_diff(i) = p_diff(i) + epsilon;

        GVector::vector2d<double> proj_p_diff;
        field2image(*it_p_f, proj_p_diff, p_diff);
        J(0,i) = ((proj_p_diff.x - (*it_p_i).x) - proj_p.x) / epsilon;
        J(1,i) = ((proj_p_diff.y - (*it_p_i).y) - proj_p.y) / epsilon;
      }
      
      alpha += J.transpose() * cov_corner_inv * J;
      beta += J.transpose() * cov_corner_inv * Eigen::Vector2d(proj_p.x, proj_p.y);
    }
    
    if (cal_type & FULL_ESTIMATION)
    {
      // First, calculate how many alpha we need to estimate
      std::vector<CalibrationData>::iterator ls_it = calibrationSegments.begin();
      
      int i = 0;
      for (; ls_it != calibrationSegments.end(); ls_it++)
      {
        std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator pts_it = (*ls_it).imgPts.begin();
        for(; pts_it != (*ls_it).imgPts.end(); pts_it++)
        {
          if(pts_it->second)
          { 
            GVector::vector2d<double> proj_p;
            GVector::vector3d<double >alpha_point;
            if(ls_it->straightLine){
              alpha_point = p_alpha(i) * (*ls_it).p1 + (1 - p_alpha(i)) * (*ls_it).p2;
            }else{
              double theta = p_alpha(i) * (*ls_it).theta1 + (1.0 - p_alpha(i)) * (*ls_it).theta2;
              alpha_point = ls_it->center + ls_it->radius*GVector::vector3d<double>(cos(theta),sin(theta),0.0);
            }
            field2image(alpha_point, proj_p, p);
            proj_p = proj_p - (*pts_it).first;
            
            J.setZero();
            
            std::vector<int>::iterator it = p_to_est.begin();
            for(; it != p_to_est.end(); it++)
            {
              int j = *it;
              
              Eigen::VectorXd p_diff = p;
              p_diff(j) = p_diff(j) + epsilon;
              
              GVector::vector2d<double> proj_p_diff;
              field2image(alpha_point, proj_p_diff, p_diff);
              J(0,j) = ((proj_p_diff.x - (*pts_it).first.x) - proj_p.x) / epsilon;
              J(1,j) = ((proj_p_diff.y - (*pts_it).first.y) - proj_p.y) / epsilon;
            }
            
            double my_alpha = p_alpha(i) + epsilon;
            if(ls_it->straightLine){
              alpha_point = my_alpha * (*ls_it).p1 + (1 - my_alpha) * (*ls_it).p2;
            }else{
              double theta = my_alpha * (*ls_it).theta1 + (1.0 - my_alpha) * (*ls_it).theta2;
              alpha_point = ls_it->center + ls_it->radius*GVector::vector3d<double>(cos(theta),sin(theta),0.0);
            }
            
            
            GVector::vector2d<double> proj_p_diff;
            field2image(alpha_point, proj_p_diff);
            J(0,STATE_SPACE_DIMENSION + i) = ((proj_p_diff.x - (*pts_it).first.x) - proj_p.x) / epsilon;
            J(1,STATE_SPACE_DIMENSION + i) = ((proj_p_diff.y - (*pts_it).first.y) - proj_p.y) / epsilon;
            
            alpha += J.transpose() * cov_ls_inv * J;
            beta += J.transpose() * cov_ls_inv * Eigen::Vector2d(proj_p.x, proj_p.y);
            i++;
          }
        }
      }    
    }
    
    // Augment alpha
    alpha += Eigen::MatrixXd::Identity(STATE_SPACE_DIMENSION + num_alpha, STATE_SPACE_DIMENSION + num_alpha) * lambda;

    // Solve for x
    Eigen::VectorXd new_p(STATE_SPACE_DIMENSION + num_alpha);
    
    // Due to an API change we need to check for
    // the right call at compile time
#ifdef EIGEN_WORLD_VERSION
    alpha.llt().solve(-beta, &new_p);
#else
    Eigen::Cholesky<Eigen::MatrixXd> c(alpha);
    new_p = c.solve(-beta);
#endif

    // Calculate chisqr again
    double chisqr = calc_chisqr(p_f, p_i, new_p, cal_type);

    if (chisqr < old_chisqr)
    {
      focal_length->setDouble(focal_length->getDouble() + new_p[FOCAL_LENGTH]);
      principal_point_x->setDouble(principal_point_x->getDouble() + new_p[PP_X]);
      principal_point_y->setDouble(principal_point_y->getDouble() + new_p[PP_Y]);
      distortion->setDouble(distortion->getDouble() + new_p[DIST]);
      tx->setDouble(tx->getDouble() + new_p[T_1]);
      ty->setDouble(ty->getDouble() + new_p[T_2]);
      tz->setDouble(tz->getDouble() + new_p[T_3]);

      Quaternion<double> q_diff;
      GVector::vector3d<double> aa_diff(new_p[Q_1], new_p[Q_2], new_p[Q_3]);
      q_diff.setAxis(aa_diff.norm(), aa_diff.length());
      Quaternion<double> q_field2cam = Quaternion<double>(q0->getDouble(),q1->getDouble(),q2->getDouble(),q3->getDouble());
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
      if (focal_length->getDouble() < 0)
      {
        focal_length->setDouble(-focal_length->getDouble());
        q_field2cam = q_rotate180 * q_field2cam;
        q0->setDouble(q_field2cam.x);
        q1->setDouble(q_field2cam.y);
        q2->setDouble(q_field2cam.z);
        q3->setDouble(q_field2cam.w);
      }
      
      if (old_chisqr - chisqr < 0.001)
        stop_optimization = true;
      else
      {
        lambda /= 10;
        convergence_counter = 0;
      }

      old_chisqr = chisqr;
#ifndef NDEBUG
      std::cerr << "Chi-square: "<< old_chisqr << std::endl;
#endif
    }
    else
    {
      lambda *= 10;
      
      if (convergence_counter++ > 10)
        stop_optimization = true;

    }
    if ((GetTimeSec() - t_start) > additional_calibration_information->convergence_timeout->getDouble()) stop_optimization=true;
  }
  
// Debug output starts here
#ifndef NDEBUG
  
  // Estimated parameters
  std::cerr << "Estimated parameters: " << std::endl;
  std::cerr << focal_length->getDouble() << " " << principal_point_x->getDouble() << " " << principal_point_y->getDouble() << " " << distortion->getDouble() << std::endl;
  std::cerr << q0->getDouble() << " " << q1->getDouble() << " " << q2->getDouble() << " " << q3->getDouble() << std::endl;
  std::cerr << tx->getDouble() << " " << ty->getDouble() << " " << tz->getDouble() <<  std::endl;
  std::cerr << "alphas: " << p_alpha << std::endl;

  // Testing calibration by projecting the four field points into the image plane
  // and calculate MSE
  std::vector<GVector::vector3d<double> >::iterator it_p_f  = p_f.begin();
  std::vector<GVector::vector2d<double> >::iterator it_p_i  = p_i.begin();
  
  double corner_x(0);
  double corner_y(0);
  for(; it_p_f != p_f.end(); it_p_f++, it_p_i++)
  {
    GVector::vector2d<double> proj_p;
    field2image(*it_p_f, proj_p);
    GVector::vector3d<double> some_point;
    image2field(some_point,proj_p, 150);
    std::cerr << "Point in world: ("<< it_p_f->x << "," << it_p_f->y << "," << it_p_f->z  << ")" << std::endl;
    std::cerr << "Point should be at (" << it_p_i->x << "," << it_p_i->y << ") and is projected at (" << proj_p.x << "," << proj_p.y <<")" << std::endl;
    
    corner_x += (proj_p.x - it_p_i->x) * (proj_p.x - it_p_i->x);
    corner_y += (proj_p.y - it_p_i->y) * (proj_p.y - it_p_i->y);
  }
  
  std::cerr << "RESIDUAL CORNER POINTS: " << sqrt(corner_x/4) << " " << sqrt(corner_y/4) << std::endl;

 if (cal_type & FULL_ESTIMATION)
 {
  // Testing calibration by projecting the points on the lines into the image plane
  // and calculate MSE
  double line_x(0);
  double line_y(0);

  std::vector<CalibrationData>::iterator ls_it = calibrationSegments.begin();

  int i = 0;
  for (; ls_it != calibrationSegments.end(); ls_it++)
  {
    std::vector< std::pair<GVector::vector2d<double>,bool> >::iterator pts_it = (*ls_it).imgPts.begin();
    for(; pts_it != (*ls_it).imgPts.end(); pts_it++)
    {
      if(pts_it->second)
      { 
        GVector::vector2d<double> proj_p; 
        double alpha = p_alpha(i);
        GVector::vector3d<double >alpha_point;
        if(ls_it->straightLine){
          alpha_point = alpha * (*ls_it).p1 + (1 - alpha) * (*ls_it).p2;
        }else{
          double theta = alpha * (*ls_it).theta1 + (1.0 - alpha) * (*ls_it).theta2;
          alpha_point = ls_it->center + ls_it->radius*GVector::vector3d<double>(cos(theta),sin(theta),0.0);
        }
        
        field2image(alpha_point, proj_p, p);
        
        line_x += (proj_p.x - pts_it->first.x) * (proj_p.x - pts_it->first.x);
        line_y += (proj_p.y - pts_it->first.y) * (proj_p.y - pts_it->first.y);
        
        i++;
      }
    }
  }
  
  if (i != 0)
    std::cerr << "RESIDUAL LINE POINTS: " << sqrt(line_x/(double)i) << " " << sqrt(line_y/(double)i) << std::endl;
 }
 
#endif
}


CameraParameters::AdditionalCalibrationInformation::AdditionalCalibrationInformation()
{
  left_corner_image_x = new VarDouble("left corner in image x", 10.0);
  left_corner_image_y = new VarDouble("left corner in image y", 10.0);
  right_corner_image_x = new VarDouble("right corner in image x", 300.0);
  right_corner_image_y = new VarDouble("right corner in image y", 10.0);
  left_centerline_image_x = new VarDouble("left centerline in image x", 10.0);
  left_centerline_image_y = new VarDouble("left centerline in image y", 200.0);
  right_centerline_image_x = new VarDouble("right centerline in image x", 300.0);
  right_centerline_image_y = new VarDouble("right centerline in image y", 200.0);
  initial_distortion = new VarDouble("initial distortion", 1.0);
  camera_height = new VarDouble("camera height", 4000.0);
  line_search_corridor_width = new VarDouble("line search corridor width", 280.0);
  convergence_timeout = new VarDouble("convergence timeout (s)", 10.0);
  cov_corner_x = new VarDouble("Cov corner measurement x", 1.0);
  cov_corner_y = new VarDouble("Cov corner measurement y", 1.0);
  cov_ls_x = new VarDouble("Cov line segment measurement x", 1.0);
  cov_ls_y = new VarDouble("Cov line segment measurement y", 1.0);
  pointsPerLine = new VarInt("Points per line",20);
  pointsInsideGoal = new VarInt("Points inside the goal",5);
  pointsInsideCenterCircle = new VarInt("Points inside center circle",6);
  pointsOnCenterCircle = new VarInt("Points on center circle",12);
  pointsOnDefenseAreaArc = new VarInt("Points on defense area arc",5);
  pointsOnDefenseStretch = new VarInt("Points on defense stretch",4);
}

CameraParameters::AdditionalCalibrationInformation::~AdditionalCalibrationInformation()
{
  delete left_corner_image_x;
  delete left_corner_image_y;
  delete right_corner_image_x;
  delete right_corner_image_y;
  delete left_centerline_image_x;
  delete left_centerline_image_y;
  delete right_centerline_image_x;
  delete right_centerline_image_y;
  delete initial_distortion;
  delete camera_height;
  delete line_search_corridor_width;
  delete convergence_timeout;
  delete cov_corner_x;
  delete cov_corner_y;
  delete cov_ls_x;
  delete cov_ls_y;
  delete pointsPerLine;
  delete pointsInsideGoal;
  delete pointsInsideCenterCircle;
  delete pointsOnCenterCircle;
  delete pointsOnDefenseAreaArc;
  delete pointsOnDefenseStretch;
}

void CameraParameters::AdditionalCalibrationInformation::addSettingsToList(VarList& list) 
{
  list.addChild(left_corner_image_x);
  list.addChild(left_corner_image_y);
  list.addChild(right_corner_image_x);
  list.addChild(right_corner_image_y);
  list.addChild(left_centerline_image_x);
  list.addChild(left_centerline_image_y);
  list.addChild(right_centerline_image_x);
  list.addChild(right_centerline_image_y);
  list.addChild(initial_distortion);
  list.addChild(camera_height);
  list.addChild(line_search_corridor_width);
  list.addChild(convergence_timeout);
  list.addChild(cov_corner_x);
  list.addChild(cov_corner_y);
  list.addChild(cov_ls_x);
  list.addChild(cov_ls_y);
  list.addChild(pointsPerLine);
  list.addChild(pointsInsideGoal);
  list.addChild(pointsInsideCenterCircle);
  list.addChild(pointsOnCenterCircle);
  list.addChild(pointsOnDefenseAreaArc);
  list.addChild(pointsOnDefenseStretch);
}

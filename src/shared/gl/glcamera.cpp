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
  \file    glcamera.cpp
  \brief   C++ Implementation: GLCamera
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "glcamera.h"

GLCamera::GLCamera()
{
  reset();
  pan_mode=GLCAM_PAN_POSITION_CENTRIC;
}

void GLCamera::reset() {
  GLObject::reset();
  forward.set(0.0,0.0,1.0);
  rot.clear();
}

GLCamera::GLCAM_PAN_MODE_ENUM GLCamera::getPanMode() {
  return pan_mode;
}

void GLCamera::setPanMode(GLCamera::GLCAM_PAN_MODE_ENUM mode) {
  pan_mode=mode;
}

void GLCamera::setDistance(double distance, bool allow_negative) {
  if (distance < 0.0 && allow_negative==false) distance=0.0;
  forward.z=distance;
}

double GLCamera::getDistance() {
  return forward.z;
}

quat GLCamera::getQuaternion() {
  return (rot);
}

void GLCamera::preRender() {
  double m[16];
  rot.getMatrix(m);

  //this pans in camera coordinates (centered around camera):
  //glTranslatef(0.0,0.0,-forward.z);
  glTranslated(-forward.x,-forward.y,-forward.z);
  
  glMultMatrixd(m);
}

vector3d GLCamera::getLensPosition() {
  vector3d result=pos;
  if (forward.sqlength() > 0.0) {
    result+=rot.rotateVectorByQuaternion(forward);
  }
  return (result);
}

void GLCamera::postRender() {
  glTranslatef(-pos.x,-pos.y,-pos.z);
}

void GLCamera::render() {

}

void GLCamera::apply() {
  //since this is a camera transformation that should affect
  //the entire scene we do not want QGLObject's standard apply implementation
  //because it would restore the old matrix when finished
  preRender();
  render();
  postRender();
}

GLCamera::~GLCamera()
{
}

void GLCamera::freeLookAt(const vector3d & point) {

  rot = shortestArc((vector3d(0.0,0.0,1.0)).norm(),(pos-point).norm());
  rot.invert();
}

void GLCamera::lookAt(const vector3d & point) {
  //1st compute yaw:
  vector3d diff=(pos-point);
  vector3d diff_flat(diff.x,0.0,diff.z);
  double yaw=atan2(diff.x,diff.z);

  double pitch=shortest_angle(diff,diff_flat);

  if (diff.y > 0.0) pitch*=(-1.0);

  rot.setEuler(pitch,yaw,0.0);
}



void GLCamera::setEuler(double pitch, double yaw, double roll) {
  if (pitch > M_PI_HALF) pitch=M_PI_HALF;
  if (pitch < -M_PI_HALF) pitch=-M_PI_HALF;
  rot.setEuler(pitch,yaw,roll);
}

void GLCamera::setRoll(double roll) {
  setEuler(rot.getPitch(),rot.getYaw(),roll);
}

void GLCamera::setPitch(double pitch) {
  setEuler(pitch,rot.getYaw(),rot.getRoll());
}

void GLCamera::setYaw(double yaw) {
  setEuler(rot.getPitch(),yaw,rot.getRoll());
}

void GLCamera::getEuler(double & pitch, double & yaw, double & roll) {
  rot.getEuler(pitch,yaw,roll);
}

double GLCamera::getRoll() {
  return rot.getRoll();

}

double GLCamera::getPitch() {
  return rot.getPitch();
}

double GLCamera::getYaw() {
  return rot.getYaw();
}


#ifndef NO_QT
//--------QT specific things go in here:

void GLCamera::wheelEvent ( QWheelEvent * event ) {
  int delta=event->delta();
  if (delta < 0) {
    setDistance(max(getDistance() + 0.05,getDistance()*1.15));
    //this->zoom.zoomIn();
  } else {
    setDistance(max(0.0,min(getDistance() - 0.05, getDistance()/1.15)));
    //this->zoom.zoomOut();
  }
  event->accept();
}
void GLCamera::keyPressEvent ( QKeyEvent * event ) {
  (void)event;
}

void GLCamera::mouseAction ( QMouseEvent * event ) {
  if ((event->buttons() & Qt::RightButton) != 0) {
    QPoint offset=mouseStart_pan-event->pos();
    if (pan_mode==GLCAM_PAN_POSITION_CENTRIC) {
      forward.x=(pan_start_x+(double)offset.x()*max(getDistance(),1.0)/700.0);
      forward.y=(pan_start_y-(double)offset.y()*max(getDistance(),1.0)/700.0);
    } else if (pan_mode==GLCAM_PAN_FREE_FLY) {
      vector3d trans;
      trans.x=((double)offset.x()/10.0);
      trans.y=(-(double)offset.y()/10.0);
      trans.z=0.0;
      trans=rot.rotateVectorByQuaternion(trans);
      pos=pan_start_pos+trans;
    }
  }
  if ((event->buttons() & Qt::LeftButton) != 0) {
    QPoint offset=mouseStart_rotate-event->pos();
    setEuler(p_start+(double)offset.y()/100.0,y_start+(double)offset.x()/100.0,r_start);
  }
}

void GLCamera::mousePressEvent ( QMouseEvent * event ) {
  
  if (event->button()==Qt::RightButton) {
    mouseStart_pan=event->pos();
    if (pan_mode==GLCAM_PAN_POSITION_CENTRIC) {
      pan_start_x=forward.x;
      pan_start_y=forward.y;
    } else if (pan_mode==GLCAM_PAN_FREE_FLY) {
      pan_start_pos=pos;
    }
  } else if (event->button()==Qt::LeftButton) {
    getEuler(p_start,y_start,r_start);
    mouseStart_rotate=event->pos();
  }
  
  mouseAction(event);
}
void GLCamera::mouseReleaseEvent ( QMouseEvent * event ) {
  mouseAction(event);
}
void GLCamera::mouseMoveEvent ( QMouseEvent * event ) {
  mouseAction(event);
}
#endif

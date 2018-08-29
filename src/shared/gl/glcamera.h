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
  \file    glcamera.h
  \brief   C++ Interface: GLCamera
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef GLCAMERA_H
#define GLCAMERA_H
#include "GL/gl.h"
#include "globject.h"
#include "geometry.h"
#include <cmath>
#ifndef NO_QT
  #include <QWheelEvent>
  #include <QKeyEvent>
#endif
#ifndef M_PI_HALF
  #define M_PI_HALF 1.570796326794896619231321691639751442098584699687552910487472296154
#endif
/*!
  \class  GLCamera
  \author Stefan Zickler
  \brief  An OpenGL camera class
*/
class GLCamera : public GLObject {
public:
enum GLCAM_PAN_MODE_ENUM {
  GLCAM_PAN_POSITION_CENTRIC,
  GLCAM_PAN_FREE_FLY,
};
protected:

    GLCAM_PAN_MODE_ENUM pan_mode;

public:

    vector3d forward;
    quat getQuaternion();

    GLCamera();

    ~GLCamera();
    void reset();
    //overloaded gl-object stuff
    public:
    virtual void apply();
    virtual void setState(const GLCamera & cam) {
      forward=cam.forward;
      pos=cam.pos;
      scale=cam.scale;
      rot=cam.rot;
    }
    protected:
    virtual void preRender();
    virtual void postRender();
    virtual void render();
    public:

    GLCAM_PAN_MODE_ENUM getPanMode();
    void setPanMode(GLCAM_PAN_MODE_ENUM mode);

    void setDistance(double distance, bool allow_negative=false);
    double getDistance();

    vector3d getLensPosition(); //computes the camera lenses position in world space

    void freeLookAt(const vector3d & point); //look at object including rolling
    void lookAt(const vector3d & point); //look at object with fixed roll

    void setEuler(double pitch, double yaw, double roll);
    void setRoll(double roll);
    void setPitch(double pitch);
    void setYaw(double yaw);

    void getEuler(double & pitch, double & yaw, double & roll);
    double getRoll();
    double getPitch();
    double getYaw();

    #ifndef NO_QT
    //some easily accessible event-handlers for camera orbiting using mouse/keyboard
    protected:
      QPoint mouseStart_rotate;
      QPoint mouseStart_pan;
      double p_start,y_start,r_start;
      double pan_start_x,pan_start_y;
      vector3d pan_start_pos;
      void mouseAction ( QMouseEvent * event );
    public:
      void wheelEvent ( QWheelEvent * event );
      void keyPressEvent ( QKeyEvent * event );
      void mousePressEvent ( QMouseEvent * event );
      void mouseReleaseEvent ( QMouseEvent * event );
      void mouseMoveEvent ( QMouseEvent * event );
    #endif
};


/*!
  \class  GLSmoothCamera
  \author Stefan Zickler
  \brief  A smoothly animated OpenGL camera class
*/
class GLSmoothCamera : public GLCamera {
  //this camera smoothly scrolls to a different camera target using a basic mass-spring system
private:
  vector3d v_forward;
  vector3d v_pos;
  double v_angle;
  bool prevent_overshoot;
  double k_acc;
  double k_damp;
public:
  GLCamera target;
  GLSmoothCamera(bool _prevent_overshoot=true, double k_acceleration=20.0, double k_damping=10.0) {
    k_acc=k_acceleration;
    k_damp=k_damping;
    v_forward.set(0.0,0.0,0.0);
    v_pos.set(0.0,0.0,0.0);
    v_angle = 0.0;
    prevent_overshoot=_prevent_overshoot;
  };
  virtual ~GLSmoothCamera() {};
  void step(const double dt) {
    //compute transition "delta" vectors:
    //this is the actual distance to the target
    vector3d d_forward = target.forward - forward;
    vector3d d_pos     = target.pos - pos;
    double   d_angle   = GVector::shortest_angle(rot.getZvector(),target.rot.getZvector());
    if (std::isnan(d_angle)) d_angle=0.0;
    //compute spring forces:

    vector3d f_forward = d_forward * k_acc - v_forward * k_damp;
    vector3d f_pos   =   d_pos * k_acc     - v_pos * k_damp;
    double   f_angle   = d_angle * k_acc   - v_angle * k_damp;

    //integrate:
    vector3d vnew_forward = v_forward + (f_forward * dt);
    vector3d vnew_pos     = v_pos     + (f_pos * dt);
    double   vnew_angle   = v_angle   + (f_angle * dt);

    //compute how far we will move (our 'applied' delta positions)
    vector3d delta_forward = (vnew_forward+v_forward) * 0.5 * dt;
    vector3d delta_pos     = (vnew_pos+v_pos) * 0.5 * dt;
    double   delta_angle   = (vnew_angle+v_angle) * 0.5 * dt;

    if (prevent_overshoot) {
      //hardcoded fix against overshooting: if applied deltas are bigger than target deltas, then shrink em
      //we will now never, ever overshoot...although it might be desirable if the target is moving
      if (delta_forward.length() > d_forward.length()) delta_forward=d_forward;
      if (delta_pos.length() > d_pos.length()) delta_pos=d_pos;
      if (delta_angle > d_angle) delta_angle=d_angle;
    }

    //add deltas to position.
    pos+=delta_pos;
    forward+=delta_forward;

    //perform angular blending:
    double blend_n=delta_angle / d_angle;

    if (blend_n==0.0 || d_angle==0.0 || delta_angle==0.0 || std::isnan(blend_n)) {
      //don't rotate!
    } else {
      rot.blend(blend_n,target.rot);
    }
    fflush(stdout);
    v_forward=vnew_forward;
    v_pos=vnew_pos;
    v_angle=vnew_angle;

  };

};


#endif

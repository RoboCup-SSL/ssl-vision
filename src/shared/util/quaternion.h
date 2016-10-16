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
  \file    quaternion.h
  \brief   A Quaternion class
  \author  Stefan Zickler, (C) 2007
*/
//========================================================================

/*!
  \class  Quaternion
  \brief   A Quaternion class
  \author Stefan Zickler, (C) 2007
          Some parts of this class were ported from
          the Cal3D library which is released under GPL.
          Some other parts taken from public domain listed here:
          http://gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
          and here:
          http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/index.htm
*/

#include <iostream>
#include <cmath>
#include "gvector.h"
#ifndef QUATERNION_H
#define QUATERNION_H
#ifndef PIOVER180
  #define PIOVER180 0.01745329251994329576923690768488612713442871888541725456097191440171
#endif

template <class num>
class Quaternion
{
public:
  num x;
  num y;
  num z;
  num w;

  // constructors/destructor
public:
  Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f){};
  Quaternion(const Quaternion<num>& q): x(q.x), y(q.y), z(q.z), w(q.w) {};
  Quaternion(num qx, num qy, num qz, num qw): x(qx), y(qy), z(qz), w(qw) {};
  ~Quaternion() {};

  // member functions
public:

  void operator=(const Quaternion<num> & q)
  {
    x = q.x;
    y = q.y;
    z = q.z;
    w = q.w;
  }

  void operator*=(const Quaternion<num> & q)
  {
    num qx, qy, qz, qw;
    qx = x;
    qy = y;
    qz = z;
    qw = w;

    x = qw * q.x + qx * q.w + qy * q.z - qz * q.y;
    y = qw * q.y - qx * q.z + qy * q.w + qz * q.x;
    z = qw * q.z + qx * q.y - qy * q.x + qz * q.w;
    w = qw * q.w - qx * q.x - qy * q.y - qz * q.z;
  }

  void operator*=(const GVector::vector3d<num> & v)
  {
    num qx, qy, qz, qw;
    qx = x;
    qy = y;
    qz = z;
    qw = w;

    x = qw * v.x            + qy * v.z - qz * v.y;
    y = qw * v.y - qx * v.z            + qz * v.x;
    z = qw * v.z + qx * v.y - qy * v.x;
    w =          - qx * v.x - qy * v.y - qz * v.z;
  }

  bool operator==(const Quaternion<num> & rhs) const
  {
    return x == rhs.x &&
           y == rhs.y &&
           z == rhs.z &&
           w == rhs.w;
  }

  bool operator!=(const Quaternion<num> & rhs) const
  {
    return !operator==(rhs);
  }

  void norm()
  {
    // Don't normalize if we don't have to
    num mag2 = w * w + x * x + y * y + z * z;
    if (fabs(mag2 - 1.0f) > 0.000001f) {
            num mag = sqrt(mag2);
            if(mag!=0.0) {
            w /= mag;
            x /= mag;
            y /= mag;
            z /= mag;
            }
    }
  }

  void blend(num d, const Quaternion<num> & q)
  {
    num norm;
    norm = x * q.x + y * q.y + z * q.z + w * q.w;

    bool bFlip;
    bFlip = false;

    if(norm < 0.0f) {
      norm = -norm;
      bFlip = true;
    }

    num inv_d;
    if(1.0f - norm < 0.000001f) {
      inv_d = 1.0f - d;
    } else {
      num theta;
      theta = (num) acos(norm);

      num s;
      s = (num) (1.0f / sin(theta));

      inv_d = (num) sin((1.0f - d) * theta) * s;
      d = (num) sin(d * theta) * s;
    }

    if(bFlip) {
      d = -d;
    }

    x = inv_d * x + d * q.x;
    y = inv_d * y + d * q.y;
    z = inv_d * z + d * q.z;
    w = inv_d * w + d * q.w;

  }

  void clear()
  {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 1.0f;
  }
  void conjugate()
  {
    x = -x;
    y = -y;
    z = -z;
  }

  void invert()
  {
    conjugate();
    const num norm = (x*x) + (y*y) + (z*z) + (w*w);

    if (norm == 0.0f) return;

    const num inv_norm = 1 / norm;
    x *= inv_norm;
    y *= inv_norm;
    z *= inv_norm;
    w *= inv_norm;
  }

  void set(num qx, num qy, num qz, num qw)
  {
    x = qx;
    y = qy;
    z = qz;
    w = qw;
  }
  
  // Convert from Axis Angle
  void setAxis(const GVector::vector3d<num> &v, num angle)
  {
    if (angle != .0f) {
      num sinAngle;
      angle *= 0.5f;
      GVector::vector3d<num> vn(v);
      vn.normalize();

      sinAngle = sin(angle);

      x = (vn.x * sinAngle);
      y = (vn.y * sinAngle);
      z = (vn.z * sinAngle);
      w = cos(angle);
    }
    else {
      x = .0f;
      y = .0f;
      z = .0f;
      w = 1.f;
    }    
  }

  GVector::vector3d<num> getZvector() {
    GVector::vector3d<num> v;
    v.set(0.0,0.0,1.0);
    return(rotateVectorByQuaternion(v));
  }

  GVector::vector3d<num> rotateVectorByQuaternion(const GVector::vector3d<num> v) const {
    num x2 = x * x;
    num y2 = y * y;
    num z2 = z * z;
    num xy = x * y;
    num xz = x * z;
    num yz = y * z;
    num wx = w * x;
    num wy = w * y;
    num wz = w * z;
    GVector::vector3d<num> res;
    res.x= (1.0f - 2.0f * (y2 + z2)) * v.x +  (2.0f * (xy - wz))        * v.y +  (2.0f * (xz + wy))        * v.z;
    res.y= (2.0f * (xy + wz)) * v.x        +  (1.0f - 2.0f * (x2 + z2)) * v.y +  (2.0f * (yz - wx))        * v.z;
    res.z= (2.0f * (xz - wy)) * v.x        +  (2.0f * (yz + wx))        * v.y +  (1.0f - 2.0f * (x2 + y2)) * v.z;
    return res;
  }


  // Convert to Matrix
  void getMatrix(num * m) const
  {

    num x2 = x * x;
    num y2 = y * y;
    num z2 = z * z;
    num xy = x * y;
    num xz = x * z;
    num yz = y * z;
    num wx = w * x;
    num wy = w * y;
    num wz = w * z;

    // This calculation would be a lot more complicated for non-unit length quaternions
    // Note: The constructor of Matrix4 expects the Matrix in column-major format like expected by
    //   OpenGL
    m[0]= 1.0f - 2.0f * (y2 + z2); m[1]= 2.0f * (xy - wz);        m[2]= 2.0f * (xz + wy);         m[3]=0.0f;
    m[4]= 2.0f * (xy + wz);        m[5]= 1.0f - 2.0f * (x2 + z2); m[6]= 2.0f * (yz - wx);         m[7]=0.0f;
    m[8]= 2.0f * (xz - wy);        m[9]= 2.0f * (yz + wx);        m[10]= 1.0f - 2.0f * (x2 + y2); m[11]=0.0f;
    m[12]=0.0f;                    m[13]=0.0f;                    m[14]= 0.0f;                    m[15]=1.0f;

     /*1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
                    2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
                    2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f};*/
  }

  // Convert to Axis/Angles
  void getAxisAngle(GVector::vector3d<num> &axis, num &angle)
  {
    num scale = sqrt(x * x + y * y + z * z);
    axis.x = x / scale;
    axis.y = y / scale;
    axis.z = z / scale;
    angle = acos(w) * 2.0f;
  }

  num getAngle()
  {
    return acos(w) * 2.0f;
  }

  void setEuler(num pitch, num yaw, num roll)
  {
    // Basically we create 3 Quaternions, one for pitch, one for yaw, one for roll
    // and multiply those together.
    // the calculation below does the same, just shorter

    double c1 = cos(yaw/2);
    double s1 = sin(yaw/2);
    double c2 = cos(roll/2);
    double s2 = sin(roll/2);
    double c3 = cos(pitch/2);
    double s3 = sin(pitch/2);
    double c1c2 = c1*c2;
    double s1s2 = s1*s2;
    w =c1c2*c3 - s1s2*s3;
    x =c1c2*s3 + s1s2*c3;
    y =s1*c2*c3 + c1*s2*s3;
    z =c1*s2*c3 - s1*c2*s3;

    norm();
  }

  inline double getYaw() { //heading
    if (fabs((x*y + z*w) - 0.5) <  0.000001f) { //it's north pole
        return 2 * atan2(x,w);
    }
    if (fabs((x*y + z*w) + 0.5) <  0.000001f) { //it's south pole
      return -2 * atan2(x,w);
    }

    return atan2(2* y*w-2*x*z , 1 - 2*(y*y) - 2*(z*z));
  }

  inline double getPitch() { //bank
    if (fabs((x*y + z*w) - 0.5) <  0.000001f) { //it's north pole
        return 0.0;
    }
    if (fabs((x*y + z*w) + 0.5) <  0.000001f) { //it's south pole
      return 0.0;
    }
    return atan2(2*x*w-2*y*z , 1 - 2*(x*x) - 2*(z*z));
  }

  inline double getRoll() { //attitude
      return asin(2*x*y + 2*z*w);
  }
  //            y           x          z
  void getEuler(num & pitch, num & yaw, num & roll ) {
    pitch=getPitch();
    yaw=getYaw();
    roll=getRoll();
  }
};

template <class num>
static Quaternion<num> operator*(const Quaternion<num>& q, const Quaternion<num>& r)
{
  return Quaternion<num>(
           r.w * q.x + r.x * q.w + r.y * q.z - r.z * q.y,
           r.w * q.y - r.x * q.z + r.y * q.w + r.z * q.x,
           r.w * q.z + r.x * q.y - r.y * q.x + r.z * q.w,
           r.w * q.w - r.x * q.x - r.y * q.y - r.z * q.z
         );
}

template <class num>
static Quaternion<num> shortestArc( const GVector::vector3d<num> & from, const GVector::vector3d<num> & to )
{
  GVector::vector3d<num> cross = from.cross(to); //Compute vector cross product
  num dot = from.dot(to) ;      //Compute dot product

  dot = (num) sqrt( 2*(dot+1) ) ; //We will use this equation twice

  cross /= dot ; //Get the x, y, z components

  if (std::isnan(cross.x) || std::isnan(cross.y) || std::isnan(cross.z) || std::isnan(-dot/2)) {
    printf("ERROR x: %f   y:%f   z: %f   w: %f\n",cross.x,cross.y,cross.z,-dot/2);
    fflush(stdout);
    exit(0);
  }
  //Return with the w component (Note that w is inverted because 3D has
  // left-handed rotations )
  return Quaternion<num>( cross.x, cross.y, cross.z, -dot/2 ) ;
}

#endif

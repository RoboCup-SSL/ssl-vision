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
  \file    gvector.h
  \brief   Simple vector class for 2D and 3D vectors
  \author  James R. Bruce <jbruce@cs.cmu.edu>, (C) 2004
*/
//========================================================================

#ifndef __GVECTOR_H__
#define __GVECTOR_H__

#include <math.h>
#include "util.h"

#define V3COMP(p) p.x,p.y,p.z
#define V2COMP(p) p.x,p.y
namespace GVector {

//=====================================================================//
//  Vector3D Class
//=====================================================================//

#define EPSILON (1.0E-10)

template <class num>
class vector3d{
public:
  num x,y,z;

  vector3d()
    {}
  vector3d(num nx,num ny,num nz)
    {x=nx; y=ny; z=nz;}

  void set(num nx,num ny,num nz)
    {x=nx; y=ny; z=nz;}
  void setAll(num nx)
    {x=y=z=nx;}
  void set(vector3d<num> p)
    {x=p.x; y=p.y; z=p.z;}
  void zero()
    {x=y=z=0;}

  vector3d<num> &operator=(const vector3d<num> p)
    {set(p); return(*this);}

  /// element accessor
  num &operator[](int idx)
    {return(((num*)this)[idx]);}
  const num &operator[](int idx) const
    {return(((num*)this)[idx]);}

  num length() const MustUseResult;
  num sqlength() const MustUseResult;
  vector3d<num> norm() const MustUseResult;
  vector3d<num> norm(const num len) const MustUseResult;
  void normalize();
  bool nonzero() const MustUseResult
    {return(x!=0 || y!=0 || z!=0);}

  num dot(const vector3d<num> p) const MustUseResult;
  vector3d<num> cross(const vector3d<num> p) const MustUseResult;

  vector3d<num> &operator+=(const vector3d<num> p);
  vector3d<num> &operator-=(const vector3d<num> p);
  vector3d<num> &operator*=(const vector3d<num> p);
  vector3d<num> &operator/=(const vector3d<num> p);

  vector3d<num> operator+(const vector3d<num> p) const;
  vector3d<num> operator-(const vector3d<num> p) const;
  vector3d<num> operator*(const vector3d<num> p) const;
  vector3d<num> operator/(const vector3d<num> p) const;

  vector3d<num> operator*(num f) const;
  vector3d<num> operator/(num f) const;
  vector3d<num> &operator*=(num f);
  vector3d<num> &operator/=(num f);

  vector3d<num> operator-() const;

  bool operator==(const vector3d<num> p) const;
  bool operator!=(const vector3d<num> p) const;
  bool operator< (const vector3d<num> p) const;
  bool operator> (const vector3d<num> p) const;
  bool operator<=(const vector3d<num> p) const;
  bool operator>=(const vector3d<num> p) const;

  vector3d<num> rotate(const vector3d<num> r,const double a) const;
  vector3d<num> rotate_x(const double a) const;
  vector3d<num> rotate_y(const double a) const;
  vector3d<num> rotate_z(const double a) const;

  //double shortest_angle(const vector3d<num> a,const vector3d<num> b);
  //vector3d<num> shortest_axis(const vector3d<num> a,const vector3d<num> b);

  bool finite() const MustUseResult
    {return(::finite(x) && ::finite(y) && ::finite(z));}

  void take_min(const vector3d<num> p);
  void take_max(const vector3d<num> p);
};

template <class num>
num vector3d<num>::length() const
{
  return(sqrt(x*x + y*y + z*z));
}

template <class num>
num vector3d<num>::sqlength() const
{
  return(x*x + y*y + z*z);
}

template <class num>
vector3d<num> vector3d<num>::norm() const
{
  vector3d<num> p;
  num l;

  l = sqrt(x*x + y*y + z*z);
  p.x = x / l;
  p.y = y / l;
  p.z = z / l;

  return(p);
}

template <class num>
vector3d<num> vector3d<num>::norm(const num len) const
{
  vector3d<num> p;
  num f;

  f = len / sqrt(x*x + y*y + z*z);
  p.x = x * f;
  p.y = y * f;
  p.z = z * f;

  return(p);
}

template <class num>
void vector3d<num>::normalize()
{
  num l;

  l = sqrt(x*x + y*y + z*z);
  x /= l;
  y /= l;
  z /= l;
}

template <class num>
num vector3d<num>::dot(const vector3d<num> p) const
{
  return(x*p.x + y*p.y + z*p.z);
}

template <class num>
num dot(const vector3d<num> a,const vector3d<num> b)
{
  return(a.x*b.x + a.y*b.y + a.z*b.z);
}

template <class num>
num absdot(const vector3d<num> a,const vector3d<num> b)
{
  return(fabs(a.x*b.x) + fabs(a.y*b.y) + fabs(a.z*b.z));
}

template <class num>
vector3d<num> vector3d<num>::cross(const vector3d<num> p) const
{
  vector3d<num> r;

  // right handed
  r.x = y*p.z - z*p.y;
  r.y = z*p.x - x*p.z;
  r.z = x*p.y - y*p.x;

  return(r);
}

template <class num>
vector3d<num> cross(const vector3d<num> a,const vector3d<num> b)
{
  vector3d<num> r;

  // right handed
  r.x = a.y*b.z - a.z*b.y;
  r.y = a.z*b.x - a.x*b.z;
  r.z = a.x*b.y - a.y*b.x;

  return(r);
}

#define VECTOR3D_EQUAL_BINARY_OPERATOR(opr) \
  template <class num> \
  vector3d<num> &vector3d<num>::operator opr (const vector3d<num> p) \
  {                  \
    x = x opr p.x;   \
    y = y opr p.y;   \
    z = z opr p.z;   \
    return(*this);   \
  }

VECTOR3D_EQUAL_BINARY_OPERATOR(+=)
VECTOR3D_EQUAL_BINARY_OPERATOR(-=)
VECTOR3D_EQUAL_BINARY_OPERATOR(*=)
VECTOR3D_EQUAL_BINARY_OPERATOR(/=)

#define VECTOR3D_BINARY_OPERATOR(opr) \
  template <class num> \
  vector3d<num> vector3d<num>::operator opr (const vector3d<num> p) const \
  {                  \
    vector3d<num> r; \
    r.x = x opr p.x; \
    r.y = y opr p.y; \
    r.z = z opr p.z; \
    return(r);       \
  }

VECTOR3D_BINARY_OPERATOR(+)
VECTOR3D_BINARY_OPERATOR(-)
VECTOR3D_BINARY_OPERATOR(*)
VECTOR3D_BINARY_OPERATOR(/)

#define VECTOR3D_SCALAR_OPERATOR(opr) \
  template <class num> \
  vector3d<num> vector3d<num>::operator opr (const num f) const \
  {                  \
    vector3d<num> r; \
    r.x = x opr f;   \
    r.y = y opr f;   \
    r.z = z opr f;   \
    return(r);       \
  }

VECTOR3D_SCALAR_OPERATOR(*)
VECTOR3D_SCALAR_OPERATOR(/)

#define VECTOR3D_EQUAL_SCALAR_OPERATOR(opr) \
  template <class num> \
  vector3d<num> &vector3d<num>::operator opr (num f) \
  {                \
    x = x opr f;   \
    y = y opr f;   \
    z = z opr f;   \
    return(*this); \
  }

VECTOR3D_EQUAL_SCALAR_OPERATOR(*=)
VECTOR3D_EQUAL_SCALAR_OPERATOR(/=)

#define VECTOR3D_LOGIC_OPERATOR(opr,combine) \
  template <class num> \
  bool vector3d<num>::operator opr (const vector3d<num> p) const \
  {                            \
    return((x opr p.x) combine \
           (y opr p.y) combine \
           (z opr p.z));       \
  }

VECTOR3D_LOGIC_OPERATOR(==,&&)
VECTOR3D_LOGIC_OPERATOR(!=,||)

VECTOR3D_LOGIC_OPERATOR(< ,&&)
VECTOR3D_LOGIC_OPERATOR(> ,&&)
VECTOR3D_LOGIC_OPERATOR(<=,&&)
VECTOR3D_LOGIC_OPERATOR(>=,&&)

template <class num>
vector3d<num> vector3d<num>::operator-() const
{
  vector3d<num> r;

  r.x = -x;
  r.y = -y;
  r.z = -z;

  return(r);
}

template <class num1,class num2>
vector3d<num2> operator*(num1 f,const vector3d<num2> &a)
{
  vector3d<num2> r;

  r.x = f * a.x;
  r.y = f * a.y;
  r.z = f * a.z;

  return(r);
}

template <class num>
inline vector3d<num> abs(vector3d<num> a)
{
  a.x = ::fabs(a.x);
  a.y = ::fabs(a.y);
  a.z = ::fabs(a.z);

  return(a);
}

template <class num>
inline vector3d<num> min(vector3d<num> a,vector3d<num> b)
{
  vector3d<num> v;

  v.x = ::min(a.x,b.x);
  v.y = ::min(a.y,b.y);
  v.z = ::min(a.z,b.z);

  return(v);
}

template <class num>
inline vector3d<num> max(vector3d<num> a,vector3d<num> b)
{
  vector3d<num> v;

  v.x = ::max(a.x,b.x);
  v.y = ::max(a.y,b.y);
  v.z = ::max(a.z,b.z);

  return(v);
}

template <class num>
inline vector3d<num> bound(vector3d<num> v,num low,num high)
{
  v.x = ::bound(v.x,low,high);
  v.y = ::bound(v.y,low,high);
  v.z = ::bound(v.z,low,high);

  return(v);
}

// returns point rotated around axis <r> by <a> radians (right handed)
template <class num>
vector3d<num> vector3d<num>::rotate(const vector3d<num> r,const double a) const
{
  vector3d<num> q;
  double s,c,t;

  s = sin(a);
  c = cos(a);
  t = 1 - c;

  q.x = (t * r.x * r.x + c      ) * x
      + (t * r.x * r.y - s * r.z) * y
      + (t * r.x * r.z + s * r.y) * z;

  q.y = (t * r.y * r.x + s * r.z) * x
      + (t * r.y * r.y + c      ) * y
      + (t * r.y * r.z - s * r.x) * z;

  q.z = (t * r.z * r.x - s * r.y) * x
      + (t * r.z * r.y + s * r.x) * y
      + (t * r.z * r.z + c      ) * z;

  return(q);
}

// returns point rotated around X axis by <a> radians (right handed)
template <class num>
vector3d<num> vector3d<num>::rotate_x(const double a) const
{
  vector3d<num> q;
  double s,c;

  s = sin(a);
  c = cos(a);

  q.x = x;
  q.y = c*y + -s*z;
  q.z = s*y + c*z;

  return(q);
}

// returns point rotated around Y axis by <a> radians (right handed)
template <class num>
vector3d<num> vector3d<num>::rotate_y(const double a) const
{
  vector3d<num> q;
  double s,c;

  s = sin(a);
  c = cos(a);

  q.x = c*x + s*z;
  q.y = y;
  q.z = -s*x + c*z;

  return(q);
}

// returns point rotated around Z axis by <a> radians (right handed)
template <class num>
vector3d<num> vector3d<num>::rotate_z(const double a) const
{
  vector3d<num> q;
  double s,c;

  s = sin(a);
  c = cos(a);

  q.x = c*x + -s*y;
  q.y = s*x + c*y;
  q.z = z;

  return(q);
}

template <class num>
double shortest_angle(const vector3d<num> a,const vector3d<num> b) {
  return(acos(std::max(-1.0,std::min(1.0,dot(a,b)/(a.length()*b.length())))));
}

template <class num>
vector3d<num> shortest_axis(const vector3d<num> a,const vector3d<num> b) {
  return(cross(a,b).norm());
}


// set the vector to the minimum of its components and p's components
template <class num>
void vector3d<num>::take_min(const vector3d<num> p)
{
  if(p.x < x) x = p.x;
  if(p.y < y) y = p.y;
  if(p.z < z) z = p.z;
}

// set the vector to the maximum of its components and p's components
template <class num>
void vector3d<num>::take_max(const vector3d<num> p)
{
  if(p.x > x) x = p.x;
  if(p.y > y) y = p.y;
  if(p.z > z) z = p.z;
}

// returns distance between two points
template <class num>
num dist(const vector3d<num> a,const vector3d<num> b)
{
  num dx,dy,dz;

  dx = a.x - b.x;
  dy = a.y - b.y;
  dz = a.z - b.z;

  return(sqrt(dx*dx + dy*dy + dz*dz));
}

template <class num>
inline num distance(const vector3d<num> a,const vector3d<num> b)
{
  return(dist(a,b));
}

// returns square of distance between two points
template <class num>
num sqdist(const vector3d<num> a,const vector3d<num> b)
{
  num dx,dy,dz;

  dx = a.x - b.x;
  dy = a.y - b.y;
  dz = a.z - b.z;

  return(dx*dx + dy*dy + dz*dz);
}

template <class num>
inline num sqdistance(const vector3d<num> a,const vector3d<num> b)
{
  return(sqdist(a,b));
}

// returns distance from point p to line x0-x1
template <class num>
num distance_to_line(const vector3d<num> x0,const vector3d<num> x1,const vector3d<num> p)
{
  // FIXME: this is probably broken
  vector3d<num> x;
  num t;

  t = ((p.x - x0.x) + (p.y - x0.y) + (p.z - x0.z)) / (x1.x + x1.y + x1.z);
  x = x0 + (x1 - x0) * t;

  return(distance(x,p));
}


//=====================================================================//
//  Vector2D Class
//=====================================================================//

template <class num>
class vector2d{
public:
  num x,y;

  vector2d()
    {}
  vector2d(num nx,num ny)
    {x=nx; y=ny;}

  /// set the components of the vector
  void set(num nx,num ny)
    {x=nx; y=ny;}
  /// set the components of the vector to the same value
  void setAll(num nx)
    {x=y=nx;}
  /// set the components of the vector
  void set(vector2d<num> p)
    {x=p.x; y=p.y;}
  /// zero all components of the vector
  void zero()
    {x=y=0;}

  /// copy constructor
  vector2d<num> &operator=(vector2d<num> p)
    {set(p); return(*this);}

  /// element accessor
  num &operator[](int idx)
    {return(((num*)this)[idx]);}
  const num &operator[](int idx) const
    {return(((num*)this)[idx]);}

  /// calculate Euclidean length
  num length() const MustUseResult;
  /// calculate squared Euclidean length (faster than length())
  num sqlength() const MustUseResult;
  /// calculate the clockwise angle from <1,0>
  num angle() const MustUseResult
    {return(atan2(y,x));}
  /// make a unit vector at given angle
  void heading(num angle)
    {x=cos(angle); y=sin(angle);}

  /// return a unit length vector in the same direction
  vector2d<num> norm() const MustUseResult;
  /// return a length 'len' vector in the same direction
  vector2d<num> norm(const num len) const MustUseResult;
  /// normalize to unit length in place
  void normalize();
  /// bound vector to a maximum length
  vector2d<num> bound(const num max_length) const MustUseResult;
  /// return if vector has any length at all
  bool nonzero() const MustUseResult
    {return(x!=0 || y!=0);}

  /// return dot product of vector with p
  num dot(const vector2d<num> p) const MustUseResult;
  /// return dot product of vector with p, equivalent to (this->perp()).dot(p)
  num perpdot(const vector2d<num> p) const MustUseResult;
  /// return z component of 3D cross product on 2D vectors.  right handed.
  num cross(const vector2d<num> p) const MustUseResult;

  /// return the perpendicular of a vector (i.e. rotated 90 deg clockwise)
  vector2d<num> perp() const MustUseResult
    {return(vector2d(-y, x));}

  /// add a vector to the current element values
  vector2d<num> &operator+=(const vector2d<num> p);
  /// subtract a vector from the current element values
  vector2d<num> &operator-=(const vector2d<num> p);
  /// multiply (elementwise) a vector with the current element values
  vector2d<num> &operator*=(const vector2d<num> p);
  /// divide (elementwise) a vector with the current element values
  vector2d<num> &operator/=(const vector2d<num> p);

  /// return vector sum of this vector and p
  vector2d<num> operator+(const vector2d<num> p) const;
  /// return vector difference of this vector and p
  vector2d<num> operator-(const vector2d<num> p) const;
  /// return elementwise product of this vector and p
  vector2d<num> operator*(const vector2d<num> p) const;
  /// return elementwise division of this vector by p
  vector2d<num> operator/(const vector2d<num> p) const;

  /// return this vector scaled by f
  vector2d<num> operator*(const num f) const;
  /// return this vector scaled by 1/f
  vector2d<num> operator/(const num f) const;
  /// scale this vector by f
  vector2d<num> &operator*=(num f);
  /// scale this vector by 1/f
  vector2d<num> &operator/=(num f);

  /// negate vector (reflect through origin) <x,y> -> <-x,-y>
  vector2d<num> operator-() const;

  bool operator==(const vector2d<num> p) const;
  bool operator!=(const vector2d<num> p) const;
  bool operator< (const vector2d<num> p) const;
  bool operator> (const vector2d<num> p) const;
  bool operator<=(const vector2d<num> p) const;
  bool operator>=(const vector2d<num> p) const;

  /// return vector rotated by angle a
  vector2d<num> rotate(const double a) const MustUseResult;

  // vector2d<num> project(const vector2d<num> p) const;
  vector2d<num> project_in(const vector2d<num> p) const MustUseResult;
  vector2d<num> project_out(const vector2d<num> p) const MustUseResult;

  /// return true if both elements are finite, otherwise return false
  bool finite() const MustUseResult
    {return(::finite(x) && ::finite(y));}

  /// set the vector to the minimum of its components and p's components
  void take_min(const vector2d<num> p);
  /// set the vector to the maximum of its components and p's components
  void take_max(const vector2d<num> p);
};

template <class num>
num vector2d<num>::length() const
{
  return(sqrt(x*x + y*y));
}

template <class num>
num vector2d<num>::sqlength() const
{
  return(x*x + y*y);
}

template <class num>
vector2d<num> vector2d<num>::norm() const
{
  vector2d<num> p;
  num l;

  l = sqrt(x*x + y*y);
  p.x = x / l;
  p.y = y / l;

  return(p);
}

template <class num>
vector2d<num> vector2d<num>::norm(const num len) const
{
  vector2d<num> p;
  num f;

  f = len / sqrt(x*x + y*y);
  p.x = x * f;
  p.y = y * f;

  return(p);
}

template <class num>
void vector2d<num>::normalize()
{
  num l;

  l = sqrt(x*x + y*y);
  x /= l;
  y /= l;
}

template <class num>
vector2d<num> vector2d<num>::bound(const num max_length) const
{
  vector2d<num> p;
  num lsq,f;

  lsq = x*x + y*y;

  if(lsq < sq(max_length)){
    p.set(x,y);
  }else{
    f = max_length / sqrt(lsq);
    p.set(f*x,f*y);
  }

  return(p);
}

template <class num>
num vector2d<num>::dot(const vector2d<num> p) const
{
  return(x*p.x + y*p.y);
}

template <class num>
num vector2d<num>::perpdot(const vector2d<num> p) const
// perp product, equivalent to (this->perp()).dot(p)
{
  return(x*p.y - y*p.x);
}

template <class num>
num dot(const vector2d<num> a,const vector2d<num> b)
{
  return(a.x*b.x + a.y*b.y);
}

template <class num>
num cosine(const vector2d<num> a,const vector2d<num> b)
// equivalent to dot(a.norm(),b.norm())
{
  num l;

  l = sqrt(a.x*a.x + a.y*a.y) * sqrt(b.x*b.x + b.y*b.y);

  return((a.x*b.x + a.y*b.y) / l);
}

template <class num>
num vector2d<num>::cross(const vector2d<num> p) const
{
  // right handed
  return(x*p.y - p.x*y);
}

// returns point rotated by <a> radians
template <class num>
vector2d<num> vector2d<num>::rotate(const double a) const
{
  vector2d<num> q;
  double s,c;

  s = sin(a);
  c = cos(a);

  q.x = c*x + -s*y;
  q.y = s*x + c*y;

  return(q);
}

/* Depricated: replace "p.project(basis)" with "basis.project_out(p)"
/// returns vector projected onto (p, p.perp()) basis.
/// equivalent to q = p*x + p.perp()*y;
template <class num>
vector2d<num> vector2d<num>::project(const vector2d<num> p) const
{
  vector2d<num> q;

  q.x = p.x*x - p.y*y;
  q.y = p.y*x + p.x*y;

  return(q);
}
*/

/// takes a vector p in outer coordinate space and returns one
/// projected onto basis given by this,this.perp()
template <class num>
vector2d<num> vector2d<num>::project_in(const vector2d<num> p) const
{
  vector2d<num> q;
  q.x = x*p.x + y*p.y; // q.x = this->dot(p);
  q.y = x*p.y - y*p.x; // q.y = this->perpdot(p);
  return(q);
}

/// takes a vector p in basis given by this,this.perp() and returns
/// one in the outer coordinate space
template <class num>
vector2d<num> vector2d<num>::project_out(const vector2d<num> p) const
{
  vector2d<num> q;
  q.x = x*p.x - y*p.y;
  q.y = y*p.x + x*p.y;
  return(q);
}

#define VECTOR2D_EQUAL_BINARY_OPERATOR(opr) \
  template <class num> \
  vector2d<num> &vector2d<num>::operator opr (const vector2d<num> p) \
  {                  \
    x = x opr p.x;   \
    y = y opr p.y;   \
    return(*this);   \
  }

VECTOR2D_EQUAL_BINARY_OPERATOR(+=)
VECTOR2D_EQUAL_BINARY_OPERATOR(-=)
VECTOR2D_EQUAL_BINARY_OPERATOR(*=)
VECTOR2D_EQUAL_BINARY_OPERATOR(/=)

#define VECTOR2D_BINARY_OPERATOR(opr) \
  template <class num> \
  vector2d<num> vector2d<num>::operator opr (const vector2d<num> p) const \
  {                  \
    vector2d<num> r; \
    r.x = x opr p.x; \
    r.y = y opr p.y; \
    return(r);       \
  }

VECTOR2D_BINARY_OPERATOR(+)
VECTOR2D_BINARY_OPERATOR(-)
VECTOR2D_BINARY_OPERATOR(*)
VECTOR2D_BINARY_OPERATOR(/)

#define VECTOR2D_SCALAR_OPERATOR(opr) \
  template <class num> \
  vector2d<num> vector2d<num>::operator opr (const num f) const \
  {                  \
    vector2d<num> r;  \
    r.x = x opr f;   \
    r.y = y opr f;   \
    return(r);       \
  }

VECTOR2D_SCALAR_OPERATOR(*)
VECTOR2D_SCALAR_OPERATOR(/)

#define VECTOR2D_EQUAL_SCALAR_OPERATOR(opr) \
  template <class num> \
  vector2d<num> &vector2d<num>::operator opr (num f) \
  {                \
    x = x opr f;   \
    y = y opr f;   \
    return(*this); \
  }

VECTOR2D_EQUAL_SCALAR_OPERATOR(*=)
VECTOR2D_EQUAL_SCALAR_OPERATOR(/=)

#define VECTOR2D_LOGIC_OPERATOR(opr,combine) \
  template <class num> \
  bool vector2d<num>::operator opr (const vector2d<num> p) const \
  {                            \
    return((x opr p.x) combine \
           (y opr p.y));       \
  }

VECTOR2D_LOGIC_OPERATOR(==,&&)
VECTOR2D_LOGIC_OPERATOR(!=,||)

VECTOR2D_LOGIC_OPERATOR(< ,&&)
VECTOR2D_LOGIC_OPERATOR(> ,&&)
VECTOR2D_LOGIC_OPERATOR(<=,&&)
VECTOR2D_LOGIC_OPERATOR(>=,&&)


template <class num>
vector2d<num> vector2d<num>::operator-() const
{
  vector2d<num> r;
  r.x = -x;
  r.y = -y;
  return(r);
}

template <class num1,class num2>
vector2d<num2> operator*(num1 f,const vector2d<num2> &a)
{
  vector2d<num2> r;

  r.x = f * a.x;
  r.y = f * a.y;

  return(r);
}

template <class num>
void vector2d<num>::take_min(const vector2d<num> p)
{
  if(p.x < x) x = p.x;
  if(p.y < y) y = p.y;
}

template <class num>
void vector2d<num>::take_max(const vector2d<num> p)
{
  if(p.x > x) x = p.x;
  if(p.y > y) y = p.y;
}

template <class num>
inline vector2d<num> abs(vector2d<num> a)
{
  a.x = ::fabs(a.x);
  a.y = ::fabs(a.y);

  return(a);
}

template <class num>
inline vector2d<num> min(vector2d<num> a,vector2d<num> b)
{
  vector2d<num> v;

  v.x = ::min(a.x,b.x);
  v.y = ::min(a.y,b.y);

  return(v);
}

template <class num>
inline vector2d<num> max(vector2d<num> a,vector2d<num> b)
{
  vector2d<num> v;

  v.x = ::max(a.x,b.x);
  v.y = ::max(a.y,b.y);

  return(v);
}

template <class num>
inline vector2d<num> bound(vector2d<num> v,num low,num high)
{
  v.x = ::bound(v.x,low,high);
  v.y = ::bound(v.y,low,high);

  return(v);
}

template <class num>
num dist(const vector2d<num> a,const vector2d<num> b)
{
  num dx,dy;

  dx = a.x - b.x;
  dy = a.y - b.y;

  return(sqrt(dx*dx + dy*dy));
}

template <class num>
inline num distance(const vector2d<num> a,const vector2d<num> b)
{
  return(dist(a,b));
}

// returns square of distance between two points
template <class num>
num sqdist(const vector2d<num> a,const vector2d<num> b)
{
  num dx,dy;

  dx = a.x - b.x;
  dy = a.y - b.y;

  return(dx*dx + dy*dy);
}

template <class num>
inline num sqdistance(const vector2d<num> a,const vector2d<num> b)
{
  return(sqdist(a,b));
}

}; // namespace vector

#endif
// __VECTOR_H__

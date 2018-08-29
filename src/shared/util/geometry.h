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
  \file    geometry.h
  \brief   Meta-header to include all significant geometry-related classes
  \author  James R. Bruce <jbruce@cs.cmu.edu>, (C) 2007
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#define USE_QUATERIONS        1
#define USE_MATRIX_FIXED_SIZE 0
#define USE_POSE              1

//==== Vector types ====//

#include "gvector.h"
#include "geomalgo.h"

typedef GVector::vector2d<double> vector2d;
typedef GVector::vector3d<double> vector3d;

typedef GVector::vector2d<float> vector2f;
typedef GVector::vector3d<float> vector3f;

typedef GVector::vector2d<int> vector2i;
typedef GVector::vector3d<int> vector3i;

struct vector2s{
  int16_t x,y;
};

struct vector3s{
  int16_t x,y,z;
};

//==== Bounding boxes ====//

#include "bbox.h"

typedef BBox::BBox2D<vector2d,double> BBox2d;
typedef BBox::BBox3D<vector3d,double> BBox3d;

typedef BBox::BBox2D<vector2f,float> BBox2f;
typedef BBox::BBox3D<vector3f,float> BBox3f;

//==== Range Type ====//

#include "range.h"

typedef Range::Range<float ,false,true> RangeFloat;
typedef Range::Range<double,false,true> RangeDouble;
typedef Range::Range<int   ,false,true> RangeInt;

typedef Range::Range<float ,false,false> ClosedRangeFloat;
typedef Range::Range<double,false,false> ClosedRangeDouble;
typedef Range::Range<int   ,false,false> ClosedRangeInt;

//==== Angle and matrix types ====//

#if USE_QUATERIONS
#include "quaternion.h"
typedef Quaternion<double> quat;
typedef Quaternion<float> quatf;
#endif

#if USE_MATRIX_FIXED_SIZE
#include "matrix_fixed.h"
typedef Mat3x3<double> mat3x3d;
typedef Mat3x3<float> mat3x3f;
#endif

//==== Other geometric types ====//

#if USE_POSE
#include "pose.h"
typedef DynamicPose2D<vector2f,float > DPose2f;
typedef DynamicPose2D<vector2d,double> DPose2d;
#endif

//==== Some vector conversion functions ====//

template <class vector_a,class vector_b>
void vcopy2d(vector_a &dest,const vector_b &src)
{
  dest.x = src.x;
  dest.y = src.y;
}

template <class vector_a,class vector_b>
void vcopy3d(vector_a &dest,const vector_b &src)
{
  dest.x = src.x;
  dest.y = src.y;
  dest.z = src.z;
}

template <class vec_in>
inline vector2f vec2f(const vec_in &p)
{
  return(vector2f(p.x,p.y));
}

template <class vec_in>
inline vector2d vec2d(const vec_in &p)
{
  return(vector2d(p.x,p.y));
}

template <class vec_in>
inline vector3f vec3f(const vec_in &p)
{
  return(vector3f(p.x,p.y,p.z));
}

template <class vec_in>
inline vector3d vec3d(const vec_in &p)
{
  return(vector3d(p.x,p.y,p.z));
}

template <class vec_in>
inline vector2s vec2s(const vec_in &p)
{
  vector2s vs;
  vs.x = (int)rint(p.x);
  vs.y = (int)rint(p.y);
  return(vs);
}

template <class vec_in>
inline vector3s vec3s(const vec_in &p)
{
  vector3s vs;
  vs.x = (int)rint(p.x);
  vs.y = (int)rint(p.y);
  vs.z = (int)rint(p.z);
  return(vs);
}

//==== Angle conversion, and normally missing constant(s) ====//

#define RAD(deg) ((deg) * (M_PI / 180.0)) /* convert radians to degrees */
#define DEG(rad) ((rad) * (180.0 / M_PI)) /* convert degrees to radians */

#ifndef HUGE_VALF
#define HUGE_VALF (1E37)
#endif

#endif // __GEOMETRY_H__

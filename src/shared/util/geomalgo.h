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
  \file    geomalgo.h
  \brief   A collection of geometry algorithms
  \author  James R. Bruce <jbruce@cs.cmu.edu>, (C) 2007
*/
//========================================================================

#ifndef __GEOM_ALGO_H__
#define __GEOM_ALGO_H__

#include <stdio.h>
#include "gvector.h"

namespace GVector {

// returns distance from point p to line x0-x1
template <class num>
num distance_to_line(const vector2d<num> x0,const vector2d<num> x1,const vector2d<num> p)
{
  // dot with unit length normal to line
  return(fabs((x1 - x0).norm().perpdot(p - x0)));
}

// returns perpendicular offset from line x0-x1 to point p
template <class num>
num offset_to_line(const vector2d<num> x0,const vector2d<num> x1,const vector2d<num> p)
{
  vector2d<num> n;

  // get normal to line
  n = (x1 - x0).perp().norm();

  return(n.dot(p - x0));
}

// returns perpendicular offset from line x0-x1 to point p
template <class num>
num offset_along_line(const vector2d<num> x0,const vector2d<num> x1,const vector2d<num> p)
{
  vector2d<num> n,v;

  // get normal to line
  n = x1 - x0;
  n.normalize();

  v = p - x0;

  return(n.dot(v));
}

// returns nearest point on segment a0-a1 to line b0-b1
template <class num>
vector2d<num> segment_near_line(const vector2d<num> a0,const vector2d<num> a1,
                                const vector2d<num> b0,const vector2d<num> b1)
{
  vector2d<num> v,n,p;
  double dn,t;

  v = a1-a0;
  n = (b1-b0).norm();
  n.set(-n.y,n.x);

  dn = dot(v,n);
  if(fabs(dn) < EPSILON) return(a0);

  t = -dot(a0-b0,n) / dn;
  // printf("t=%f dn=%f\n",t,dn);
  if(t < 0) t = 0;
  if(t > 1) t = 1;
  p = a0 + v*t;

  return(p);
}

//
template <class num>
vector2d<num> intersection(const vector2d<num> a1, const vector2d<num> a2,
                           const vector2d<num> b1, const vector2d<num> b2)
{
  vector2d<num> a = a2 - a1;

  vector2d<num> b1r = (b1 - a1).rotate(-a.angle());
  vector2d<num> b2r = (b2 - a1).rotate(-a.angle());
  vector2d<num> br = (b1r - b2r);

  return 
    vector2d<num>(b2r.x - b2r.y * (br.x / br.y), 0.0).rotate(a.angle()) + a1;
}

// gives counterclockwise angle from <a-b> to <c-b>
template <class num>
num vertex_angle(const vector2d<num> a,
                 const vector2d<num> b,
                 const vector2d<num> c)
{
  return(angle_mod((a-b).angle() - (c-b).angle()));
}

// calc_circle
template <class num>
bool CalcCircle(vector2d<num> &cen,num &rad,
                const vector2d<num> p1,
                const vector2d<num> p2,
                const vector2d<num> p3)
{
  vector2d<num> a,b;
  num da,db;

  // edge vectors
  a = (p2 - p1);
  b = (p3 - p2);

  // dot products of midpoint of each edge vector
  da = a.dot(p1 + p2) * 0.5;
  db = b.dot(p2 + p3) * 0.5;

  /*
  We want to find a point c on each line, so:
    [ a.x a.y ] [c.x] = [da]
    [ b.x b.y ] [c.y]   [db]

  We can invert the matrix to do that, and multiply it on the left of both
  side.  That yeilds a direct expression for center point c.
  */

  num det = a.x * b.y - a.y * b.x;
  if(fabs(det) < EPSILON) return(false);

  cen.x = ( b.y*da + -a.y*db) / det;
  cen.y = (-b.x*da +  a.x*db) / det;

  // use averaging to get a more numerically accurate radius
  // rad = sqrt((sqdist(cen,p1) + sqdist(cen,p2) + sqdist(cen,p3)) / 3.0);
  rad = dist(cen,p1);

  if(false){
    printf("%f %f, %f %f\n",
           a.dot(cen),da,
           b.dot(cen),db);
  }

  return(true);
}

//==== Generic functions =============================================//
// (work on 2d or 3d vectors)

template <class vector>
vector interpolate(const vector x0,const vector x1,double t)
{
  // t = bound(t, 0.0, 1.0);
  return(x0 + (x1-x0)*t);
}

template <class vector>
vector point_on_line(const vector x0,const vector x1,const vector p)
// returns nearest point on line through x0-x1 to point p
// Preconditions: x0!=x1
{
  vector sx,sp,r;
  double f,l;

  sx = x1 - x0;
  sp = p  - x0;

  f = dot(sx,sp);
  l = sx.sqlength();

  // if line is degenerate, any point will do
  if(l < EPSILON) return(p);

  // calculate point along line nearest to p
  r = x0 + sx * (f / l);
  return(r);
}

template <class vector>
vector point_on_segment(const vector x0,const vector x1,const vector p)
// returns nearest point on line segment x0-x1 to point p
{
  vector sx,sp,r;
  double f,l;

  sx = x1 - x0;
  sp = p  - x0;

  f = dot(sx,sp);
  if(f <= 0.0) return(x0); // also handles x0=x1 case

  l = sx.sqlength();
  if(f >= l) return(x1);

  r = x0 + sx * (f / l);

  return(r);
}

// returns shortest distance between line segment x0-x1 to point p
template <class vector>
double distance_to_segment(const vector x0,const vector x1,const vector p)
{
  vector sx,sp,r;
  double f,l;

  sx = x1 - x0;
  sp = p  - x0;

  f = dot(sx,sp);
  if(f <= 0.0) return(distance(x0,p)); // also handles x0=x1 case

  l = sx.sqlength();
  if(f >= l) return(distance(x1,p));

  r = x0 + sx * (f / l);

  return(distance(r,p));
}

template <class vector>
double closest_point_time(const vector x1,const vector v1,
                          const vector x2,const vector v2)
// returns time of closest point of approach of two points
// moving along constant velocity vectors.
{
  vector v  = v1 - v2;
  double sl = v.sqlength();
  double t;

  if(sl < EPSILON) return(0.0); // parallel tracks, any time is ok.

  t = -v.dot(x1 - x2) / sl;
  if(t < 0.0) return(0.0); // nearest time was in the past, now
                           // is closest point from now on.

  return(t);
}

// Ported from: dist3D_Segment_to_Segment
//   from http://geometryalgorithms.com
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose providing
// that this copyright notice is included with it.  SoftSurfer makes
// no warranty for this code, and cannot be held liable for any real
// or imagined damage resulting from its use.  Users of this code must
// verify correctness for their application.
template <class vector>
double distance_seg_to_seg(vector s1a,vector s1b,vector s2a,vector s2b)
// return distnace between segments s1a-s1b and s2a-s2b
{
  vector dp;
  vector u = s1b - s1a;
  vector v = s2b - s2a;
  vector w = s1a - s2a;
  float a = dot(u,u);        // always >= 0
  float b = dot(u,v);
  float c = dot(v,v);        // always >= 0
  float d = dot(u,w);
  float e = dot(v,w);
  float D = a*c - b*b;       // always >= 0
  float sc, sN, sD = D;      // sc = sN / sD, default sD = D >= 0
  float tc, tN, tD = D;      // tc = tN / tD, default tD = D >= 0

  if(true){
    printf("SegDist (%f,%f)-(%f,%f) to (%f,%f)-(%f,%f) a=%f b=%f\n",
           V2COMP(s1a),V2COMP(s1b),V2COMP(s2a),V2COMP(s2b),
           a,b);
  }

  if((a < EPSILON) || (b < EPSILON)){
    if((a < EPSILON) && (b < EPSILON)){
      return(dist(s1a,s2a));
    }else if(a < EPSILON){
      return(distance_to_segment(s2a,s2b,s1a));
    }else{
      return(distance_to_segment(s1a,s1b,s2a));
    }
  }

  // compute the line parameters of the two closest points
  if(D < EPSILON){ // the lines are almost parallel
    sN = 0.0; // force using point P0 on segment S1
    sD = 1.0; // to prevent possible division by 0.0 later
    tN = e;
    tD = c;
  }else{                // get the closest points on the infinite lines
    sN = (b*e - c*d);
    tN = (a*e - b*d);
    if(sN < 0){         // sc < 0 => the s=0 edge is visible
      sN = 0.0;
      tN = e;
      tD = c;
    }else if(sN > sD){  // sc > 1 => the s=1 edge is visible
      sN = sD;
      tN = e + b;
      tD = c;
    }
  }

  if(tN < 0){           // tc < 0 => the t=0 edge is visible
    tN = 0.0;
    // recompute sc for this edge
    if(-d < 0){
      sN = 0.0;
    }else if(-d > a){
      sN = sD;
    }else{
      sN = -d;
      sD = a;
    }
  }else if(tN > tD){      // tc > 1 => the t=1 edge is visible
    tN = tD;
    // recompute sc for this edge
    if((-d + b) < 0){
      sN = 0;
    }else if((-d + b) > a){
      sN = sD;
    }else{
      sN = (-d + b);
      sD = a;
    }
  }

  if(false){
    if(sD<1E-9 || tD<1E-9){
      printf("seg_seg_degen D=%f sD=%f tD=%f\n",D,sD,tD);
      printf("  SegDist (%f,%f)-(%f,%f) to (%f,%f)-(%f,%f)\n",
             V2COMP(s1a),V2COMP(s1b),V2COMP(s2a),V2COMP(s2b));
    }
  }

  // finally do the division to get sc and tc
  // sc = sN / sD;
  // tc = tN / tD;
  sc = (fabs(sN) < EPSILON)? 0.0 : sN / sD;
  tc = (fabs(tN) < EPSILON)? 0.0 : tN / tD;

  // get the difference of the two closest points
  dp = w + u*sc - v*tc; // = S1(sc) - S2(tc)

  return(dp.length()); // return the closest distance
}

// Inputs: plane origin, plane normal, ray origin ray vector.
// NOTE: both vectors are assumed to be normalized
template <class vector>
double ray_plane_intersect(vector pOrigin,vector pNormal,
                           vector rOrigin,vector rVector)
{
  return(dot(-pNormal,(rOrigin - pOrigin)) / (dot(pNormal,rVector)));

//  return((dot(pNormal,pOrigin) - dot(pNormal,rOrigin)) /
//         dot(pNormal,rVector));
  
//  double numer = dot(pNormal,rOrigin) - dot(pNormal,pOrigin);
//  double denom = dot(pNormal,rVector);
//  return(-(numer / denom));
  
}

template <class vector, class real>
real ray_sphere_intersect(vector rO, vector rV,vector sO,real sR)
// intersect a ray (r0+rV*t) with a sphere at s0 with radius sR
// returns: t, or -1 if no intersection
// NOTE: rV is assumed to be normalized
{
  vector Q = sO - rO;
  real c = Q.length();
  real v = dot(Q,rV);
  real d = sR*sR - (c*c - v*v);

  // If there was no intersection, return -1
  if(d < 0.0) return(-1.0);

  // Return the distance to the [first] intersecting point
  return(v - sqrt(d));
}

template <class vector, class real>
bool CircleTangentDir(vector cen,real rad,vector p,
                      vector &left,vector &right)
{
  vector delta,basis;
  vector tl,tr;
  double d2,l,t;

  // l,r,d form a right triangle where l*l + rad*rad = d*d = d2
  delta = cen - p;
  d2 = delta.sqlength();

  t = d2 - rad*rad;
  if(t < 0) return(false); // inside circle

  // set basis as sin/d,cos/d
  l = sqrt(t);
  basis.set(l/d2,rad/d2);

  // project to get left and right tangents
  // resulting vectors are normalized
  left = basis.project_out(delta);
  basis.y = -basis.y;
  right = basis.project_out(delta);

  return(true);
}

template <class vector, class real>
bool CircleTangent(vector cen,real rad,vector p,
                   vector &left,vector &right)
{
  vector delta,basis;
  vector tl,tr;
  double d2,l,t;

  // l,r,d form a right triangle where l*l + rad*rad = d*d = d2
  delta = cen - p;
  d2 = delta.sqlength();

  t = d2 - rad*rad;
  if(t < 0) return(false); // inside circle

  // set basis as sin/d,cos/d
  l = sqrt(t);
  basis.set(l/d2,rad/d2);

  // project to get left and right tangents
  // resulting vectors are normalized
  left  = basis.project_out(delta)*l;
  basis.y = -basis.y;
  right = basis.project_out(delta)*l;

  return(true);
}

// calculate the unit vector pointing at the "midpoint" of a line
// based on splitting the angle from the origin.  The unit vector is
// returned, while the distance is stored in <dist>.  dir*dist lies on
// the line p0-p1.
template <class vector, class real>
vector LineMidpointAngular(const vector &p0,const vector &p1,real &dist)
{
  vector dir = (p0.norm() + p1.norm()).norm();
  vector pn = (p1 - p0).norm();
  dist = pn.perpdot(p0) / pn.perpdot(dir);
  return(dir);
}

}; // namespace

#endif

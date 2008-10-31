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
  \file    bbox.h
  \brief   Classes and data-structures representing bounding boxes
  \author  James R. Bruce <jbruce@cs.cmu.edu>, (C) 2007
*/
//========================================================================

#ifndef _INCLUDED_BBOX_H_
#define _INCLUDED_BBOX_H_

namespace BBox {

#define BBOX_TEM  template <class vector,class num>
#define BBOX3D_FUN BBox3D<vector,num>
#define BBOX2D_FUN BBox2D<vector,num>

BBOX_TEM
struct BBox3D{
public:
  vector cen; // center
  vector rad; // extents
public:
  bool check(const BBox3D &b) const;
  bool inside(const vector &p) const;

  void zero()
    {cen.zero(); rad.zero();}
  void set(const vector &_cen,const vector &_rad)
    {cen=_cen; rad=_rad;}
  void set(const vector &_cen,num _rad)
    {cen=_cen; rad.set(_rad,_rad,_rad);}
  void set(const vector &p0,const vector &p1,num _rad);

  void expand(num amt);
  void add(const BBox3D &b);
  void add(vector cen,num rad);
  void merge(const BBox3D &a,const BBox3D &b);
};

BBOX_TEM
void BBOX3D_FUN::set(const vector &p0,const vector &p1,num _rad)
{
  cen = (p0 + p1)*0.5;
  rad.set(fabs(p1.x - p0.x)*0.5 + _rad,
          fabs(p1.y - p0.y)*0.5 + _rad,
          fabs(p1.z - p0.z)*0.5 + _rad);
}

BBOX_TEM
bool BBOX3D_FUN::check(const BBox3D &b) const
{
  return(fabs(b.cen.x - cen.x) > (b.rad.x + rad.x) ||
         fabs(b.cen.y - cen.y) > (b.rad.y + rad.y) ||
         fabs(b.cen.z - cen.z) > (b.rad.z + rad.z));
}

BBOX_TEM
bool BBOX3D_FUN::inside(const vector &p) const
{
  return(fabs(p.x - cen.x) < rad.x &&
         fabs(p.y - cen.y) < rad.y &&
         fabs(p.z - cen.z) < rad.z);
}

BBOX_TEM
void BBOX3D_FUN::expand(num amt)
{
  rad.x += amt;
  rad.y += amt;
  rad.z += amt;
}

BBOX_TEM
void BBOX3D_FUN::add(const BBox3D &b)
{
  vector p0 = min(cen-rad, b.cen-b.rad);
  vector p1 = max(cen+rad, b.cen+b.rad);

  cen = (p1 + p0)*0.5;
  rad = (p1 - p0)*0.5;
}

BBOX_TEM
void BBOX3D_FUN::add(vector center,num radius)
{
  vector rv(radius,radius,radius);
  vector p0 = min(cen-rad, center-rv);
  vector p1 = max(cen+rad, center+rv);

  cen = (p1 + p0)*0.5;
  rad = (p1 - p0)*0.5;
}

BBOX_TEM
void BBOX3D_FUN::merge(const BBox3D &a,const BBox3D &b)
{
  vector p0 = min(a.cen-a.rad, b.cen-b.rad);
  vector p1 = max(a.cen+a.rad, b.cen+b.rad);

  cen = (p1 + p0)*0.5;
  rad = (p1 - p0)*0.5;
}

//====================================================================//

template <class vector,class num>
struct BBox2D{
public:
  vector cen; // center
  vector rad; // extents
public:
  bool check(const BBox2D &b) const;
  bool inside(const vector &p) const;

  void zero()
    {cen.zero(); rad.zero();}
  void set(const vector &_cen,const vector &_rad)
    {cen=_cen; rad=_rad;}
  void set(const vector &_cen,num _rad)
    {cen=_cen; rad.set(_rad,_rad);}
  void set(const vector &p0,const vector &p1,num _rad);

  void expand(num amt);
  void add(const BBox2D &b);
  void add(vector cen,num rad);
  void merge(const BBox2D &a,const BBox2D &b);

  float area()
    {return(4*rad.x*rad.y);}
};

BBOX_TEM
void BBOX2D_FUN::set(const vector &p0,const vector &p1,num _rad)
{
  cen = (p0 + p1)*0.5;
  rad.set(fabs(p1.x - p0.x)*0.5 + _rad,
          fabs(p1.y - p0.y)*0.5 + _rad);
}

BBOX_TEM
bool BBOX2D_FUN::check(const BBox2D &b) const
{
  return(fabs(b.cen.x - cen.x) > (b.rad.x + rad.x) ||
         fabs(b.cen.y - cen.y) > (b.rad.y + rad.y));
}

BBOX_TEM
bool BBOX2D_FUN::inside(const vector &p) const
{
  return(fabs(p.x - cen.x) < rad.x &&
         fabs(p.y - cen.y) < rad.y);
}

BBOX_TEM
void BBOX2D_FUN::expand(num amt)
{
  rad.x += amt;
  rad.y += amt;
}

BBOX_TEM
void BBOX2D_FUN::add(const BBox2D &b)
{
  vector p0 = min(cen-rad, b.cen-b.rad);
  vector p1 = max(cen+rad, b.cen+b.rad);

  cen = (p1 + p0)*0.5;
  rad = (p1 - p0)*0.5;
}

BBOX_TEM
void BBOX2D_FUN::add(vector center,num radius)
{
  vector rv(radius,radius);
  vector p0 = min(cen-rad, center-rv);
  vector p1 = max(cen+rad, center+rv);

  cen = (p1 + p0)*0.5;
  rad = (p1 - p0)*0.5;
}

BBOX_TEM
void BBOX2D_FUN::merge(const BBox2D &a,const BBox2D &b)
{
  vector p0 = min(a.cen-a.rad, b.cen-b.rad);
  vector p1 = max(a.cen+a.rad, b.cen+b.rad);

  cen = (p1 + p0)*0.5;
  rad = (p1 - p0)*0.5;
}

}; // namespace

#endif

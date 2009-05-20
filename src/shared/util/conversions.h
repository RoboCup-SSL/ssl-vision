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
  \file    conversions.h
  \brief   Various color conversion operations, but NOT very optimized
  \author  
*/
//========================================================================

#ifndef _Conversions_H_
#define _Conversions_H_

#include "util.h"
#include "colors.h"
#ifndef NO_DC1394_CONVERSIONS
  #include <dc1394/conversions.h>
#endif

//#include "ccvt.h"

//-------------------------------------------------
//NOTE full image conversion routines (yuv->rgb) in this file
//     have been measured to be REALLY slow
//     Some functions will use dc1394 for speedup (if available)
//-------------------------------------------------


class Conversions {

public:
// base conversion methods for each pixel group
inline static void yuv2rgb(int y, int u, int v, int & r, int & g, int & b) 
{
  r = bound(y         + ((v*1436) >>10), 0, 255);
  g = bound(y - ((u*352 + v*731) >> 10), 0, 255);
  b = bound(y + ((u*1814) >> 10)       , 0, 255);
}

inline static rgb yuv2rgb(yuv const &in)
{
  int r, g, b;
  int y, u, v;
  y = (int) in.y;
  u = (int) in.u-128;
  v = (int) in.v-128;

  yuv2rgb(y, u, v, r, g, b);
  rgb col;
  col.r   = (unsigned char) r;
  col.g = (unsigned char) g;
  col.b  = (unsigned char) b;
  return (col);
}

inline static rgb rgba2rgb(rgb const &in) {
  rgb out;
  out.set(in.r,in.g,in.b);
  return out;
}

inline static  rgba rgb2rgba(rgb const &in, unsigned char alpha=0) {
  rgba out;
  out.set(in.r,in.g,in.b,alpha);
  return out;
}

inline static void rgb2yuv(int r, int g, int b, int & y, int & u, int & v) 
{
  y = bound((306*r + 601*g + 117*b)  >> 10, 0, 255);
  u = bound(((-172*r - 340*g + 512*b) >> 10)  + 128, 0, 255);
  v = bound(((512*r - 429*g - 83*b) >> 10) + 128, 0, 255);
}
inline static yuv rgb2yuv(rgb const &in)
{
  yuv col;
  int y, u, v;
  rgb2yuv((int) in.r, (int) in.g, 
          (int) in.b, y, u, v);
  col.y = (unsigned char) y;
  col.u = (unsigned char) u;
  col.v = (unsigned char) v;
  return (col);
}


//DC1394 accelerated:
static void uyvy2rgb (unsigned char *src, unsigned char *dest, int width, int height);

//others (non-accelerated):
static void uyyvyy2rgb (unsigned char *src, unsigned char *dest, int width, int height);
static void y2rgb (unsigned char *src, unsigned char *dest, int width, int height);
static void bgr2rgb (unsigned char *src, unsigned char *dest, int width, int height);
static void rgb2bgr (unsigned char *src, unsigned char *dest, int width, int height);
static void rgb482rgb (unsigned char *src, unsigned char *dest, int width, int height);
static void uyv2rgb (unsigned char *src, unsigned char *dest, int width, int height);
static void uyvy2bgr (unsigned char *src, unsigned char *dest, int width, int height);
static void y162rgb (unsigned char *src, unsigned char *dest, int width, int height, int bits);


};

#endif

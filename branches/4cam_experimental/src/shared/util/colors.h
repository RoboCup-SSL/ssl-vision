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
  \file    colors.h
  \brief   A collection of color related classes, types, and functions
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef COLORS
#define COLORS
#include <string>
#include <math.h>
#include <string.h>
#include <stdint.h>
using namespace std;

enum ColorSpace {
  CSPACE_UNDEFINED,
  CSPACE_RGB,
  CSPACE_YUV,
  CSPACE_HSV,
  CSPACE_LAB,
  CSPACE_COUNT
};

//Color Pixel-Format Enumeration
enum ColorFormat {
  COLOR_UNDEFINED,
  COLOR_RGB8,
  COLOR_RGBA8,
  COLOR_YUV411,
  COLOR_YUV422_UYVY,
  COLOR_YUV422_YUYV,
  COLOR_YUV444,
  COLOR_MONO16,
  COLOR_MONO8,
  COLOR_RGB16,
  COLOR_RAW8,
  COLOR_RAW16,
  COLOR_RAW32,
  COLOR_COUNT
};

//Color Helper Classes:
class Colors
{
public:
  static ColorFormat stringToColorFormat(const char * s)
  {
    if (strcmp(s,"rgb")==0) {
      return COLOR_RGB8;
    } else if (strcmp(s,"rgba")==0) {
      return COLOR_RGBA8;
    } else if (strcmp(s,"yuv411")==0) {
      return COLOR_YUV411;
    } else if (strcmp(s,"yuv422_uyvy")==0) {
      return COLOR_YUV422_UYVY;
    } else if (strcmp(s,"yuv422_yuyv")==0) {
      return COLOR_YUV422_YUYV;
    } else if (strcmp(s,"yuv444")==0) {
      return COLOR_YUV444;
    } else if (strcmp(s,"mono16")==0) {
      return COLOR_MONO16;
    } else if (strcmp(s,"mono8")==0) {
      return COLOR_MONO8;
    } else if (strcmp(s,"raw8")==0) {
      return COLOR_RAW8;
    } else if (strcmp(s,"raw16")==0) {
      return COLOR_RAW16;
    } else if (strcmp(s,"raw32")==0) {
      return COLOR_RAW32;
    } else if (strcmp(s,"rgb16")==0) {
      return COLOR_RGB16;
    } else {
      return COLOR_UNDEFINED;
    }
  }

  static string colorFormatToString(ColorFormat f)
  {
    if (f==COLOR_RGB8) {
      return ("rgb");
    } else if (f==COLOR_RGBA8) {
      return ("rgba");
    } else if (f==COLOR_YUV411) {
      return ("yuv411");
    } else if (f==COLOR_YUV422_UYVY) {
      return ("yuv422_uyvy");
    } else if (f==COLOR_YUV422_YUYV) {
      return ("yuv422_yuyv");
    } else if (f==COLOR_YUV444) {
      return ("yuv444");
    } else if (f==COLOR_MONO16) {
      return ("mono16");
    } else if (f==COLOR_MONO8) {
      return ("mono8");
    } else if (f==COLOR_RAW8) {
      return ("raw8");
    } else if (f==COLOR_RAW16) {
      return ("raw16");
    } else if (f==COLOR_RAW32) {
      return ("raw32");
    } else if (f==COLOR_RGB16) {
      return ("rgb16");
    } else if (f==COLOR_UNDEFINED) {
      return ("undefined");
    } else {
      return ("unknown");
    }
  }
};


/*!
  \class ColorRGB

  \brief Basic RGB color class.

  \author Stefan Zickler, (C) 2007
**/


//Basic RGB class:
template <class num, ColorFormat fmt>
class ColorRGB {
public:
//DATA:
  num r;
  num g;
  num b;

//METHODS:
  ColorRGB() {
    r=g=b=0;
  };
  ColorRGB(const ColorRGB<num, fmt> &c) {
    r=c.r;
    g=c.g;
    b=c.b;
  };
  ColorRGB(num _r, num _g, num _b) {
    r=_r;
    g=_g;
    b=_b;
  };
  ~ColorRGB() {
  };
  inline void set(num _r, num _g, num _b) {
    r=_r;
    g=_g;
    b=_b;
  };
  static ColorFormat getColorFormat() {
    return fmt;
  }
  bool operator ==(const ColorRGB<num, fmt> &c) const {
    return (r==c.r && g==c.g && b==c.b);
  };

  num inline getIntensity() {
    return (r+g+b)/3;
  }

  void inline setIntensity(num val) {
    r=g=b=val;
  }
  //subtraction operator
  ColorRGB<num, fmt> & operator-=(const ColorRGB<num, fmt> p) {
    if (p.r > r) { r=0; } else { r-=p.r; }
    if (p.g > g) { g=0; } else { g-=p.g; }
    if (p.b > b) { b=0; } else { b-=p.b; }
    return (*this);
  }

  //addition operator
  ColorRGB<num, fmt> & operator+=(const ColorRGB<num, fmt> p) {
    if ((int)p.r + (int)r > 255) { r=255; } else { r+=p.r; }
    if ((int)p.g + (int)g > 255) { g=255; } else { g+=p.g; }
    if ((int)p.b + (int)b > 255) { b=255; } else { b+=p.b; }
    return (*this);
  }

  ColorRGB<num, fmt> operator *(double f) const {
    ColorRGB<num, fmt> result(r*f,g*f,b*f);
    return result;
  };
};

//Basic RGB class:
template <class num, ColorFormat fmt>
class ColorGrey {
public:
//DATA:
  num v;

//METHODS:
  ColorGrey() {
    v=0;
  };
  ColorGrey(const ColorGrey<num, fmt> &c) {
    v=c.v;
  };
  ColorGrey(num _v) {
    v=_v;
  };
  ~ColorGrey() {
  };
  static ColorFormat getColorFormat() {
    return fmt;
  }
  bool operator ==(const ColorGrey<num, fmt> &c) const {
    return (v==c.v);
  };
  bool operator !=(const ColorGrey<num, fmt> &c) const {
    return (v!=c.v);
  };

  num inline getIntensity() {
    return v;
  }

  inline void set(num _v) {
    v=_v;
  }

  //subtraction operator
  ColorGrey<num, fmt> & operator-=(const ColorGrey<num, fmt> p) {
    if (p.v > v) { v=0; } else { v-=p.v; }
    return (*this);
  }

  ColorGrey<num, fmt> operator *(double f) const {
    ColorGrey<num, fmt> result(v*f);
    return result;
  };

  inline void setBit(int bit) {
    v |= (0x01 << bit);
  }

  inline void unsetBit(int bit) {
    v &= ~(0x01 << bit);
  }

};

template <class num, ColorFormat fmt>
class ColorRGBA {
public:
//DATA:
  num r;
  num g;
  num b;
  num a;

//METHODS:
  ColorRGBA() {
    r=g=b=0;
    a=255;
  };
  ColorRGBA(const ColorRGB<num, fmt> &c) {
    r=c.r;
    g=c.g;
    b=c.b;
    a=255;
  };
  ColorRGBA(const ColorRGBA<num, fmt> &c) {
    r=c.r;
    g=c.g;
    b=c.b;
    a=c.a;
  };
  ColorRGBA(num _r, num _g, num _b, num _a=255) {
    r=_r;
    g=_g;
    b=_b;
    a=_a;
  };
  ~ColorRGBA() {
  };
  inline void set(num _r, num _g, num _b, num _a=255) {
    r=_r;
    g=_g;
    b=_b;
    a=_a;
  }
  static ColorFormat getColorFormat() {
    return fmt;
  }
  bool operator ==(const ColorRGBA<num, fmt> &c) const {
    return (r==c.r && g==c.g && b==c.b && a=c.a);
  };
  ColorRGBA<num, fmt> operator *(double f) const {
    ColorRGBA<num, fmt> result(r*f,g*f,b*f,a*f);
    return result;
  };
};


template <class num, ColorFormat fmt>
class ColorYUV {
public:
//DATA:
  num y;
  num u;
  num v;

//METHODS:
  ColorYUV() {
    y=u=v=0;
  };
  ColorYUV(const ColorYUV<num, fmt> &c) {
    y=c.y;
    u=c.u;
    v=c.v;
  };
  ColorYUV(num _y, num _u, num _v) {
    y=_y;
    u=_u;
    v=_v;
  };
  ~ColorYUV() {
  };
  static ColorFormat getColorFormat() {
    return fmt;
  }
  bool operator ==(const ColorYUV<num, fmt> &c) const {
    return (y==c.y && u==c.u && v==c.v);
  };
  ColorYUV<num, fmt> operator *(double f) const {
    ColorYUV<num, fmt> result(y*f,u*f,v*f);
    return result;
  };
};


template <class num, ColorFormat fmt>
class ColorYUYV {
public:
//DATA:
  num y1;
  num u;
  num y2;
  num v;

//METHODS:
  ColorYUYV() {
    y1=u=y2=v=0;
  };
  ColorYUYV(const ColorYUV<num, fmt> &c) {
    y1=y2=c.y;
    u=c.u;
    v=c.v;
  };
  ColorYUYV(const ColorYUYV<num, fmt> &c) {
    y1=c.y1;
    y2=c.y2;
    u=c.u;
    v=c.v;
  };
  ColorYUYV(num _y1, num _u, num _y2, num _v) {
    y1=_y1;
    u=_u;
    y2=_y2;
    v=_v;
  };
  ~ColorYUYV() {
  };
  static ColorFormat getColorFormat() {
    return fmt;
  }
  bool operator ==(const ColorYUYV<num, fmt> &c) const {
    return (y1==c.y1 && u==c.u && y2==c.y2 && v==c.v);
  };
  ColorYUYV<num, fmt> operator *(double f) const {
    ColorYUYV<num, fmt> result(y1*f,u*f,y2*f,v*f);
    return result;
  };
};



template <class num, ColorFormat fmt>
class ColorUYVY {
public:
//DATA:
  num u;
  num y1;
  num v;
  num y2;

//METHODS:
  ColorUYVY() {
    u=y1=v=y2=0;
  };
  ColorUYVY(const ColorYUV<num, fmt> &c) {
    y1=y2=c.y;
    u=c.u;
    v=c.v;
  };
  ColorUYVY(const ColorUYVY<num, fmt> &c) {
    y1=c.y1;
    y2=c.y2;
    u=c.u;
    v=c.v;
  };
  ColorUYVY(num _u, num _y1, num _v, num _y2) {
    u=_u;
    y1=_y1;
    v=_v;
    y2=_y2;
  };
  ~ColorUYVY() {
  };
  static ColorFormat getColorFormat() {
    return fmt;
  }
  bool operator ==(const ColorYUYV<num, fmt> &c) const {
    return (u==c.u && y1==c.y1 && v==c.v && y2==c.y2);
  };
  ColorUYVY<num, fmt> operator *(double f) const {
    ColorUYVY<num, fmt> result(u*f,y1*f,v*f,y2*f);
    return result;
  };
};


typedef ColorRGB<unsigned char,COLOR_RGB8> rgb;
typedef ColorRGBA<unsigned char,COLOR_RGBA8> rgba;
typedef ColorYUV<unsigned char,COLOR_YUV444> yuv;
typedef ColorGrey<unsigned char,COLOR_MONO8> grey;
typedef ColorGrey<uint16_t,COLOR_MONO16> grey16;
typedef ColorGrey<uint8_t,COLOR_RAW8> raw8;
typedef ColorGrey<uint16_t,COLOR_RAW16> raw16;
typedef ColorGrey<uint32_t,COLOR_RAW32> raw32;
typedef ColorYUYV<uint8_t,COLOR_YUV422_UYVY> yuyv;
typedef ColorUYVY<uint8_t,COLOR_YUV422_UYVY> uyvy;
 

namespace RGB {
  static const rgb Black = rgb(  0,  0,  0 );
  static const rgb Blue  = rgb(  0,  0,255);
  static const rgb Green = rgb(  0,255,  0);
  static const rgb Red   = rgb(255,  0,  0);
  static const rgb White = rgb(255,255,255);
  static const rgb Gray  = rgb(128,128,128);
  
  static const rgb Cyan   = rgb(  0,255,255);
  static const rgb Pink   = rgb(255,  0,255);
  static const rgb Yellow = rgb(255,255,  0);
  static const rgb Orange = rgb(255,128,  0);

  static const rgb MidBlue  = rgb(  0,  0,192);
  static const rgb MidGreen = rgb(  0,192,  0);
  static const rgb MidRed   = rgb(192,  0,  0);
  
  static const rgb DarkBlue  = rgb(  0,  0,128);
  static const rgb DarkGreen = rgb(  0,128,  0);
  static const rgb DarkRed   = rgb(128,  0,  0);
  static const rgb Purple    = rgb(128,  0,128);
  static const rgb DarkGray = rgb( 64, 64, 64);
};

class ColorConversions {
public:
//input/output ranges:
//h: 0-360
//s,v 0-1
//rgb: 8-bit ints
  static void hsv2rgb(double h, double s, double v, rgb & v_out) {

int j;
double rd, gd, bd;
double f, p, q, t;

if(h==-1 || s==0.0)
{
 rd = v;
 gd = v;
 bd = v;
}
else
{
 if(h==360.0) h = 0.0;
 h = h / 60.0;
 j = (int) floor(h);
 if(j<0) j=0;
 f = h - j;
 p = v * (1-s);
 q = v * (1 - (s*f));
 t = v * (1 - (s*(1 - f)));

 switch(j)
 {
  case 0: rd = v; gd = t; bd = p; break;
  case 1: rd = q; gd = v; bd = p; break;
  case 2: rd = p; gd = v; bd = t; break;
  case 3: rd = p; gd = q; bd = v; break;
  case 4: rd = t; gd = p; bd = v; break;
  case 5: rd = v; gd = p; bd = q; break;
  default:rd = v; gd = t; bd = p; break;
 }
}

v_out.r = (int) floor((rd * 255.0) + 0.5);
v_out.g = (int) floor((gd * 255.0) + 0.5);
v_out.b = (int) floor((bd * 255.0) + 0.5);
  }
};


//FIXME: uncomment the following code
//it should work...just needs to be adapted to our datatypes:

// void rgb2hsv(int r,int  g,int b, double *hr,  double *sr, double *vr)
// {
// double rd, gd, bd, h, s, v, max, min, del, rc, gc, bc;
// 
// rd = r / 255.0;
// gd = g / 255.0;
// bd = b / 255.0;
// 
// if(rd>=gd) { if(rd>=bd) max = rd; else max = bd; }
//    else { if(gd>=bd) max = gd; else max = bd; }
// 
// if(rd<=gd) { if(rd<=bd) min = rd; else min = bd; }
//    else { if(gd<=bd) min = gd; else min = bd; }
// 
// del = max - min;
// v = max;
// if(max != 0.0) s = (del) / max;
// else s = 0.0;
// 
// h = -1;
// if(s != 0.0)
// {
//  rc = (max - rd) / del;
//  gc = (max - gd) / del;
//  bc = (max - bd) / del;
// 
//  if (rd==max) h = bc - gc;
//  else if (gd==max) h = 2 + rc - bc;
//  else if (bd==max) h = 4 + gc - rc;
// 
//  h = h * 60;
//  if (h<0) h += 360;
// }
// 
// *hr = h; *sr = s; *vr = v;
// } 


#endif

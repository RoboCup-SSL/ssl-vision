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
  \file    conversions.cpp
  \brief   Various color conversion operations, but NOT very optimized
  \author  
*/
//========================================================================

#include <stdlib.h>
#include <iostream>
//#include <libdc1394/dc1394_control.h>

#include "conversions.h"

using namespace std;
namespace Conversions {
// The following #define is there for the users who experience green/purple
// images in the display. This seems to be a videocard driver problem.

#define YUYV // instead of the standard UYVY


void bgr2rgb (unsigned char *src, 
	      unsigned char *dest, 
	      int NumPixels) {
  for (int i=0;i<NumPixels*3;i+=3) {
    dest[i]   = src[i+2];
    dest[i+1] = src[i+1];
    dest[i+2] = src[i];
  }
}

void rgb2bgr (unsigned char *src, 
	      unsigned char *dest, 
	      int NumPixels) {
  for (int i=0;i<NumPixels*3;i+=3) {
    dest[i]   = src[i+2];
    dest[i+1] = src[i+1];
    dest[i+2] = src[i];
  }
}

void rgb482rgb (unsigned char *src, 
		unsigned char *dest, 
		int NumPixels) {
  register int i = ((NumPixels + ( NumPixels << 1 )) << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;

  while (i > 0) {
    i--;
    dest[j--]=src[i--];
    i--;
    dest[j--]=src[i--];
    i--;
    dest[j--]=src[i--];
  }
}


void uyv2rgb (unsigned char *src, 
	      unsigned char *dest, 
	      int NumPixels)
{
  register int i = NumPixels + ( NumPixels << 1 ) -1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y, u, v;
  register int r, g, b;


  while (i > 0) {
    v = (unsigned char) src[i--] - 128;
    y = (unsigned char) src[i--];
    u = (unsigned char) src[i--] - 128;
    yuv2rgb (y, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
  }
}

void uyvy2rgb (unsigned char *src, 
	       unsigned char *dest, 
	       int NumPixels) {
#if 1
  register int max_i = (NumPixels << 1)-1;
  //register int max_j = NumPixels + ( NumPixels << 1 ) -1;
  register int i = 0;
  register int j = 0;
  register int y0, y1, u, v;
  register int r, g, b;

  while (i < max_i) {
    u  = (unsigned char) src[i++] - 128;
    y0 = (unsigned char) src[i++];
    v  = (unsigned char) src[i++] - 128;
    y1 = (unsigned char) src[i++];
    yuv2rgb (y0, u, v, r, g, b);
    dest[j++] = r;
    dest[j++] = g;
    dest[j++] = b;
    yuv2rgb (y1, u, v, r, g, b);
    dest[j++] = r;
    dest[j++] = g;
    dest[j++] = b;
  }
#else
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y0, y1, u, v;
  register int r, g, b;

  while (i > 0) {
    y1 = (unsigned char) src[i--];
    v  = (unsigned char) src[i--] - 128;
    y0 = (unsigned char) src[i--];
    u  = (unsigned char) src[i--] - 128;
    yuv2rgb (y1, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    yuv2rgb (y0, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
  }
#endif
}

void uyvy2bgr (unsigned char *src, 
	       unsigned char *dest, 
	       int NumPixels) {
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y0, y1, u, v;
  register int r, g, b;

  while (i > 0) {
    y1 = (unsigned char) src[i--];
    v  = (unsigned char) src[i--] - 128;
    y0 = (unsigned char) src[i--];
    u  = (unsigned char) src[i--] - 128;
    yuv2rgb (y1, u, v, r, g, b);
    dest[j--] = r;
    dest[j--] = g;
    dest[j--] = b;
    yuv2rgb (y0, u, v, r, g, b);
    dest[j--] = r;
    dest[j--] = g;
    dest[j--] = b;
  }
}

void uyyvyy2rgb (unsigned char *src, 
		 unsigned char *dest, 
		 int NumPixels) {
  register int i = NumPixels + ( NumPixels >> 1 )-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y0, y1, y2, y3, u, v;
  register int r, g, b;
  
  while (i > 0) {
    y3 = (unsigned char) src[i--];
    y2 = (unsigned char) src[i--];
    v  = (unsigned char) src[i--] - 128;
    y1 = (unsigned char) src[i--];
    y0 = (unsigned char) src[i--];
    u  = (unsigned char) src[i--] - 128;
    yuv2rgb (y3, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    yuv2rgb (y2, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    yuv2rgb (y1, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    yuv2rgb (y0, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
  }
}

void y2rgb (unsigned char *src, 
	    unsigned char *dest, 
	    int NumPixels) {
  register int i = NumPixels-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while (i > 0) {
    y = (unsigned char) src[i--];
    dest[j--] = y;
    dest[j--] = y;
    dest[j--] = y;
  }
}

void y162rgb (unsigned char *src, 
	      unsigned char *dest, 
	      int NumPixels, 
	      int bits) {
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while (i > 0) {
    y = src[i--];
    y = (y + (src[i--]<<8))>>(bits-8);
    dest[j--] = y;
    dest[j--] = y;
    dest[j--] = y;
  }
}

#if 0
// This functino is the only one that is dependent
// on the IEEE1394 header. So we will only remove it
// from compilation if IEEE1394 is not defined

void convert_to_rgb(unsigned char *src, 
		    unsigned char *dest, 
		    int width, 
		    int height, 
		    int mode, 
		    int bits) {

  switch(mode) {
  case MODE_160x120_YUV444:
    uyv2rgb(src,dest,width*height);
    break;
  case MODE_320x240_YUV422:
  case MODE_640x480_YUV422:
  case MODE_800x600_YUV422:
  case MODE_1024x768_YUV422:
  case MODE_1280x960_YUV422:
  case MODE_1600x1200_YUV422:
    uyvy2rgb(src,dest,width*height);
    break;
  case MODE_640x480_YUV411:
    uyyvyy2rgb(src,dest,width*height);
    break;
  case MODE_640x480_RGB:
  case MODE_800x600_RGB:
  case MODE_1024x768_RGB:
  case MODE_1280x960_RGB:
  case MODE_1600x1200_RGB:
    memcpy(dest,src,3*width*height);
    break;
  case MODE_640x480_MONO:
  case MODE_800x600_MONO:
  case MODE_1024x768_MONO:
  case MODE_1280x960_MONO:
  case MODE_1600x1200_MONO:
    y2rgb(src,dest,width*height);
    break;
  case MODE_640x480_MONO16:
  case MODE_800x600_MONO16:
  case MODE_1024x768_MONO16:
  case MODE_1280x960_MONO16:
  case MODE_1600x1200_MONO16:
    y162rgb(src,dest,width*height,bits);
    break;
  default:
    // Unknown mode.  Just do the memcpy
#ifdef _DEBUG
    cerr << "Unknown conversion mode: forcing rgb2rgb" << endl;
#endif
    memcpy(dest,src,3*width*height);
  }
}
#endif
}

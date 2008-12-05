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


#include "conversions.h"

using namespace std;
// The following #define is there for the users who experience green/purple
// images in the display. This seems to be a videocard driver problem.


void Conversions::bgr2rgb ( unsigned char *src,
                            unsigned char *dest,
                            int width,
                            int height ) {
  int NumPixels = width*height;
  for ( int i=0;i<NumPixels*3;i+=3 ) {
    dest[i]   = src[i+2];
    dest[i+1] = src[i+1];
    dest[i+2] = src[i];
  }
}

void Conversions::rgb2bgr ( unsigned char *src,
                            unsigned char *dest,
                            int width,
                            int height ) {
  int NumPixels = width*height;
  for ( int i=0;i<NumPixels*3;i+=3 ) {
    dest[i]   = src[i+2];
    dest[i+1] = src[i+1];
    dest[i+2] = src[i];
  }
}

void Conversions::rgb482rgb ( unsigned char *src,
                              unsigned char *dest,
                              int width,
                              int height ) {
  int NumPixels = width*height;
  register int i = ( ( NumPixels + ( NumPixels << 1 ) ) << 1 )-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;

  while ( i > 0 ) {
    i--;
    dest[j--]=src[i--];
    i--;
    dest[j--]=src[i--];
    i--;
    dest[j--]=src[i--];
  }
}


void Conversions::uyv2rgb ( unsigned char *src,
                            unsigned char *dest,
                            int width,
                            int height ) {
  int NumPixels = width*height;
  register int i = NumPixels + ( NumPixels << 1 ) -1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y, u, v;
  int r, g, b;


  while ( i > 0 ) {
    v = ( unsigned char ) src[i--] - 128;
    y = ( unsigned char ) src[i--];
    u = ( unsigned char ) src[i--] - 128;
    yuv2rgb ( y, u, v, r, g, b );
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
  }
}

void Conversions::uyvy2rgb ( unsigned char *src,
                             unsigned char *dest,
                             int width,
                             int height ) {
  #ifndef NO_DC1394_CONVERSIONS
    dc1394_convert_to_RGB8(src,dest, width, height, DC1394_BYTE_ORDER_UYVY,
                       DC1394_COLOR_CODING_YUV422, 8);
  #else
  
  int NumPixels = width*height;
                             
  register int max_i = ( NumPixels << 1 )-1;
  //register int max_j = NumPixels + ( NumPixels << 1 ) -1;
  register int i = 0;
  register int j = 0;
  register int y0, y1, u, v;
  register int r, g, b;

  while ( i < max_i ) {
    u  = ( unsigned char ) src[i++] - 128;
    y0 = ( unsigned char ) src[i++];
    v  = ( unsigned char ) src[i++] - 128;
    y1 = ( unsigned char ) src[i++];
    yuv2rgb ( y0, u, v, r, g, b );
    dest[j++] = r;
    dest[j++] = g;
    dest[j++] = b;
    yuv2rgb ( y1, u, v, r, g, b );
    dest[j++] = r;
    dest[j++] = g;
    dest[j++] = b;
  }
  #endif
}

void Conversions::uyvy2bgr ( unsigned char *src,
                             unsigned char *dest,
                             int width,
                             int height ) {

  int NumPixels = width*height;

  register int i = ( NumPixels << 1 )-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y0, y1, u, v;
  int r, g, b;

  while ( i > 0 ) {
    y1 = ( unsigned char ) src[i--];
    v  = ( unsigned char ) src[i--] - 128;
    y0 = ( unsigned char ) src[i--];
    u  = ( unsigned char ) src[i--] - 128;
    yuv2rgb ( y1, u, v, r, g, b );
    dest[j--] = r;
    dest[j--] = g;
    dest[j--] = b;
    yuv2rgb ( y0, u, v, r, g, b );
    dest[j--] = r;
    dest[j--] = g;
    dest[j--] = b;
  }
}

void Conversions::uyyvyy2rgb ( unsigned char *src,
                               unsigned char *dest,
                               int width,
                               int height ) {

  int NumPixels = width*height;
  register int i = NumPixels + ( NumPixels >> 1 )-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y0, y1, y2, y3, u, v;
  int r, g, b;

  while ( i > 0 ) {
    y3 = ( unsigned char ) src[i--];
    y2 = ( unsigned char ) src[i--];
    v  = ( unsigned char ) src[i--] - 128;
    y1 = ( unsigned char ) src[i--];
    y0 = ( unsigned char ) src[i--];
    u  = ( unsigned char ) src[i--] - 128;
    yuv2rgb ( y3, u, v, r, g, b );
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    yuv2rgb ( y2, u, v, r, g, b );
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    yuv2rgb ( y1, u, v, r, g, b );
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    yuv2rgb ( y0, u, v, r, g, b );
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
  }
}

void Conversions::y2rgb ( unsigned char *src,
                          unsigned char *dest,
                          int width,
                          int height ) {
  int NumPixels = width*height;
  register int i = NumPixels-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while ( i > 0 ) {
    y = ( unsigned char ) src[i--];
    dest[j--] = y;
    dest[j--] = y;
    dest[j--] = y;
  }
}

void Conversions::y162rgb ( unsigned char *src,
                            unsigned char *dest,
                            int width,
                            int height,
                            int bits ) {
  int NumPixels = width*height;
  register int i = ( NumPixels << 1 )-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while ( i > 0 ) {
    y = src[i--];
    y = ( y + ( src[i--]<<8 ) ) >> ( bits-8 );
    dest[j--] = y;
    dest[j--] = y;
    dest[j--] = y;
  }
}




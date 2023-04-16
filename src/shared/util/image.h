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
  \file    image.h
  \brief   C++ Interface: Image
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#pragma once

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cassert>
#include <string>
#include "colors.h"
#include "util.h"
#include "image_interface.h"
#include "rawimage.h"
#include "image_io.h"
#include "font.h"
#include "conversions.h"

/*!
  \class Image
  \brief A template-based 2D raster-image class
*/
template <class PIXEL>
class Image : public ImageInterface
{
public:
  //Variables:
  bool _external;
  int width;
  int height;
  PIXEL * data;

  //this does a shallow copy (it needs the raw image's data to keep existing
  void fromRawImage(const RawImage & img)
  {
    if (PIXEL::getColorFormat() == img.getColorFormat()) {
      clear();
      _external=true;
      data=(PIXEL *)img.getData();
      width=img.getWidth();
      height=img.getHeight();
    } else {
      fprintf(stderr,"cannot create image from rawimage. colortypes do not match\n");
    }
  }

  void copyToRawImage(RawImage & img) {
    if (PIXEL::getColorFormat() == img.getColorFormat() && img.getNumBytes() == getNumBytes()) {
      memcpy(img.getData(),getData(),getNumBytes());
      img.setWidth( getWidth());
      img.setHeight( getHeight());
    } else {
      fprintf(stderr,"cannot copy image to rawimage. colortypes and/or size do not match\n");
    }

  }

  //this does a shallow copy (it needs the original image's data to keep existing
  //for a deep copy use the copy(...) function
  void fromImage(const Image<PIXEL> & img)
  {
    clear();
    _external=true;
    data=img.getData();
    width=img.getWidth();
    height=img.getHeight();
  }

  void allocate (int w, int h)
  {
    //if (width==w && height==h && _external==false && data!=0) return;
    assert(w >= 0 && h >= 0);
    if (data!=0 && _external==false) {
      if (width==w && height==h) return;
      delete[] data;
    }
    if (w==0 && h==0) {
      data=0;
    } else {
      data=new PIXEL[w*h];
    }
    _external=false;
    width=w;
    height=h;
  }
  void clear() {
    allocate(0,0);
  };
  //Methods:
  Image(const RawImage & ri) {
    data=0;
    _external=false;
    allocate(0,0);
    fromRawImage(ri);
  }

  Image()
  {
    data=0;
    _external=false;
    allocate(0,0);
  }

  Image(int w, int h)
  {
    data=0;
    _external=false;
    allocate(w,h);
  }

  ~Image()
  {
    if (data!=0 && _external==false) {
      delete[] data;
    }
  }

  ColorFormat getColorFormat() const
  {
    return PIXEL::getColorFormat();
  }

  int getWidth () const
  {
    return width;
  };

  int getHeight()  const
  {
    return height;
  };

  int getNumPixels() const
  {
    return width*height;
  }

  int getNumBytes() const
  {
    return width*height*sizeof(PIXEL);
  }

  void fillBlack() {
    memset((void*) data,0,getNumBytes());
  }

  void fillColor(const PIXEL & color) {
    int count=getNumPixels();
    PIXEL * p=getPixelData();
    for (int i=0;i<count;i++) {
      (*p)=color;
      p++;
    }
  }

  PIXEL * getPixelData  () const
  {
    return data;
  };

  unsigned char * getData() const
  {
    return (unsigned char *) data;
  }

  PIXEL getPixel (int number) const
  {
    assert(number >= 0 && number < (width*height));
    return(*(data+number));
  }

  PIXEL getPixel (int x,int y) const
  {
    assert(x >= 0 && y >=0 && x<width && y<height);
    return(*(data+(width*y)+x));
  }

  PIXEL * getPixelPointer (int x,int y) const
  {
    assert(x >= 0 && y >=0 && x<width && y<height);
    return((data+(width*y)+x));
  }

  inline void setPixel (int x,int y, PIXEL val)
  {
    if (x >= 0 && y >=0 && x<width && y<height) {
      (*(data+(width*y)+x))=val;
    }
  }


  void drawLine (int x0, int y0, int x1, int y1 , PIXEL val)
  {

  int x, y, dx, dy, sx, sy, ax, ay, decy, decx;
  x = x0;
  y = y0;
  dx = x1 - x0;
  dy = y1 - y0;
  if ( dx > 0 ) {
    sx = 1;
  } else {
    if ( dx < 0 ) {
      sx = -1;
      dx = -dx;
    } else {
      sx = 0;
    }
  }
  if ( dy > 0 ) {
    sy = 1;
  } else {
    if ( dy < 0 ) {
      sy = -1;
      dy = -dy;
    } else {
      sy = 0;
    }
  }
  ax = 2 * dx;
  ay = 2 * dy;
  if ( dy <= dx ) {
    for ( decy = ay - ax; ; x = x + sx, decy = decy + ay ) {
      setPixel(x,y,val);
      //TABLE [_curstep] [ x ][ y ] = color;
      if ( x == x1 ) break;
      if ( decy >= 0 ) {
        decy = decy - ax;
        y = y + sy;
      }
    }
  } else {
    for ( decx = ax - ay; ; y = y + sy, decx = decx + ax ) {
      setPixel(x,y,val);

      if ( y == y1 ) break;
      if ( decx >= 0 ) {
        decx = decx - ay;
        x = x + sx;
      }
    }
  }

}

void drawFatLine (int x0, int y0, int x1, int y1 , PIXEL val)
{

  int x, y, dx, dy, sx, sy, ax, ay, decy, decx;
  x = x0;
  y = y0;
  dx = x1 - x0;
  dy = y1 - y0;
  if ( dx > 0 ) {
    sx = 1;
  } else {
    if ( dx < 0 ) {
      sx = -1;
      dx = -dx;
    } else {
      sx = 0;
    }
  }
  if ( dy > 0 ) {
    sy = 1;
  } else {
    if ( dy < 0 ) {
      sy = -1;
      dy = -dy;
    } else {
      sy = 0;
    }
  }
  ax = 2 * dx;
  ay = 2 * dy;
  if ( dy <= dx ) {
    for ( decy = ay - ax; ; x = x + sx, decy = decy + ay ) {
      setPixel(x,y,val);setPixel(x+1,y,val);setPixel(x-1,y,val);
      setPixel(x,y-1,val);setPixel(x+1,y-1,val);setPixel(x-1,y-1,val);
      setPixel(x,y+1,val);setPixel(x+1,y+1,val);setPixel(x-1,y+1,val);
      //TABLE [_curstep] [ x ][ y ] = color;
      if ( x == x1 ) break;
      if ( decy >= 0 ) {
        decy = decy - ax;
        y = y + sy;
      }
    }
  } else {
    for ( decx = ax - ay; ; y = y + sy, decx = decx + ax ) {
      setPixel(x,y,val);setPixel(x+1,y,val);setPixel(x-1,y,val);
      setPixel(x,y-1,val);setPixel(x+1,y-1,val);setPixel(x-1,y-1,val);
      setPixel(x,y+1,val);setPixel(x+1,y+1,val);setPixel(x-1,y+1,val);
      if ( y == y1 ) break;
      if ( decx >= 0 ) {
        decx = decx - ay;
        x = x + sx;
      }
    }
  }

}

  void drawBox(int x, int y, int width, int height, PIXEL val)
  {
    drawLine(x, y, x+width, y, val);
    drawLine(x+width, y, x+width, y+height, val);
    drawLine(x+width, y+height, x, y+height, val);
    drawLine(x, y+height, x, y, val);
  }

  void drawFatBox(int x, int y, int width, int height, PIXEL val)
  {
    drawFatLine(x, y, x+width, y, val);
    drawFatLine(x+width, y, x+width, y+height, val);
    drawFatLine(x+width, y+height, x, y+height, val);
    drawFatLine(x, y+height, x, y, val);
  }

  bool load(string filename) {
  (void)filename;
   if ((PIXEL::getColorFormat()==COLOR_RGB8) || (PIXEL::getColorFormat()==COLOR_RGBA8)) {
      clear();
      int w,h;
      w=0;
      h=0;
      if (PIXEL::getColorFormat()==COLOR_RGB8) {
        data=(PIXEL *)(ImageIO::readRGB(w,h,filename.c_str()));
      } else {
        data=(PIXEL *)(ImageIO::readRGBA(w,h,filename.c_str()));
      }
      if (data!=0) {
        width=w;
        height=h;
        return true;
      } else {
        return false;
      }
   } else {
   	//TODO: loading of formats other than pure RGB
   	//      is not yet supported
   	return false;
   }
  }

  bool save(string filename) {
   if (PIXEL::getColorFormat()==COLOR_RGB8) {
   	 return ImageIO::writeRGB(getPixelData(), getWidth() , getHeight() ,filename.c_str());
   } else {
   	//TODO: saving of formats other than pure RGB
   	//      is not yet supported
   	return false;
   }
  }

  void copy(const Image &source, bool allow_external=false) {
    if (!(allow_external && source.getWidth()==getWidth() && source.getHeight() == getHeight())) {
      allocate(source.getWidth(),source.getHeight());
    }
    memcpy((void*) data,source.getData(),source.getNumBytes());
  }

  void copyFromRectArea(const Image &source, int x, int y, int w, int h, bool allow_external=false) {
    if (x < 0) x=0;
    if (y < 0) y=0;
    if (w < 0) w=0;
    if (h < 0) h=0;
    if ((x+w) >= source.getWidth()) w = source.getWidth() - x;
    if ((y+h) >= source.getHeight()) h = source.getHeight() - y;
    if (w<=0 || h <=0) w=h=0;

    if (!(allow_external && w==getWidth() && h == getHeight())) {
      allocate(w,h);
    }
    if (w==0||h==0) return;
    //TODO: add rect copy from source:
    int my=y+h;
    PIXEL * src_ptr = source.getPixelPointer(x,y);
    PIXEL * tgt_ptr = getPixelData();
    for (int i = y ; i < my; i++) {
      memcpy((void*) tgt_ptr, src_ptr ,sizeof(PIXEL) * w);
      src_ptr+=source.getWidth();
      tgt_ptr+=w;
    }
  }

  void subtract(const Image &source) {
    if (source.getNumPixels()==getNumPixels()) {
      register PIXEL * a=getPixelData();
      register PIXEL * b=source.getData();
      register unsigned int i;
      register unsigned int pixelCount=getNumPixels();
      for (i=0;i<pixelCount;i++) {
        (*a)-=(*b);
        a++;
        b++;
      }
    }
  }

  void convertToIntensity() {
      register PIXEL * a=getPixelData();
      register unsigned int i;
      register unsigned int pixelCount=getNumPixels();
      for (i=0;i<pixelCount;i++) {
        a->r = a->g = a->b = a->getIntensity();
        a++;
      }
  }

  void binarizeGreyImage(unsigned int threshold) {
      register PIXEL * a=getPixelData();
      register unsigned int i;
      register unsigned int pixelCount=getNumPixels();
      for (i=0;i<pixelCount;i++) {
        if (a->r > threshold) {a->r = 255; } else {a->r=0; };
        a->g=a->b=a->r;
        a++;
      }
  }


  void binarizeChanneledImage(unsigned int threshold_r,unsigned int threshold_g,unsigned int threshold_b) {
      register PIXEL * a=getPixelData();
      register unsigned int i;
      register unsigned int pixelCount=getNumPixels();
      for (i=0;i<pixelCount;i++) {
        if (a->r > threshold_r) {a->r = 255; } else {a->r=0; };
        if (a->g > threshold_g) {a->g = 255; } else {a->g=0; };
        if (a->b > threshold_b) {a->b = 255; } else {a->b=0; };
        a++;
      }
  }

  void drawChar(int x, int y, char c, PIXEL val)
  {
    int charWidth(8), charHeight(8), charSize(8);
    int x0=x;
    unsigned char* charpos(gfxPrimitivesFontdata + (unsigned char) c * charSize);

    unsigned char patt(0);
    for (int iy = 0; iy < charHeight; iy++) {
      unsigned char mask = 0x00;
      x=x0;
      for (int ix = 0; ix < charWidth; ix++) {
        if (!(mask >>= 1)) {
          patt = *charpos++;
          mask = 0x80;
        }

        if (patt & mask) setPixel(x,y,val);
        x++;
      }
      y++;
    }
  }

  void drawString(int x, int y, std::string s, PIXEL val)
  {
    for (unsigned int i=0; i < s.length(); i++)
      drawChar(x+8*i, y, s[i], val);
  }

};

/*!
  \class rgbImage
  \brief an 8-bit per channel rgb image class, based on the Image template class
*/
typedef Image<rgb>  rgbImage;

/*!
  \class rgbaImage
  \brief an 8-bit-per channel rgb+alpha image class, based on the Image template class
*/
typedef Image<rgba> rgbaImage;

/*!
  \class yuvImage
  \brief an 8-bit-per channel yuv image class, based on the Image template class
*/
typedef Image<yuv>  yuvImage;

//*!
//  \class uyvyImage
//  \brief a YUV422 image as it is typically transmitted by IEEE1394
//*/
//typedef Image<uyvy> uyvyImage;


/*!
  \class greyImage
  \brief an 8-bit greyscale image class, based on the Image template class
*/
typedef Image<grey> greyImage;

typedef Image<raw8> rawImage8;
typedef Image<raw16> rawImage16;
typedef Image<raw32> rawImage32;

/*!
  \class Images
  \brief A class with simple static image manipulation operations
*/
class Images
{
public:
  static void convert(const yuvImage & a, rgbImage & b) {
    if (a.getNumPixels()==b.getNumPixels()) {
      yuv * p1=a.getPixelData();
      rgb * p2=b.getPixelData();
      int n=a.getNumPixels();
      for (int i=0;i<n;i++) {
        *p2=Conversions::yuv2rgb(*p1);
        p1++;
        p2++;
      }
    } else {
      fprintf(stderr,"Cannot convert image of different sizes\n");
    }
  }
  static void convert(const rgbImage & a, yuvImage & b) {
    if (a.getNumPixels()==b.getNumPixels()) {
      rgb * p1=a.getPixelData();
      yuv * p2=b.getPixelData();
      int n=a.getNumPixels();
      for (int i=0;i<n;i++) {
        *p2=Conversions::rgb2yuv(*p1);
        p1++;
        p2++;
      }
    } else {
      fprintf(stderr,"Cannot convert image of different sizes\n");
    }
  }
  static void convert(const rgbImage & a, greyImage & b) {
    if (a.getNumPixels()==b.getNumPixels()) {
      rgb * p1=a.getPixelData();
      grey * p2=b.getPixelData();
      int n=a.getNumPixels();
      for (int i=0;i<n;i++) {
        p2->v=(p1->getIntensity());
        p1++;
        p2++;
      }
    } else {
      fprintf(stderr,"Cannot convert image of different sizes\n");
    }
  }
  static void convert(const rgbImage & a, rgbaImage & b) {
    if (a.getNumPixels()==b.getNumPixels()) {
      rgb * p1=a.getPixelData();
      rgba * p2=b.getPixelData();
      int n=a.getNumPixels();
      for (int i=0;i<n;i++) {
        p2->set(p1->r,p1->g,p1->b);
        p1++;
        p2++;
      }
    } else {
      fprintf(stderr,"Cannot convert image of different sizes\n");
    }
  }

  static void RGBAsetAlpha(const greyImage & a, rgbaImage & b) {
    if (a.getNumPixels()==b.getNumPixels()) {
      grey * p1=a.getPixelData();
      rgba * p2=b.getPixelData();
      int n=a.getNumPixels();
      for (int i=0;i<n;i++) {
        p2->a = p1->v;
        p1++;
        p2++;
      }
    } else {
      fprintf(stderr,"Cannot convert image of different sizes\n");
    }
  }

  static void RGBAsetRGB(const rgbImage & a, rgbaImage & b) {
    if (a.getNumPixels()==b.getNumPixels()) {
      rgb * p1=a.getPixelData();
      rgba * p2=b.getPixelData();
      int n=a.getNumPixels();
      for (int i=0;i<n;i++) {
        p2->r = p1->r;
        p2->g = p1->g;
        p2->b = p1->b;
        p1++;
        p2++;
      }
    } else {
      fprintf(stderr,"Cannot convert image of different sizes\n");
    }
  }

};

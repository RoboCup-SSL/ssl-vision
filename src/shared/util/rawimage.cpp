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
  \file    rawimage.cpp
  \brief   C++ Implementation: RawImage
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "rawimage.h"

RawImage::RawImage()
{
  data=0;
  width=0;
  height=0;
  format=COLOR_UNDEFINED;
  time=0.0;
}


RawImage::~RawImage()
{
}


int RawImage::getWidth() const
{
  return width;
}

int RawImage::getHeight() const
{
  return height;
}

ColorFormat RawImage::getColorFormat() const
{
  return format;
}

double RawImage::getTime() const
{
  return time;
}

unsigned char * RawImage::getData() const
{
  return data;
}

int RawImage::getNumPixels() const
{
  return width*height;
}

int RawImage::getNumBytes() const
{
  return computeImageSize(format,getNumPixels());
}

int RawImage::getNumColorBlocks() const {
  int pixelCount=width*height;
  switch (getColorFormat()) {
    case COLOR_YUV422_UYVY:
    return pixelCount/2;
    case COLOR_YUV411:
    return pixelCount/4;
    default:
    return pixelCount;
  }
}

void RawImage::setColorFormat(ColorFormat f)
{
  format=f;
}

void RawImage::setWidth(int w)
{
  width=w;
}

void RawImage::setHeight(int h)
{
  height=h;
}

void RawImage::setTime(double t)
{
  time=t;
}

void RawImage::setData(unsigned char * d)
{
  if (data!=0) delete[] data;
  data=d;
}

void  RawImage::allocate (ColorFormat fmt, int w, int h)
{
  if(w >= 0 && h >= 0) {
    if (data!=0) {
      delete[] data;
    }
    if (w==0 && h==0) {
      data=0;
    } else {
      data=new unsigned char[computeImageSize(fmt,w*h)];
    }
    width=w;
    height=h;
    format=fmt;
  }
}

void  RawImage::ensure_allocation (ColorFormat fmt, int w, int h)
{
  if(data == 0 || format != fmt || width != w || height!=h) {
    allocate(fmt,w,h);
  }
}

void RawImage::deepCopyFromRawImage(const RawImage & img, bool copyMetaData)
{
  ensure_allocation(img.getColorFormat(),img.getWidth(),img.getHeight());
  memcpy(getData(),img.getData(),img.getNumBytes());
  if (copyMetaData) {
    time=img.time;
  }
}

void RawImage::clear()
{
  allocate(getColorFormat(),0,0);
};

int RawImage::computeImageSize(ColorFormat fmt, int pixelCount)
{
  switch (fmt) {
    case COLOR_RGB8:
    return pixelCount*3;
    case COLOR_RGBA8:
    return pixelCount*4;
    case COLOR_YUV444:
    return pixelCount*3;
    case COLOR_YUV422_UYVY:
    return pixelCount*2;
    case COLOR_YUV411:
    return pixelCount*3/2;
    case COLOR_MONO8:
    return pixelCount;
    case COLOR_MONO16:
    return pixelCount*2;
    default:
    return 0;
  }
}


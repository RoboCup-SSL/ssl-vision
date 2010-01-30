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
  \file    rawimage.h
  \brief   C++ Interface: RawImage
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef RAWIMAGE_H
#define RAWIMAGE_H
#include "image_interface.h"
#include "colors.h"

/*!
  \class  RawImage
  \brief  A class providing description and storage of raw image data
  \author Stefan Zickler, (C) 2008

  The RawImage class stores a pointer and basic meta-data (width,
  height, color-format, timestamp).

  This class is mostly used for storing captured data.
  For an image class providing higher level processing functions, look at
  Image and its template instantiations rgbImage, rgbaImage, greyImage etc.
*/
class RawImage : public ImageInterface
{
  protected:
  /// pointer to capture buffer
  unsigned char * data;

  /// width of the image in pixels
  int width;

  /// height of the image in pixels
  int height;

  /// encoding format of the image
  ColorFormat format;

  /// capture timestamp of the image
  double   time;

  public:
  RawImage();

  ~RawImage();

  //accessors:
  int getWidth() const;
  int getHeight() const;
  ColorFormat getColorFormat() const;
  double getTime() const;
  unsigned char * getData() const;
  int getNumBytes() const;
  int getNumColorBlocks() const;
  int getNumPixels() const;

  //mutators:
  void setColorFormat(ColorFormat f);
  void setWidth(int w);
  void setHeight(int h);
  void setTime(double t);
  void setData(unsigned char * d);
  void allocate (ColorFormat fmt, int w, int h);
  void ensure_allocation (ColorFormat fmt, int w, int h);
  void deepCopyFromRawImage(const RawImage & img, bool copyMetaData);
  void clear();

  //helpers:
  static int computeImageSize(ColorFormat fmt, int pixelCount);

};

#endif

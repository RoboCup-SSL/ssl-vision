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
  unsigned char * data = nullptr;

  /// width of the image in pixels
  int width = 0;

  /// height of the image in pixels
  int height = 0;

  /// encoding format of the image
  ColorFormat format = COLOR_UNDEFINED;

  /// capture timestamp of the image in [s]
  double time = 0.0;

  /// capture timestamp of the image in [ns]
  double time_cam = 0;

  public:
  RawImage();

  ~RawImage();

  //accessors:
  int getWidth() const;
  int getHeight() const;
  ColorFormat getColorFormat() const;
  double getTime() const;
  double getTimeCam() const;
  unsigned char * getData() const;
  int getNumBytes() const;
  int getNumColorBlocks() const;
  int getNumPixels() const;

  rgb getRgb(int x, int y) const;
  yuv getYuv(int x, int y) const;

  //mutators:
  void setColorFormat(ColorFormat f);
  void setWidth(int w);
  void setHeight(int h);
  void setTime(double t);
  void setTimeCam(double t);
  void setData(unsigned char * d);
  void allocate (ColorFormat fmt, int w, int h);
  void ensure_allocation (ColorFormat fmt, int w, int h);
  void deepCopyFromRawImage(const RawImage & img, bool copyMetaData);
  void clear();

  //helpers:
  static int computeImageSize(ColorFormat fmt, int pixelCount);

};

#endif

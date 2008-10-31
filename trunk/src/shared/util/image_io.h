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
  \file    image_io.h
  \brief   C++ Interface: ImageIO
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#ifndef __IMAGE_IO_H__
#define __IMAGE_IO_H__

#include "colors.h"
#include "jpeglib.h"

/*!
  \class ImageIO
  \brief A class containing helper functions for reading and writing image data to/from files

  This class relies on QT4 as well as the standard jpeg lib.

  Note, that this class is not particularly clean, and needs some work!
*/
class ImageIO {
protected:
  static void copyBGRAtoRGBA(rgba * dst,unsigned char * src,unsigned int size);
  static void copyBGRtoRGB(rgb * dst,unsigned char * src,unsigned int size);
public:
  static unsigned char *readGrayscale(int &width,int &height, const char *filename);
  static rgb *readRGB(             int &width,int &height,const char *filename);
  static rgba *readRGBA(             int &width,int &height,const char *filename);
  static bool writeRGB(rgb *imgbuf,int  width,int  height,const char *filename);
  // manually selected format image writers
  static bool writePPM( rgb *imgbuf, int width, int height, const char *filename);
  static bool writeJPEG(rgb *imgbuf, int width, int height, const char *filename,
               int quality, bool flipY=false);


  //bool WriteJPEG(rgb *imgbuf, int width, int height, const char *filename,
  //               int quality = 90);
  //bool WritePNG( rgb *imgbuf, int width, int height, const char *filename);

};

#endif

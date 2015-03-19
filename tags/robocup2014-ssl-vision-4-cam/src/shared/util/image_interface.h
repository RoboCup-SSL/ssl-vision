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
  \file    image_interface.h
  \brief   C++ Interface: image_interface
  \author  Stefan Zickler, 2008
*/
//========================================================================
#ifndef __IMAGE_INTERFACE_H__
#define __IMAGE_INTERFACE_H__
#include "colors.h"

class ImageInterface
{
public:
  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
  virtual ColorFormat getColorFormat() const = 0;
  virtual unsigned char * getData() const = 0;
  virtual int getNumBytes() const = 0;
  virtual int getNumPixels() const = 0;
  virtual ~ImageInterface() {};
};

#endif

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
  \file    cmvision_threshold.cpp
  \brief   C++ Implementation: cmvision_threshold
  \author  James Bruce (Original CMVision implementation and algorithms),
           Some Code Restructuring, and data structure changes: Stefan Zickler 2008
*/
//========================================================================
#include "cmvision_threshold.h"

CMVisionThreshold::CMVisionThreshold()
{
}


CMVisionThreshold::~CMVisionThreshold()
{
}


void CMVisionThreshold::colorizeImageFromThresholding(rgbImage & target, const Image<raw8> & source, LUT3D * lut) {
  target.allocate(source.getWidth(),source.getHeight());
  int n = source.getNumPixels();

  rgb * vis_ptr = target.getPixelData();
  raw8 * seg_ptr = source.getPixelData();
  for (int i=0;i<n;i++) {
    vis_ptr[i]=lut->getChannel(seg_ptr[i].getIntensity()).draw_color;
  }
}

bool CMVisionThreshold::thresholdImageYUV422_UYVY(Image<raw8> * target, const RawImage * source, YUVLUT * lut) {
  if (source->getColorFormat()!=COLOR_YUV422_UYVY) {
    //TODO add YUV444 and maybe even 411 mode
    fprintf(stderr,"CMVision thresholdImageYUV422_UYVY assumes YUV422 as input, but found %s\n", Colors::colorFormatToString(source->getColorFormat()).c_str());
    return false;
  }

  register lut_mask_t * LUT = lut->getTable();

  register unsigned int          target_size    = target->getNumPixels();
  register uyvy *       source_pointer = (uyvy*)(source->getData());
  register raw8 *      target_pointer = target->getPixelData();

  if (target->getNumPixels() != source->getNumPixels()) {
    fprintf(stderr, "CMVision YUV422_UYVY thresholding: source (num=%d  w=%d  h=%d) and target (num=%d w=%d h=%d) pixel counts do not match!\n", source->getNumPixels(),source->getWidth(),source->getHeight(), target->getNumPixels(),target->getWidth(),target->getHeight());
    return false;
  }

  lut->lock();
  int X_SHIFT=lut->X_SHIFT;
  int Y_SHIFT=lut->Y_SHIFT;
  int Z_SHIFT=lut->Z_SHIFT;
  int Z_AND_Y_BITS=lut->Z_AND_Y_BITS;
  int Z_BITS = lut->Z_BITS;
  uyvy p;
  for (unsigned int i=0;i<target_size;i+=2) {
    p=source_pointer[(i >> 0x01)];
    register int B=((p.u >> Y_SHIFT) << Z_BITS);
    register int C=(p.v >> Z_SHIFT);
    target_pointer[i] =  LUT[(((p.y1 >> X_SHIFT) << Z_AND_Y_BITS) | B | C)];
    target_pointer[i+1] =  LUT[(((p.y2 >> X_SHIFT) << Z_AND_Y_BITS) | B | C)];
  }
  lut->unlock();
  //printf("time: %f\n",t.time());
  return true;
}

bool CMVisionThreshold::thresholdImageYUV444(Image<raw8> * target, const ImageInterface * source, YUVLUT * lut) {
  if (source->getColorFormat()!=COLOR_YUV444) {
    fprintf(stderr,"CMVision thresholdImageYUV444 assumes YUV444 as input, but found %s\n", Colors::colorFormatToString(source->getColorFormat()).c_str());
    return false;
  }

  register lut_mask_t * LUT = lut->getTable();

  register unsigned int          target_size    = target->getNumPixels();
  register yuv  *                source_pointer = (yuv*)(source->getData());
  register raw8 *                target_pointer = target->getPixelData();

  if (target->getNumPixels() != source->getNumPixels()) {
     fprintf(stderr, "CMVision YUV444 thresholding: source (num=%d  w=%d  h=%d) and target (num=%d w=%d h=%d) pixel counts do not match!\n", source->getNumPixels(),source->getWidth(),source->getHeight(), target->getNumPixels(),target->getWidth(),target->getHeight());
    return false;
  } 

  lut->lock();
  int X_SHIFT=lut->X_SHIFT;
  int Y_SHIFT=lut->Y_SHIFT;
  int Z_SHIFT=lut->Z_SHIFT;
  int Z_AND_Y_BITS=lut->Z_AND_Y_BITS;
  int Z_BITS = lut->Z_BITS;
  yuv p;
  for (unsigned int i=0;i<target_size;i++) {
    p=source_pointer[i];
    target_pointer[i] =  LUT[(((p.y >> X_SHIFT) << Z_AND_Y_BITS) | ((p.u >> Y_SHIFT) << Z_BITS) | (p.v >> Z_SHIFT))];
  }
  lut->unlock();

  return true;
}



bool CMVisionThreshold::thresholdImageRGB(Image<raw8> * target, const ImageInterface * source, RGBLUT * lut) {
  if (source->getColorFormat()!=COLOR_RGB8) {
    fprintf(stderr,"CMVision RGB thresholding assumes RGB8 as input, but found %s\n", Colors::colorFormatToString(source->getColorFormat()).c_str());
    return false;
  }

  register lut_mask_t * LUT = lut->getTable();
  int          source_size    = source->getNumPixels();
  rgb *        source_pointer = (rgb*)(source->getData());
  raw8 *      target_pointer = target->getPixelData();

  if (target->getNumPixels() != source->getNumPixels()) {
    fprintf(stderr, "CMVision RGB thresholding: source (num=%d  w=%d  h=%d) and target (num=%d w=%d h=%d) pixel counts do not match!\n", source->getNumPixels(),source->getWidth(),source->getHeight(), target->getNumPixels(),target->getWidth(),target->getHeight());
    return false;
  }

  int X_SHIFT=lut->X_SHIFT;
  int Y_SHIFT=lut->Y_SHIFT;
  int Z_SHIFT=lut->Z_SHIFT;
  int Z_AND_Y_BITS=lut->Z_AND_Y_BITS;
  int Z_BITS = lut->Z_BITS;
  for (int i=0;i<source_size;i++) {
    rgb p=source_pointer[i];
    target_pointer[i] =  LUT[(((p.r >> X_SHIFT) << Z_AND_Y_BITS) | ((p.g >> Y_SHIFT) << Z_BITS) | (p.b >> Z_SHIFT))];
  }

  return true;
}

//static void thresholdImage(Image * target, const Image<yuv> * source, const YUVLUT * lut);

//static void thresholdImage(Image * target, const Image<yuvy> * source, const YUVLUT * lut);

//static void thresholdImage(Image * target, const Image<uyvy> * source, const LUT3D * lut);

//typedef ColorYUYV<uint8_t,COLOR_YUV422_UYVY> yuyv;
//typedef ColorUYVY<uint8_t,COLOR_YUV422_UYVY> uyvy;

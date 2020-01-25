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
#ifdef __AVX2__
#include <x86intrin.h>
#endif

CMVisionThreshold::CMVisionThreshold()
{
}


CMVisionThreshold::~CMVisionThreshold()
{
}

bool CMVisionThreshold::thresholdImageYUV422_UYVY(Image<raw8> * target, const RawImage * source, YUVLUT * lut, const ImageInterface* mask) {
  if (source->getColorFormat()!=COLOR_YUV422_UYVY) {
    //TODO add YUV444 and maybe even 411 mode
    fprintf(stderr,"CMVision thresholdImageYUV422_UYVY assumes YUV422 as input, but found %s\n", Colors::colorFormatToString(source->getColorFormat()).c_str());
    return false;
  }

  register lut_mask_t * LUT = lut->getTable();

  register unsigned int          target_size    = target->getNumPixels();
  register uyvy *       source_pointer = (uyvy*)(source->getData());
  register raw8 *      target_pointer = target->getPixelData();
  register unsigned char *      mask_pointer = mask->getData();

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
    target_pointer[i] =  mask_pointer[i] & LUT[(((p.y1 >> X_SHIFT) << Z_AND_Y_BITS) | B | C)];
    target_pointer[i+1] =  mask_pointer[i+1] & LUT[(((p.y2 >> X_SHIFT) << Z_AND_Y_BITS) | B | C)];
  }
  lut->unlock();
  return true;
}

bool CMVisionThreshold::thresholdImageYUV444(Image<raw8> * target, const ImageInterface * source, YUVLUT * lut, const ImageInterface* mask) {
  if (source->getColorFormat()!=COLOR_YUV444) {
    fprintf(stderr,"CMVision thresholdImageYUV444 assumes YUV444 as input, but found %s\n", Colors::colorFormatToString(source->getColorFormat()).c_str());
    return false;
  }

  register lut_mask_t * LUT = lut->getTable();

  register unsigned int          target_size    = target->getNumPixels();
  register yuv  *                source_pointer = (yuv*)(source->getData());
  register raw8 *                target_pointer = target->getPixelData();
  register unsigned char *       mask_pointer = mask->getData();

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
    target_pointer[i] =  mask_pointer[i] & LUT[(((p.y >> X_SHIFT) << Z_AND_Y_BITS) | ((p.u >> Y_SHIFT) << Z_BITS) | (p.v >> Z_SHIFT))];
  }
  lut->unlock();

  return true;
}



bool CMVisionThreshold::thresholdImageRGB(Image<raw8> * target, const ImageInterface * source, RGBLUT * lut, const ImageInterface* mask) {
  if (source->getColorFormat()!=COLOR_RGB8) {
    fprintf(stderr,"CMVision RGB thresholding assumes RGB8 as input, but found %s\n", Colors::colorFormatToString(source->getColorFormat()).c_str());
    return false;
  }

  register lut_mask_t * LUT = lut->getTable();
  int source_size    = source->getNumPixels();
  const rgb * source_pointer = (const rgb*)(source->getData());
  auto * target_pointer = (uint8_t*) target->getPixelData();
  auto * mask_pointer = mask->getData();

  if (target->getNumPixels() != source->getNumPixels()) {
    fprintf(stderr, "CMVision RGB thresholding: source (num=%d  w=%d  h=%d) and target (num=%d w=%d h=%d) pixel counts do not match!\n", source->getNumPixels(),source->getWidth(),source->getHeight(), target->getNumPixels(),target->getWidth(),target->getHeight());
    return false;
  }

  int X_SHIFT=lut->X_SHIFT;
  int Y_SHIFT=lut->Y_SHIFT;
  int Z_SHIFT=lut->Z_SHIFT;
  int Z_AND_Y_BITS=lut->Z_AND_Y_BITS;
  int Z_BITS = lut->Z_BITS;

#ifdef __AVX2__
  // unpacking from: https://docs.google.com/presentation/d/1I0-SiHid1hTsv7tjLST2dYW5YF5AJVfs9l4Rg9rvz48/edit#slide=id.g1eefe20b_0_125
  __m128i ssse3_red_indeces_0 = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0);
  __m128i ssse3_red_indeces_1 = _mm_set_epi8(-1, -1, -1, -1, -1, 14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1);
  __m128i ssse3_red_indeces_2 = _mm_set_epi8(13, 10, 7, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
  __m128i ssse3_green_indeces_0 = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1);
  __m128i ssse3_green_indeces_1 = _mm_set_epi8(-1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1);
  __m128i ssse3_green_indeces_2 = _mm_set_epi8(14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
  __m128i ssse3_blue_indeces_0 = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5, 2);
  __m128i ssse3_blue_indeces_1 = _mm_set_epi8(-1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1, -1);
  __m128i ssse3_blue_indeces_2 = _mm_set_epi8(15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

  uint16_t idx[16];
  const rgb* p=&source_pointer[0];
  const uint8_t* source_pixel = (const uint8_t*)p;

  for (int i=0; i<source_size; i+=16) {

    // crazy RGB unpacking
    const __m128i chunk0 = _mm_loadu_si128((const __m128i*)(source_pixel));
    const __m128i chunk1 = _mm_loadu_si128((const __m128i*)(source_pixel + 16));
    const __m128i chunk2 = _mm_loadu_si128((const __m128i*)(source_pixel + 32));
    source_pixel += 48;

    const __m128i red = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_red_indeces_0),
                                                  _mm_shuffle_epi8(chunk1, ssse3_red_indeces_1)), _mm_shuffle_epi8(chunk2, ssse3_red_indeces_2));
    const __m128i green = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_green_indeces_0),
                                                    _mm_shuffle_epi8(chunk1, ssse3_green_indeces_1)), _mm_shuffle_epi8(chunk2, ssse3_green_indeces_2));
    const __m128i blue = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_blue_indeces_0),
                                                   _mm_shuffle_epi8(chunk1, ssse3_blue_indeces_1)), _mm_shuffle_epi8(chunk2, ssse3_blue_indeces_2));

    // widen pixel values to 16bit
    __m256i r = _mm256_cvtepu8_epi16(red);
    __m256i b = _mm256_cvtepu8_epi16(blue);
    __m256i g = _mm256_cvtepu8_epi16(green);

    // do the original shifts on 16 values in parallel
    __m256i rs = _mm256_slli_epi16(_mm256_srli_epi16(r, X_SHIFT), Z_AND_Y_BITS);
    __m256i gs = _mm256_slli_epi16(_mm256_srli_epi16(g, Y_SHIFT), Z_BITS);
    __m256i bs = _mm256_srli_epi16(b, Z_SHIFT);

    // construct LUT indices (ORing)
    __m256i result = _mm256_or_si256(rs, _mm256_or_si256(gs, bs));

    _mm256_storeu_si256((__m256i*)idx, result);

#pragma GCC unroll 16
    for(int j=0; j<16; j++) {
      target_pointer[i+j] = mask_pointer[i] & LUT[idx[j]];
    }
  }
#else
  #pragma GCC unroll 4
  for (int i=0; i<source_size; i++) {
    rgb p=source_pointer[i];
    target_pointer[i] = mask_pointer[i] & LUT[(((p.r >> X_SHIFT) << Z_AND_Y_BITS) | ((p.g >> Y_SHIFT) << Z_BITS) | (p.b >> Z_SHIFT))];
  }
#endif

  return true;
}

#pragma once

#include <image.h>

#include <opencv2/opencv.hpp>

#include "colors.h"
#include "rawimage.h"
#include "util.h"

class ConversionsGreyscale {
 public:
  static void cvColor2Grey(const RawImage& src, Image<raw8>* dst);
  static void cvColor2Grey(const RawImage& src, int src_data_format, Image<raw8>* dst,
                           cv::ColorConversionCodes conversion_code);
  static void cv16bit2_8bit(const RawImage& src, Image<raw8>* dst);
  static void manualColor2Grey(const RawImage& src, Image<raw8>* dst);
  static void copyData(const RawImage& src, Image<raw8>* dst);
};

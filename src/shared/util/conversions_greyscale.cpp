#include "conversions_greyscale.h"

void ConversionsGreyscale::cvColor2Grey(const RawImage &src, Image<raw8> *dst) {
  // Allocate image if not already allocated. Allocate method will do
  // nothing if data already allocated for correct width/height.
  dst->allocate(src.getWidth(), src.getHeight());

  switch (src.getColorFormat()) {
    case COLOR_RGB8:
      cvColor2Grey(src, CV_8UC3, dst, cv::COLOR_RGB2GRAY);
      break;
    case COLOR_RGBA8:
      cvColor2Grey(src, CV_8UC4, dst, cv::COLOR_RGBA2GRAY);
      break;
    case COLOR_YUV422_UYVY:
      cvColor2Grey(src, CV_8UC2, dst, cv::COLOR_YUV2GRAY_UYVY);
      break;
    case COLOR_YUV422_YUYV:
      cvColor2Grey(src, CV_8UC2, dst, cv::COLOR_YUV2GRAY_YUYV);
      break;
    case COLOR_RGB16:
      cvColor2Grey(src, CV_16UC3, dst, cv::COLOR_RGB2GRAY);
      break;
    case COLOR_RAW16:
      [[fallthrough]];
    case COLOR_MONO16:
      cv16bit2_8bit(src, dst);
      break;
    case COLOR_MONO8:
      [[fallthrough]];
    case COLOR_RAW8:
      // already in the correct format
      copyData(src, dst);
      break;
    case COLOR_YUV411:
      [[fallthrough]];
    case COLOR_YUV444:
      [[fallthrough]];
    default:
      manualColor2Grey(src, dst);
      break;
  }
}

void ConversionsGreyscale::cvColor2Grey(const RawImage &src, const int src_data_format, Image<raw8> *dst,
                                        const cv::ColorConversionCodes conversion_code) {
  cv::Mat srcMat(src.getWidth(), src.getHeight(), src_data_format, src.getData());
  cv::Mat dstMat(dst->getWidth(), dst->getHeight(), CV_8UC1, dst->getData());
  cv::cvtColor(srcMat, dstMat, conversion_code);
}

void ConversionsGreyscale::cv16bit2_8bit(const RawImage &src, Image<raw8> *dst) {
  cv::Mat srcMat(src.getWidth(), src.getHeight(), CV_16UC1, src.getData());
  cv::Mat dstMat(dst->getWidth(), dst->getHeight(), CV_8UC1, dst->getData());
  // convertTo drops higher bits. Need to rescale 16 bit values to
  // 8bit range. Should have a scale factor of 1/256. See
  // https://stackoverflow.com/a/10420743
  srcMat.convertTo(dstMat, CV_8U, 0.00390625);
}

void ConversionsGreyscale::manualColor2Grey(const RawImage &src, Image<raw8> *dst) {
  for (int i = 0; i < src.getWidth(); ++i) {
    for (int j = 0; j < src.getHeight(); ++j) {
      const auto pixel = src.getRgb(i, j);
      *(dst->getPixelPointer(i, j)) = (pixel.r + pixel.g + pixel.b) / 3;
    }
  }
}

void ConversionsGreyscale::copyData(const RawImage &src, Image<raw8> *dst) {
  cv::Mat srcMat(src.getWidth(), src.getHeight(), CV_8UC1, src.getData());
  cv::Mat dstMat(dst->getWidth(), dst->getHeight(), CV_8UC1, dst->getData());
  srcMat.copyTo(dstMat);
}

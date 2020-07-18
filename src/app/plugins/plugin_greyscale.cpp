#include "plugin_greyscale.h"
#include <colors.h>
#include <image.h>
#include <iostream>
#include <opencv2/opencv.hpp>

PluginGreyscale::PluginGreyscale(FrameBuffer *buffer)
    : VisionPlugin(buffer), settings(new VarList("Greyscale")) {}

ProcessResult PluginGreyscale::process(FrameData *data,
                                       RenderOptions *options) {

  Image<raw8> *img_greyscale;
  if ((img_greyscale = reinterpret_cast<Image<raw8> *>(
           data->map.get("greyscale"))) == nullptr) {
    img_greyscale = reinterpret_cast<Image<raw8> *>(
        data->map.insert("greyscale", new Image<raw8>()));
  }

  // Allocate image if not already allocated. Allocate method will do
  // nothing if data already allocated for correct width/height.
  img_greyscale->allocate(data->video.getWidth(), data->video.getHeight());

  switch (data->video.getColorFormat()) {
  case COLOR_RGB8:
    cvColor2Grey(data->video, CV_8UC3, img_greyscale, cv::COLOR_RGB2GRAY);
    break;
  case COLOR_RGBA8:
    cvColor2Grey(data->video, CV_8UC4, img_greyscale, cv::COLOR_RGBA2GRAY);
    break;
  case COLOR_YUV422_UYVY:
    cvColor2Grey(data->video, CV_8UC2, img_greyscale, cv::COLOR_YUV2GRAY_UYVY);
    break;
  case COLOR_YUV422_YUYV:
    cvColor2Grey(data->video, CV_8UC2, img_greyscale, cv::COLOR_YUV2GRAY_YUYV);
    break;
  case COLOR_RGB16:
    cvColor2Grey(data->video, CV_16UC3, img_greyscale, cv::COLOR_RGB2GRAY);
    break;
  case COLOR_RAW16:
    [[fallthrough]];
  case COLOR_MONO16:
    cv16bit2_8bit(data->video, img_greyscale);
    break;
  case COLOR_MONO8:
    [[fallthrough]];
  case COLOR_RAW8:
    // already in the correct format
    copyData(data->video, img_greyscale);
    break;
  case COLOR_YUV411:
    [[fallthrough]];
  case COLOR_YUV444:
    [[fallthrough]];
  default:
    manualColor2Grey(data->video, img_greyscale);
    break;
  }
  
  return ProcessingOk;
}

VarList *PluginGreyscale::getSettings() { return settings.get(); }

std::string PluginGreyscale::getName() { return "Greyscale"; }

void PluginGreyscale::cvColor2Grey(
    const RawImage &src, const int src_data_format, Image<raw8> *dst,
    const cv::ColorConversionCodes conversion_code) {
  cv::Mat srcMat(src.getWidth(), src.getHeight(), src_data_format,
                 src.getData());
  cv::Mat dstMat(dst->getWidth(), dst->getHeight(), CV_8UC1, dst->getData());
  cv::cvtColor(srcMat, dstMat, conversion_code);
}

void PluginGreyscale::cv16bit2_8bit(const RawImage &src, Image<raw8> *dst) {
  cv::Mat srcMat(src.getWidth(), src.getHeight(), CV_16UC1, src.getData());
  cv::Mat dstMat(dst->getWidth(), dst->getHeight(), CV_8UC1, dst->getData());
  // convertTo drops higher bits. Need to rescale 16 bit values to
  // 8bit range. Should have a scale factor of 1/256. See
  // https://stackoverflow.com/a/10420743
  srcMat.convertTo(dstMat, CV_8U, 0.00390625);
}

void PluginGreyscale::manualColor2Grey(const RawImage &src, Image<raw8> *dst) {
  for (int i = 0; i < src.getWidth(); ++i) {
    for (int j = 0; j < src.getHeight(); ++j) {
      const auto pixel = src.getRgb(i, j);
      *(dst->getPixelPointer(i, j)) = (pixel.r + pixel.g + pixel.b) / 3;
    }
  }
}

void PluginGreyscale::copyData(const RawImage &src, Image<raw8> *dst) {
  cv::Mat srcMat(src.getWidth(), src.getHeight(), CV_8UC1, src.getData());
  cv::Mat dstMat(dst->getWidth(), dst->getHeight(), CV_8UC1, dst->getData());
  srcMat.copyTo(dstMat);
}

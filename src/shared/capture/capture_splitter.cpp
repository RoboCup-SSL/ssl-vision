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

#include "capture_splitter.h"
#include <algorithm>
#include <opencv2/opencv.hpp>

#ifndef VDATA_NO_QT
CaptureSplitter::CaptureSplitter(VarList * _settings, int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
#else
CaptureSplitter::CaptureSplitter(VarList * _settings) : CaptureInterface(_settings)
#endif
{
  is_capturing=false;
  switch(default_camera_id)
  {
    case 0:
    default:
      settings->addChild(relative_height_offset = new VarDouble("Relative height offset", 0.0, 0.0, 1.0));
      settings->addChild(relative_width_offset = new VarDouble("Relative width offset", 0.0, 0.0, 1.0));
      break;
    case 1:
      settings->addChild(relative_height_offset = new VarDouble("Relative height offset", 0.4, 0.0, 1.0));
      settings->addChild(relative_width_offset = new VarDouble("Relative width offset", 0.0, 0.0, 1.0));
      break;
    case 2:
      settings->addChild(relative_height_offset = new VarDouble("Relative height offset", 0.0, 0.0, 1.0));
      settings->addChild(relative_width_offset = new VarDouble("Relative width offset", 0.4, 0.0, 1.0));
      break;
    case 3:
      settings->addChild(relative_height_offset = new VarDouble("Relative height offset", 0.4, 0.0, 1.0));
      settings->addChild(relative_width_offset = new VarDouble("Relative width offset", 0.4, 0.0, 1.0));
      break;
  }

  if(default_camera_id < 4) {
    settings->addChild(relative_height = new VarDouble("Relative height", 0.6, 0.0, 1.0));
    settings->addChild(relative_width = new VarDouble("Relative width", 0.6, 0.0, 1.0));
  } else {
    settings->addChild(relative_height = new VarDouble("Relative height", 1.0, 0.0, 1.0));
    settings->addChild(relative_width = new VarDouble("Relative width", 1.0, 0.0, 1.0));
  }

  image_buffer = new RawImage();
}

CaptureSplitter::~CaptureSplitter()
{
  image_buffer->clear();
  delete image_buffer;
}

bool CaptureSplitter::stopCapture()
{
  cleanup();
  return true;
}

void CaptureSplitter::cleanup()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  is_capturing=false;
  full_image_arrived_mutex.unlock();
  frame_processed_mutex.unlock();
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

bool CaptureSplitter::startCapture()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif

  is_capturing = true;
  full_image_arrived_mutex.try_lock();
  frame_processed_mutex.try_lock();

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

bool CaptureSplitter::copyAndConvertFrame(const RawImage &src, RawImage &target) {
#ifndef VDATA_NO_QT
  mutex.lock();
#endif

  if (src.getWidth() == 0 || src.getHeight() == 0) {
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  }
  target.setTime(src.getTime());

  // make sure we use values from 0 to 1
  double rel_height_offset = std::min(1.0, std::max(0.0, relative_height_offset->getDouble()));
  double rel_height = std::min(1.0, std::max(0.0, relative_height->getDouble()));
  double rel_width_offset = std::min(1.0, std::max(0.0, relative_width_offset->getDouble()));
  double rel_width = std::min(1.0, std::max(0.0, relative_width->getDouble()));

  // make sure we keep within the src image
  rel_height = std::min(rel_height, 1 - rel_height_offset);
  rel_width = std::min(rel_width, 1 - rel_width_offset);

  // calculate offset in src image
  int height_offset = (int) (src.getHeight() * rel_height_offset);
  int width_offset = (int) (src.getWidth() * rel_width_offset);

  // calculate dimensions of target image
  int height = (int) (src.getHeight() * rel_height);
  int width = (int) (src.getWidth() * rel_width);

  // for some reason, the image gets screwed when the width is not a multiple of 4
  width -= width % 4;

  // allocate target image
  image_buffer->ensure_allocation(src.getColorFormat(), width, height);

  if(image_buffer->getData() == nullptr)
  {
    std::cout << "Could not allocate image. Source color format '" << Colors::colorFormatToString(src.getColorFormat()) << "' not supported?" << std::endl;
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  }

  // copy the respective part of the source image
  unsigned char *data_buf = image_buffer->getData();
  unsigned char *data_src = src.getData();
  auto pixel_size = (size_t) RawImage::computeImageSize(image_buffer->getColorFormat(), 1);
  for (int i = 0; i < height; i++) {
    memcpy(
            data_buf + i * width * pixel_size,
            data_src + ((i + height_offset) * src.getWidth() + width_offset) * pixel_size,
            pixel_size * width);
  }

  target.ensure_allocation(ColorFormat::COLOR_RGB8, width, height);

  if(image_buffer->getData() == nullptr)
  {
    std::cout << "Could not allocate image. Target color format '" << Colors::colorFormatToString(target.getColorFormat()) << "' not supported?" << std::endl;
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  }

  if(image_buffer->getColorFormat() == ColorFormat::COLOR_RAW8) {
  cv::Mat srcMat(height, width, CV_8UC1, data_buf);
  cv::Mat dstMat(height, width, CV_8UC3, target.getData());
  cvtColor(srcMat, dstMat, cv::COLOR_BayerBG2RGB);
  } else if(image_buffer->getColorFormat() != ColorFormat::COLOR_RGB8)
  {
    std::cout << "Unsupported image format: " << Colors::colorFormatToString(image_buffer->getColorFormat()) << std::endl;
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  }

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

RawImage CaptureSplitter::getFrame()
{
#ifndef VDATA_NO_QT
   mutex.lock();
#endif

  full_image_arrived_mutex.lock();

  RawImage frame;
  if(full_image != nullptr)
  {
    frame = *full_image;
  }

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return frame;
}

void CaptureSplitter::releaseFrame()
{
  frame_processed_mutex.unlock();
}

string CaptureSplitter::getCaptureMethodName() const
{
  return "Splitter";
}

void CaptureSplitter::onNewFrame(RawImage* image)
{
  if(isCapturing()) {
    full_image = image;
    full_image_arrived_mutex.unlock();
  }
}

void CaptureSplitter::waitUntilFrameProcessed()
{
  if(isCapturing()) {
	frame_processed_mutex.lock();
  }
}

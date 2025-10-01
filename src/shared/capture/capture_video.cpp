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
  \file    capture_video.cpp
  \brief   C++ Implementation: CaptureVideo
  \author  Felix Weinmann, (C) 2024
*/
//========================================================================

#include "capture_video.h"

#include <opencv2/imgproc.hpp>

CaptureVideo::CaptureVideo(VarList* settings) : CaptureInterface(settings) {
  settings->addChild(v_cap_file = new VarString("file", ""));
  settings->addChild(v_cap_upscale = new VarBool("upscale", false));
}


RawImage CaptureVideo::getFrame() {
  if(!capture.read(frame)) {
    std::cout << "End of video stream reached" << std::endl;
    return img;
  }

  int factor = v_cap_upscale->getBool() ? 2 : 1;
  img.ensure_allocation(ColorFormat::COLOR_RGB8, factor*frame.cols, factor*frame.rows);
  cv::Mat dstImg(img.getHeight(), img.getWidth(), CV_8UC3, img.getData());

  /* ssl-vision expects typical color images scaled up from the raw bayer matrix
   * (4 sensor fields RG-GB are interpolated into four RGB pixels).
   * To support downscaled image formats (4 color fields RG-GB into 1 RGB value)
   * where ssl-vision needs the interpolated resolution to work as usual
   * this upscaling option has been added. */
  if (v_cap_upscale->getBool()) {
    cv::resize(frame, dstImg, dstImg.size());
    cvtColor(dstImg, dstImg, cv::COLOR_BGR2RGB);
  } else {
    // convert to default ssl-vision format (RGB8)
    cvtColor(frame, dstImg, cv::COLOR_BGR2RGB);
  }

  timestamp += 1.0/capture.get(cv::CAP_PROP_FPS);
  img.setTimeCam(timestamp);

  return img;
}

void CaptureVideo::releaseFrame() {
  // frame and img are not cleared here to prevent unnecessary reallocation with the next frame.
}


bool CaptureVideo::startCapture() {
  timestamp = 0.0;
  frame = cv::Mat();
  capture = cv::VideoCapture(v_cap_file->getString());
  is_capturing = capture.isOpened();
  return is_capturing;
}

bool CaptureVideo::stopCapture() {
  capture.release();
  frame.release();
  is_capturing = false;
  return !is_capturing;
}


bool CaptureVideo::isCapturing() {
  return is_capturing;
}

string CaptureVideo::getCaptureMethodName() const {
  return "Video";
}

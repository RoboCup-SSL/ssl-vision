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
  \file    capture_video.h
  \brief   C++ Interface: CaptureVideo
  \author  Felix Weinmann, (C) 2024
*/
//========================================================================

#ifndef SSL_VISION_CAPTURE_VIDEO_H
#define SSL_VISION_CAPTURE_VIDEO_H

#include <opencv2/videoio.hpp>

#include "captureinterface.h"

class CaptureVideo : public CaptureInterface {
 public:
  explicit CaptureVideo(VarList* settings);

  RawImage getFrame() override;
  bool isCapturing() override;
  void releaseFrame() override;
  bool startCapture() override;
  bool stopCapture() override;
  string getCaptureMethodName() const override;

 private:
  VarString* v_cap_file;
  VarBool* v_cap_upscale;

  cv::Mat frame;
  cv::VideoCapture capture;
  RawImage img;
  double timestamp;

  bool is_capturing = false;
};

#endif  // SSL_VISION_CAPTURE_VIDEO_H

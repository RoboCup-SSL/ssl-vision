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
  \file    capture_vapix.cpp
  \brief   C++ Implementation: CaptureVapix
  \author  Sebastian Olsson, (C) 2023
*/
//========================================================================

#include "capture_vapix.h"
#include <memory>

int CaptureVapix::num_cams = 0;

CaptureVapix::CaptureVapix(VarList *_settings, int default_camera_id, QObject *parent) : QObject(parent), CaptureInterface(_settings) {
  is_capturing = false;


  mutex.lock();

  num_cams++;
  settings->addChild(capture_settings = new VarList("Capture Settings"));
  settings->addChild(dcam_parameters = new VarList("Camera Parameters"));

  //=======================CAPTURE SETTINGS==========================
  //TODO: Keep?: v_cam_network_prefix = new VarString("cam network prefix", default_network_prefix);

  v_cam_bus = new VarInt("cam idx", default_camera_id);
  v_cam_ip = new VarString("cam ip", "");
  v_cam_port = new VarString("cam port", "");
  v_cam_username = new VarString("cam username", "");
  v_cam_password = new VarString("cam password", "");

  v_convert_to_mode = new VarStringEnum("convert to mode", Colors::colorFormatToString(COLOR_RGB8));
  v_convert_to_mode->addItem(Colors::colorFormatToString(COLOR_RAW8));
  v_convert_to_mode->addItem(Colors::colorFormatToString(COLOR_RGB8));

  capture_settings->addChild(v_cam_bus);
  capture_settings->addChild(v_cam_ip);
  capture_settings->addChild(v_cam_port);
  capture_settings->addChild(v_cam_username);
  capture_settings->addChild(v_cam_password);
  capture_settings->addChild(v_convert_to_mode);

  // //=======================CAMERA SETTINGS==========================
  // v_acquisition = new VarBool("Acquisition", true);

  v_capture_mode = new VarStringEnum("capture mode", Colors::colorFormatToString(COLOR_RAW8));
  v_capture_mode->addItem(Colors::colorFormatToString(COLOR_RAW8));

  // v_expose_auto = new VarStringEnum("Auto Expose", toString(Spinnaker::ExposureAuto_Off));
  // v_expose_auto->addItem(toString(Spinnaker::ExposureAuto_Off));
  // v_expose_auto->addItem(toString(Spinnaker::ExposureAuto_Once));
  // v_expose_auto->addItem(toString(Spinnaker::ExposureAuto_Continuous));
  // v_expose_us = new VarDouble("Expose [us]", 8000, 10, 100000);

  // v_gain_auto = new VarStringEnum("Auto Gain", toString(Spinnaker::GainAuto_Off));
  // v_gain_auto->addItem(toString(Spinnaker::GainAuto_Off));
  // v_gain_auto->addItem(toString(Spinnaker::GainAuto_Once));
  // v_gain_auto->addItem(toString(Spinnaker::GainAuto_Continuous));
  // v_gain_db = new VarDouble("Gain [dB]", 12.0, 0.0, 12.0);

  // v_gamma = new VarDouble("Gamma", 0.4, 0.25, 4.0);
  // v_gamma_enabled = new VarBool("Gamma enabled", false);

  // v_white_balance_auto = new VarStringEnum("Auto White Balance", toString(Spinnaker::BalanceWhiteAuto_Off));
  // v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Off));
  // v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Once));
  // v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Continuous));

  // v_white_balance_red = new VarDouble("White Balance Red", 2, 0.125, 8);
  // v_white_balance_blue = new VarDouble("White Balance Blue", 2, 0.125, 8);

  // v_image_width = new VarInt("Image Width", 0);
  // v_image_height = new VarInt("Image Height", 0);
  // v_image_offset_x = new VarInt("Image Offset X", 0);
  // v_image_offset_y = new VarInt("Image Offset Y", 0);

  // v_frame_rate_enable = new VarBool("Limit Frame Rate", false);
  // v_frame_rate = new VarDouble("Frame Rate", 0.0);
  // v_frame_rate_result = new VarDouble("Resulting Frame Rate", 0.0);

  // v_trigger_reset = new VarTrigger("Reset Parameters", "Reset");

  // dcam_parameters->addChild(v_acquisition);
  dcam_parameters->addChild(v_capture_mode);
  // dcam_parameters->addChild(v_expose_auto);
  // dcam_parameters->addChild(v_expose_us);
  // dcam_parameters->addChild(v_gain_auto);
  // dcam_parameters->addChild(v_gain_db);
  // dcam_parameters->addChild(v_gamma);
  // dcam_parameters->addChild(v_gamma_enabled);
  // dcam_parameters->addChild(v_white_balance_auto);
  // dcam_parameters->addChild(v_white_balance_red);
  // dcam_parameters->addChild(v_white_balance_blue);
  // dcam_parameters->addChild(v_image_width);
  // dcam_parameters->addChild(v_image_height);
  // dcam_parameters->addChild(v_image_offset_x);
  // dcam_parameters->addChild(v_image_offset_y);
  // dcam_parameters->addChild(v_frame_rate_enable);
  // dcam_parameters->addChild(v_frame_rate);
  // dcam_parameters->addChild(v_frame_rate_result);
  // dcam_parameters->addChild(v_trigger_reset);

  // mvc_connect(dcam_parameters);
  mutex.unlock();
}

CaptureVapix::~CaptureVapix() {
  capture_settings->deleteAllChildren();
  // dcam_parameters->deleteAllChildren();
  num_cams--;
}

// void CaptureVapix::reloadParameters() {
//   writeParameterValues();
//   readParameterValues();
// }

void CaptureVapix::slotResetTriggered() {
  mutex.lock();

  reset_parameters = true;

  mutex.unlock();
}

// void CaptureVapix::readAllParameterValues() {
//   mutex.lock();
//   readParameterValues();
//   mutex.unlock();
// }

// void CaptureVapix::init_camera() {
//   //
// }

// void CaptureVapix::readParameterValues() {

  // Currently our cameras does not support getting parameters due to extremely old firmware...

  // if (!isCapturing())
  //   return;

  // try {
  //   getCameraValueFloat(pCam->ExposureTime, v_expose_us);
  //   v_expose_auto->setString(toString(pCam->ExposureAuto.GetValue()));

  //   getCameraValueFloat(pCam->Gain, v_gain_db);
  //   v_gain_auto->setString(toString(pCam->GainAuto.GetValue()));

  //   v_white_balance_auto->setString(toString(pCam->BalanceWhiteAuto.GetValue()));
  //   pCam->BalanceRatioSelector.SetValue(Spinnaker::BalanceRatioSelector_Blue);
  //   getCameraValueFloat(pCam->BalanceRatio, v_white_balance_blue);
  //   pCam->BalanceRatioSelector.SetValue(Spinnaker::BalanceRatioSelector_Red);
  //   getCameraValueFloat(pCam->BalanceRatio, v_white_balance_red);

  //   getCameraValueInt(pCam->Width, v_image_width);
  //   getCameraValueInt(pCam->Height, v_image_height);
  //   getCameraValueInt(pCam->OffsetX, v_image_offset_x);
  //   getCameraValueInt(pCam->OffsetY, v_image_offset_y);

  //   v_gamma_enabled->setBool(pCam->GammaEnable.GetValue());
  //   getCameraValueFloat(pCam->Gamma, v_gamma);

  //   v_frame_rate_enable->setBool(pCam->AcquisitionFrameRateEnable.GetValue());
  //   getCameraValueFloat(pCam->AcquisitionFrameRate, v_frame_rate);
  //   getCameraValueFloat(pCam->AcquisitionResultingFrameRate, v_frame_rate_result);

  //   if (IsWritable(pCam->UserSetLoad)) {
  //     v_trigger_reset->removeFlags(VARTYPE_FLAG_READONLY);
  //   } else {
  //     v_trigger_reset->addFlags(VARTYPE_FLAG_READONLY);
  //   }
  // }
  // catch (Spinnaker::Exception &e) {
  //   fprintf(stderr, "Spinnaker: Could not read parameters: %s\n", e.GetFullErrorMessage());
  // }
// }

// void CaptureVapix::writeParameterValues() {
//   if (!isCapturing())
//     return;

//   if (reset_parameters) {
//     try {
//       // Set default startup profile to default (ssl-vision stores and sets all necessary parameters on start)
//       pCam->UserSetDefault.SetValue(Spinnaker::UserSetDefault_Default);

//       // Load default user set
//       pCam->UserSetSelector.SetValue(Spinnaker::UserSetSelector_Default);

//       if (IsWritable(pCam->UserSetLoad)) {
//         pCam->UserSetLoad.Execute();
//         init_camera();
//       }
//     }
//     catch (Spinnaker::Exception &e) {
//       fprintf(stderr, "Spinnaker: Could not reset parameters: %s\n", e.GetFullErrorMessage());
//     }
//     reset_parameters = false;
//     return;
//   }

//   try {
//     if (IsWritable(pCam->PixelFormat)) {
//       ColorFormat out_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
//       if (out_color == COLOR_RAW8) {
//         pCam->PixelFormat.SetValue(Spinnaker::PixelFormat_BayerRG8);
//       } else {
//         fprintf(stderr, "Spinnaker: Color format not supported: %s\n", Colors::colorFormatToString(COLOR_RAW8).c_str());
//       }
//       v_capture_mode->removeFlags(VARTYPE_FLAG_READONLY);
//     } else {
//       v_capture_mode->addFlags(VARTYPE_FLAG_READONLY);
//     }

//     pCam->ExposureAuto.SetValue(stringToExposureAuto(v_expose_auto->getString().c_str()));
//     setCameraValueFloat(pCam->ExposureTime, v_expose_us);

//     pCam->GainAuto.SetValue(stringToGainAuto(v_gain_auto->getString().c_str()));
//     setCameraValueFloat(pCam->Gain, v_gain_db);

//     pCam->GammaEnable.SetValue(v_gamma_enabled->getBool());
//     setCameraValueFloat(pCam->Gamma, v_gamma);

//     pCam->BalanceWhiteAuto.SetValue(stringToBalanceWhiteAuto(v_white_balance_auto->getString().c_str()));
//     pCam->BalanceRatioSelector.SetValue(Spinnaker::BalanceRatioSelector_Blue);
//     setCameraValueFloat(pCam->BalanceRatio, v_white_balance_blue);
//     pCam->BalanceRatioSelector.SetValue(Spinnaker::BalanceRatioSelector_Red);
//     setCameraValueFloat(pCam->BalanceRatio, v_white_balance_red);

//     setCameraValueInt(pCam->OffsetX, v_image_offset_x);
//     setCameraValueInt(pCam->OffsetY, v_image_offset_y);
//     setCameraValueInt(pCam->Width, v_image_width);
//     setCameraValueInt(pCam->Height, v_image_height);

//     pCam->AcquisitionFrameRateEnable.SetValue(v_frame_rate_enable->getBool());
//     setCameraValueFloat(pCam->AcquisitionFrameRate, v_frame_rate);

//     pCam->TLStream.StreamBufferHandlingMode.SetValue(Spinnaker::StreamBufferHandlingMode_NewestOnly);
//     pCam->TLStream.StreamBufferCountManual.SetValue(pCam->TLStream.StreamBufferCountManual.GetMin());
//   }
//   catch (Spinnaker::Exception &e) {
//     fprintf(stderr, "Spinnaker: Could not write parameters: %s\n", e.GetFullErrorMessage());
//   }
// }

bool CaptureVapix::stopCapture() {
  if (isCapturing()) {
    is_capturing = false;
    camera.release();
    fprintf(stderr, "VAPIX: Stopping capture on cam %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
  }

  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i: tmp) {
    i->removeFlags(VARTYPE_FLAG_READONLY);
  }

  return true;
}

bool CaptureVapix::startCapture() {
  mutex.lock();


  fprintf(stderr, "VAPIX: Starting capture on camera %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
  
  //pSystem = Spinnaker::System::GetInstance();

  //Spinnaker::CameraList camList = pSystem->GetCameras();
  fprintf(stderr, "VAPIX: Number of cams: %d\n", num_cams);

  if (v_cam_bus->getInt() >= num_cams) {
    fprintf(stderr, "VAPIX: Invalid cam_id: %u\n", v_cam_bus->getInt());
    //pSystem->ReleaseInstance();
    mutex.unlock();
    return false;
  }
  // rtsp://root:root@192.168.1.1/axis-media/media.amp
  string rtsp_url = "rtsp://" + v_cam_username->getString() + ":" + v_cam_password->getString() + "@" + v_cam_ip->getString() + ":" + v_cam_port->getString() + video_stream_route;
  bool success = camera.open(rtsp_url);
  
  if (!success) {
    fprintf(stderr, "VAPIX: Camera %d could not connect to %s\n", v_cam_bus->getInt(), rtsp_url.c_str());
    camera.release();
    mutex.unlock();
    return false;
  }
  
  // is_capturing must be set before reloadParameters()!
  is_capturing = true;

  fprintf(stderr, "VAPIX: Camera %d connected to IP %s successfully!\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());

  // init_camera();

  //reloadParameters();


  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i: tmp) {
    i->addFlags(VARTYPE_FLAG_READONLY);
  }

  mutex.unlock();

  return true;
}

bool CaptureVapix::copyAndConvertFrame(const RawImage &src, RawImage &target) {
  mutex.lock();
  if (src.getData() == nullptr) {
    mutex.unlock();
    return false;
  }
  target.setTime(src.getTime());
  target.setTimeCam ( src.getTimeCam() );

  ColorFormat src_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
  ColorFormat out_color = Colors::stringToColorFormat(v_convert_to_mode->getSelection().c_str());
  if (src_color == out_color) {
    // We have to copy the data here to avoid a potential double free
    // If we do 'target = src', both RawImages own the same data, and if both try to free/delete it,
    // i.e. due to an ensure_allocation(), a double free error will occur
    target.deepCopyFromRawImage(src, false);
  } else if (src_color == COLOR_RAW8 && out_color == COLOR_RGB8) {
    target.ensure_allocation(out_color, src.getWidth(), src.getHeight());
    cv::Mat srcMat(src.getHeight(), src.getWidth(), CV_8UC1, src.getData());
    cv::Mat dstMat(target.getHeight(), target.getWidth(), CV_8UC3, target.getData());
    cvtColor(srcMat, dstMat, cv::COLOR_BayerBG2RGB);
  } else {
    fprintf(stderr, "Invalid conversion from %s to %s\n",
            v_capture_mode->getSelection().c_str(), v_convert_to_mode->getSelection().c_str());
    mutex.unlock();
    return false;
  }

  mutex.unlock();
  return true;
}

RawImage CaptureVapix::getFrame() {
  mutex.lock();

  ColorFormat out_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
  RawImage result;
  result.setColorFormat(COLOR_RGB8);

  //fprintf(stderr, "%s", v_capture_mode->getSelection().c_str());
  // cv::Mat image;
  
  bool success = camera.read(p_image);
  //image = cv::imread("image.jpg");
  //cv::cvtColor(image, p_image, cv::COLOR_BGR2RGB);
  //cv::imwrite("test_image.jpg", image);

  
  if (success) {
    if (!p_image.empty()) {
        result.setData(p_image.data);
        result.setWidth(p_image.cols);
        result.setHeight(p_image.rows);
        // Ignoring timestamp ATM since I don't know if we actually receive one...
        //result.setTimeCam
      }
      else {
        fprintf(stderr, "VAPIX: Failed to read image from camera %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
      }

  }
  else {
    fprintf(stderr, "VAPIX: Could not read image from camera %d - %s\n", v_cam_bus->getInt(), v_cam_ip->getString().c_str());
  }

  mutex.unlock();
  return result;
}

void CaptureVapix::releaseFrame() {
  mutex.lock();
  p_image.release();
  mutex.unlock();
}


void CaptureVapix::changed(__attribute__((unused)) VarType *group) {
  mutex.lock();
  // TODO: Do something...
  mutex.unlock();
}

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
  \file    capture_spinnaker.cpp
  \brief   C++ Implementation: CaptureSpinnaker
  \author  Nicolai Ommer, (C) 2019
*/
//========================================================================

#include "capture_spinnaker.h"
#include <memory>
#include <opencv2/opencv.hpp>

CaptureSpinnaker::CaptureSpinnaker(VarList *_settings, int default_camera_id, QObject *parent) : QObject(parent), CaptureInterface(_settings) {
  is_capturing = false;

  mutex.lock();

  settings->addChild(capture_settings = new VarList("Capture Settings"));
  settings->addChild(dcam_parameters = new VarList("Camera Parameters"));

  //=======================CAPTURE SETTINGS==========================
  v_cam_bus = new VarInt("cam idx", default_camera_id);

  v_convert_to_mode = new VarStringEnum("convert to mode", Colors::colorFormatToString(COLOR_RGB8));
  v_convert_to_mode->addItem(Colors::colorFormatToString(COLOR_RAW8));
  v_convert_to_mode->addItem(Colors::colorFormatToString(COLOR_RGB8));

  capture_settings->addChild(v_cam_bus);
  capture_settings->addChild(v_convert_to_mode);

  //=======================CAMERA SETTINGS==========================
  v_acquisition = new VarBool("Acquisition", true);

  v_capture_mode = new VarStringEnum("capture mode", Colors::colorFormatToString(COLOR_RAW8));
  v_capture_mode->addItem(Colors::colorFormatToString(COLOR_RAW8));

  v_expose_auto = new VarStringEnum("Auto Expose", toString(Spinnaker::ExposureAuto_Off));
  v_expose_auto->addItem(toString(Spinnaker::ExposureAuto_Off));
  v_expose_auto->addItem(toString(Spinnaker::ExposureAuto_Once));
  v_expose_auto->addItem(toString(Spinnaker::ExposureAuto_Continuous));
  v_expose_us = new VarDouble("Expose [us]", 8000, 10, 100000);

  v_gain_auto = new VarStringEnum("Auto Gain", toString(Spinnaker::GainAuto_Off));
  v_gain_auto->addItem(toString(Spinnaker::GainAuto_Off));
  v_gain_auto->addItem(toString(Spinnaker::GainAuto_Once));
  v_gain_auto->addItem(toString(Spinnaker::GainAuto_Continuous));
  v_gain_db = new VarDouble("Gain [dB]", 12.0, 0.0, 12.0);

  v_gamma = new VarDouble("Gamma", 0.4, 0.25, 4.0);
  v_gamma_enabled = new VarBool("Gamma enabled", false);

  v_white_balance_auto = new VarStringEnum("Auto White Balance", toString(Spinnaker::BalanceWhiteAuto_Off));
  v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Off));
  v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Once));
  v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Continuous));

  v_white_balance_red = new VarDouble("White Balance Red", 2, 0.125, 8);
  v_white_balance_blue = new VarDouble("White Balance Blue", 2, 0.125, 8);

  v_image_width = new VarInt("Image Width", 0);
  v_image_height = new VarInt("Image Height", 0);
  v_image_offset_x = new VarInt("Image Offset X", 0);
  v_image_offset_y = new VarInt("Image Offset Y", 0);

  v_frame_rate_enable = new VarBool("Limit Frame Rate", false);
  v_frame_rate = new VarDouble("Frame Rate", 0.0);
  v_frame_rate_result = new VarDouble("Resulting Frame Rate", 0.0);

  v_trigger_reset = new VarTrigger("Reset Parameters", "Reset");

  dcam_parameters->addChild(v_acquisition);
  dcam_parameters->addChild(v_capture_mode);
  dcam_parameters->addChild(v_expose_auto);
  dcam_parameters->addChild(v_expose_us);
  dcam_parameters->addChild(v_gain_auto);
  dcam_parameters->addChild(v_gain_db);
  dcam_parameters->addChild(v_gamma);
  dcam_parameters->addChild(v_gamma_enabled);
  dcam_parameters->addChild(v_white_balance_auto);
  dcam_parameters->addChild(v_white_balance_red);
  dcam_parameters->addChild(v_white_balance_blue);
  dcam_parameters->addChild(v_image_width);
  dcam_parameters->addChild(v_image_height);
  dcam_parameters->addChild(v_image_offset_x);
  dcam_parameters->addChild(v_image_offset_y);
  dcam_parameters->addChild(v_frame_rate_enable);
  dcam_parameters->addChild(v_frame_rate);
  dcam_parameters->addChild(v_frame_rate_result);
  dcam_parameters->addChild(v_trigger_reset);

  mvc_connect(dcam_parameters);
  mutex.unlock();
}


CaptureSpinnaker::~CaptureSpinnaker() {
  capture_settings->deleteAllChildren();
  dcam_parameters->deleteAllChildren();
}

void CaptureSpinnaker::mvc_connect(VarList *group) {
  for (auto &i: group->getChildren()) {
    connect(i, SIGNAL(wasEdited(VarType * )), group, SLOT(mvcEditCompleted()));
  }
  connect(group, SIGNAL(wasEdited(VarType * )), this, SLOT(changed(VarType * )));
  connect(v_trigger_reset, SIGNAL(signalTriggered()), this, SLOT(slotResetTriggered()));
}

void CaptureSpinnaker::changed(__attribute__((unused)) VarType *group) {
  mutex.lock();
  reloadParameters();
  mutex.unlock();
}

void CaptureSpinnaker::reloadParameters() {
  writeParameterValues();
  readParameterValues();
}

void CaptureSpinnaker::slotResetTriggered() {
  mutex.lock();

  reset_parameters = true;

  mutex.unlock();
}

void CaptureSpinnaker::readAllParameterValues() {
  mutex.lock();
  readParameterValues();
  mutex.unlock();
}

void CaptureSpinnaker::init_camera() {
  pCam->TriggerMode.SetValue(Spinnaker::TriggerMode_Off);
  pCam->AcquisitionMode.SetValue(Spinnaker::AcquisitionMode_Continuous);

  if (IsWritable(pCam->GevSCPSPacketSize)) {
    pCam->GevSCPSPacketSize.SetValue(9000);
  }

  if (v_image_width->getInt() == 0) {
    v_image_width->setInt((int) pCam->WidthMax.GetValue());
  }
  if (v_image_height->getInt() == 0) {
    v_image_height->setInt((int) pCam->HeightMax.GetValue());
  }
}

void CaptureSpinnaker::readParameterValues() {
  if (!isCapturing())
    return;

  try {
    getCameraValueFloat(pCam->ExposureTime, v_expose_us);
    v_expose_auto->setString(toString(pCam->ExposureAuto.GetValue()));

    getCameraValueFloat(pCam->Gain, v_gain_db);
    v_gain_auto->setString(toString(pCam->GainAuto.GetValue()));

    v_white_balance_auto->setString(toString(pCam->BalanceWhiteAuto.GetValue()));
    pCam->BalanceRatioSelector.SetValue(Spinnaker::BalanceRatioSelector_Blue);
    getCameraValueFloat(pCam->BalanceRatio, v_white_balance_blue);
    pCam->BalanceRatioSelector.SetValue(Spinnaker::BalanceRatioSelector_Red);
    getCameraValueFloat(pCam->BalanceRatio, v_white_balance_red);

    getCameraValueInt(pCam->Width, v_image_width);
    getCameraValueInt(pCam->Height, v_image_height);
    getCameraValueInt(pCam->OffsetX, v_image_offset_x);
    getCameraValueInt(pCam->OffsetY, v_image_offset_y);

    v_gamma_enabled->setBool(pCam->GammaEnable.GetValue());
    getCameraValueFloat(pCam->Gamma, v_gamma);

    v_frame_rate_enable->setBool(pCam->AcquisitionFrameRateEnable.GetValue());
    getCameraValueFloat(pCam->AcquisitionFrameRate, v_frame_rate);
    getCameraValueFloat(pCam->AcquisitionResultingFrameRate, v_frame_rate_result);

    if (IsWritable(pCam->UserSetLoad)) {
      v_trigger_reset->removeFlags(VARTYPE_FLAG_READONLY);
    } else {
      v_trigger_reset->addFlags(VARTYPE_FLAG_READONLY);
    }
  }
  catch (Spinnaker::Exception &e) {
    fprintf(stderr, "Spinnaker: Could not read parameters: %s\n", e.GetFullErrorMessage());
  }
}

void CaptureSpinnaker::writeParameterValues() {
  if (!isCapturing())
    return;

  if (reset_parameters) {
    try {
      // Set default startup profile to default (ssl-vision stores and sets all necessary parameters on start)
      pCam->UserSetDefault.SetValue(Spinnaker::UserSetDefault_Default);

      // Load default user set
      pCam->UserSetSelector.SetValue(Spinnaker::UserSetSelector_Default);

      if (IsWritable(pCam->UserSetLoad)) {
        pCam->UserSetLoad.Execute();
        init_camera();
      }
    }
    catch (Spinnaker::Exception &e) {
      fprintf(stderr, "Spinnaker: Could not reset parameters: %s\n", e.GetFullErrorMessage());
    }
    reset_parameters = false;
    return;
  }

  try {
    if (IsWritable(pCam->PixelFormat)) {
      ColorFormat out_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
      if (out_color == COLOR_RAW8) {
        pCam->PixelFormat.SetValue(Spinnaker::PixelFormat_BayerRG8);
      } else {
        fprintf(stderr, "Spinnaker: Color format not supported: %s\n", Colors::colorFormatToString(COLOR_RAW8).c_str());
      }
      v_capture_mode->removeFlags(VARTYPE_FLAG_READONLY);
    } else {
      v_capture_mode->addFlags(VARTYPE_FLAG_READONLY);
    }

    pCam->ExposureAuto.SetValue(stringToExposureAuto(v_expose_auto->getString().c_str()));
    setCameraValueFloat(pCam->ExposureTime, v_expose_us);

    pCam->GainAuto.SetValue(stringToGainAuto(v_gain_auto->getString().c_str()));
    setCameraValueFloat(pCam->Gain, v_gain_db);

    pCam->GammaEnable.SetValue(v_gamma_enabled->getBool());
    setCameraValueFloat(pCam->Gamma, v_gamma);

    pCam->BalanceWhiteAuto.SetValue(stringToBalanceWhiteAuto(v_white_balance_auto->getString().c_str()));
    pCam->BalanceRatioSelector.SetValue(Spinnaker::BalanceRatioSelector_Blue);
    setCameraValueFloat(pCam->BalanceRatio, v_white_balance_blue);
    pCam->BalanceRatioSelector.SetValue(Spinnaker::BalanceRatioSelector_Red);
    setCameraValueFloat(pCam->BalanceRatio, v_white_balance_red);

    setCameraValueInt(pCam->OffsetX, v_image_offset_x);
    setCameraValueInt(pCam->OffsetY, v_image_offset_y);
    setCameraValueInt(pCam->Width, v_image_width);
    setCameraValueInt(pCam->Height, v_image_height);

    pCam->AcquisitionFrameRateEnable.SetValue(v_frame_rate_enable->getBool());
    setCameraValueFloat(pCam->AcquisitionFrameRate, v_frame_rate);

    pCam->TLStream.StreamBufferHandlingMode.SetValue(Spinnaker::StreamBufferHandlingMode_NewestOnly);
    pCam->TLStream.StreamBufferCountManual.SetValue(pCam->TLStream.StreamBufferCountManual.GetMin());
  }
  catch (Spinnaker::Exception &e) {
    fprintf(stderr, "Spinnaker: Could not write parameters: %s\n", e.GetFullErrorMessage());
  }
}

bool CaptureSpinnaker::stopCapture() {
  if (isCapturing()) {
    is_capturing = false;
    try {
      if (pCam->IsStreaming()) {
        pCam->EndAcquisition();
      }
      pCam->DeInit();
      pCam = (int) NULL;
      pSystem->ReleaseInstance();
    }
    catch (Spinnaker::Exception &e) {
      fprintf(stderr, "Spinnaker: Could not close device: %s\n", e.GetFullErrorMessage());
    }
  }

  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i: tmp) {
    i->removeFlags(VARTYPE_FLAG_READONLY);
  }

  return true;
}

bool CaptureSpinnaker::startCapture() {
  mutex.lock();

  pSystem = Spinnaker::System::GetInstance();

  Spinnaker::CameraList camList = pSystem->GetCameras();
  fprintf(stderr, "Spinnaker: Number of cams: %u\n", camList.GetSize());

  if (v_cam_bus->getInt() >= (int) camList.GetSize()) {
    fprintf(stderr, "Spinnaker: Invalid cam_id: %u\n", v_cam_bus->getInt());
    pSystem->ReleaseInstance();

    mutex.unlock();
    return false;
  }

  try {
    pCam = camList.GetByIndex(v_cam_bus->getInt());
  }
  catch (Spinnaker::Exception &e) {
    fprintf(stderr, "Spinnaker: Could not open device: %s\n", e.GetFullErrorMessage());
    camList.Clear();
    pSystem->ReleaseInstance();
    mutex.unlock();
    return false;
  }

  try {
    pCam->Init();
    camList.Clear();
  }
  catch (Spinnaker::Exception &e) {
    fprintf(stderr, "Spinnaker: Could not initialize device: %s\n", e.GetFullErrorMessage());
    pCam = nullptr;
    camList.Clear();
    pSystem->ReleaseInstance();
    mutex.unlock();
    return false;
  }

  // is_capturing must be set before reloadParameters()!
  is_capturing = true;

  try {
    fprintf(stderr, "Spinnaker: Opened %s - %s\n", pCam->DeviceModelName.GetValue().c_str(), pCam->DeviceSerialNumber.GetValue().c_str());

    init_camera();

    reloadParameters();
  }
  catch (Spinnaker::Exception &e) {
    fprintf(stderr, "Spinnaker: Could not initially configure device: %s\n", e.GetFullErrorMessage());
    pCam->DeInit();
    pCam = (int) NULL;
    pSystem->ReleaseInstance();
    mutex.unlock();
    return false;
  }

  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i: tmp) {
    i->addFlags(VARTYPE_FLAG_READONLY);
  }

  mutex.unlock();

  return true;
}

bool CaptureSpinnaker::copyAndConvertFrame(const RawImage &src, RawImage &target) {
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

RawImage CaptureSpinnaker::getFrame() {
  mutex.lock();

  ColorFormat out_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
  RawImage result;
  result.setColorFormat(out_color);

  try {
    if (pCam->IsStreaming() != v_acquisition->getBool()) {
      if (v_acquisition->getBool()) {
        pCam->BeginAcquisition();
      } else {
        pCam->EndAcquisition();
        readParameterValues();
      }
    }
  }
  catch (Spinnaker::Exception &e) {
    fprintf(stderr, "Spinnaker: Could not start/stop acquisition: %s\n", e.GetFullErrorMessage());
  }

  if (v_acquisition->getBool()) {
    try {
      pImage = pCam->GetNextImage();
    }
    catch (Spinnaker::Exception &e) {
      fprintf(stderr, "Spinnaker: Could not get next image: %s\n", e.GetFullErrorMessage());
    }

    if (pImage->IsIncomplete()) {
      auto description = Spinnaker::Image::GetImageStatusDescription(pImage->GetImageStatus());
      fprintf(stderr, "Spinnaker: Image incomplete: %s\n", description);
    } else {
      result.setWidth((int) pImage->GetWidth());
      result.setHeight((int) pImage->GetHeight());
      result.setData((unsigned char *) pImage->GetData());
      result.setTimeCam((double) pImage->GetTimeStamp() / 1e9);
    }
  }

  mutex.unlock();
  return result;
}

void CaptureSpinnaker::releaseFrame() {
  mutex.lock();

  try {
    if (pImage != nullptr) {
      pImage->Release();
      pImage = nullptr;
    }
  }
  catch (Spinnaker::Exception &e) {
    fprintf(stderr, "Spinnaker: Could not release image: %s\n", e.GetFullErrorMessage());
  }

  mutex.unlock();
}

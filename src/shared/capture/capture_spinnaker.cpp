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
#include <iostream>
#include <opencv2/opencv.hpp>

CaptureSpinnaker::CaptureSpinnaker(VarList * _settings,int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
{
  cam_id = (unsigned int) default_camera_id;
  is_capturing = false;

    mutex.lock();

  settings->addChild(capture_settings = new VarList("Capture Settings"));
  settings->addChild(dcam_parameters  = new VarList("Camera Parameters"));

  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild(v_cam_bus = new VarInt("cam idx",default_camera_id));

  v_convert_to_mode = new VarStringEnum("convert to mode", Colors::colorFormatToString(COLOR_RGB8));
  v_convert_to_mode->addItem(Colors::colorFormatToString(COLOR_RAW8));
  v_convert_to_mode->addItem(Colors::colorFormatToString(COLOR_RGB8));
  capture_settings->addChild(v_convert_to_mode);

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

  v_white_balance_auto = new VarStringEnum("Auto While Balance", toString(Spinnaker::BalanceWhiteAuto_Off));
  v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Off));
  v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Once));
  v_white_balance_auto->addItem(toString(Spinnaker::BalanceWhiteAuto_Continuous));

  v_gamma = new VarDouble("Gamma", 0.4, 0.25, 4.0);

  v_stream_buffer_handling_mode = new VarStringEnum("Stream Buffer Handling Mode", toString(Spinnaker::StreamBufferHandlingMode_NewestOnly));
  v_stream_buffer_handling_mode->addItem(toString(Spinnaker::StreamBufferHandlingMode_OldestFirst));
  v_stream_buffer_handling_mode->addItem(toString(Spinnaker::StreamBufferHandlingMode_OldestFirstOverwrite));
  v_stream_buffer_handling_mode->addItem(toString(Spinnaker::StreamBufferHandlingMode_NewestFirst));
  v_stream_buffer_handling_mode->addItem(toString(Spinnaker::StreamBufferHandlingMode_NewestOnly));

  v_stream_buffer_count_mode = new VarStringEnum("Stream Buffer Count Mode", toString(Spinnaker::StreamBufferCountMode_Manual));
  v_stream_buffer_count_mode->addItem(toString(Spinnaker::StreamBufferCountMode_Manual));
  v_stream_buffer_count_mode->addItem(toString(Spinnaker::StreamBufferCountMode_Auto));

  v_stream_buffer_count = new VarInt("Stream Buffer Count", 3);

  v_frame_rate = new VarDouble("Frame Rate", 0.0);
  v_frame_rate->addFlags(VARTYPE_FLAG_READONLY);
  v_frame_rate_result = new VarDouble("Resulting Frame Rate", 0.0);
  v_frame_rate_result->addFlags(VARTYPE_FLAG_READONLY);

  dcam_parameters->addChild(v_capture_mode);
  dcam_parameters->addChild(v_expose_auto);
  dcam_parameters->addChild(v_expose_us);
  dcam_parameters->addChild(v_gain_auto);
  dcam_parameters->addChild(v_gain_db);
  dcam_parameters->addChild(v_gamma);
  dcam_parameters->addChild(v_white_balance_auto);
  dcam_parameters->addChild(v_stream_buffer_handling_mode);
  dcam_parameters->addChild(v_stream_buffer_count_mode);
  dcam_parameters->addChild(v_stream_buffer_count);
  dcam_parameters->addChild(v_frame_rate);
  dcam_parameters->addChild(v_frame_rate_result);

    mvc_connect(dcam_parameters);
    mutex.unlock();
}


CaptureSpinnaker::~CaptureSpinnaker()
{
  capture_settings->deleteAllChildren();
  dcam_parameters->deleteAllChildren();
}

void CaptureSpinnaker::mvc_connect(VarList * group)
{
  for (auto &i : group->getChildren()) {
    connect(i,SIGNAL(wasEdited(VarType *)),group,SLOT(mvcEditCompleted()));
  }
  connect(group,SIGNAL(wasEdited(VarType *)),this,SLOT(changed(VarType *)));
}

void CaptureSpinnaker::changed(VarType * group)
{
  if (group->getType()==VARTYPE_ID_LIST)
  {
    writeParameterValues( (VarList *)group );
    readParameterValues( (VarList *)group );
  }
}

void CaptureSpinnaker::readAllParameterValues()
{
  readParameterValues(dcam_parameters);
}

void CaptureSpinnaker::writeAllParameterValues()
{
  writeParameterValues(dcam_parameters);
}

void CaptureSpinnaker::readParameterValues(VarList * item)
{
  if(item != dcam_parameters || pCam == nullptr)
    return;

  mutex.lock();

  try {
    v_expose_us->setDouble(pCam->ExposureTime.GetValue());
    v_expose_auto->setString(toString(pCam->ExposureAuto.GetValue()));

    v_gain_db->setDouble(pCam->Gain.GetValue());
    v_gain_auto->setString(toString(pCam->GainAuto.GetValue()));

    v_white_balance_auto->setString(toString(pCam->BalanceWhiteAuto.GetValue()));

    v_gamma->setDouble(pCam->Gamma.GetValue());

    v_stream_buffer_handling_mode->setString(toString(pCam->TLStream.StreamBufferHandlingMode.GetValue()));
    Spinnaker::StreamBufferCountModeEnum countMode = pCam->TLStream.StreamBufferCountMode.GetValue();
    v_stream_buffer_count_mode->setString(toString(countMode));
    v_stream_buffer_count->setInt(static_cast<int>(pCam->TLStream.StreamBufferCountResult.GetValue()));
    if(countMode == Spinnaker::StreamBufferCountMode_Auto) {
      v_stream_buffer_count->addFlags(VARTYPE_FLAG_READONLY);
    } else {
      v_stream_buffer_count->removeFlags(VARTYPE_FLAG_READONLY);
    }

    v_frame_rate->setDouble(pCam->AcquisitionFrameRate.GetValue());
    v_frame_rate_result->setDouble(pCam->AcquisitionResultingFrameRate.GetValue());
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while reading parameters of device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetFullErrorMessage());
  }

  mutex.unlock();
}

void CaptureSpinnaker::writeParameterValues(VarList * item)
{
  if(item != dcam_parameters || pCam == nullptr)
    return;

  mutex.lock();

  try {
    Spinnaker::ExposureAutoEnums exposureAuto = stringToExposureAuto(v_expose_auto->getString().c_str());
    pCam->ExposureAuto.SetValue(exposureAuto);
    if(exposureAuto == Spinnaker::ExposureAuto_Off) {
      pCam->ExposureTime.SetValue(v_expose_us->getDouble());
      v_expose_us->removeFlags(VARTYPE_FLAG_READONLY);
    } else {
      v_expose_us->addFlags(VARTYPE_FLAG_READONLY);
    }

    Spinnaker::GainAutoEnums gainAuto = stringToGainAuto(v_gain_auto->getString().c_str());
    pCam->GainAuto.SetValue(gainAuto);
    if(gainAuto == Spinnaker::GainAuto_Off) {
      pCam->Gain.SetValue(v_gain_db->getDouble());
      v_gain_db->removeFlags(VARTYPE_FLAG_READONLY);
    } else {
      v_gain_db->addFlags(VARTYPE_FLAG_READONLY);
    }

    pCam->BalanceWhiteAuto.SetValue(stringToBalanceWhiteAuto(v_white_balance_auto->getString().c_str()));

    pCam->Gamma.SetValue(v_gamma->getDouble());

    // reference: https://www.ptgrey.com/tan/11174
    pCam->TLStream.StreamBufferHandlingMode.SetValue(stringToStreamBufferHandlingMode(v_stream_buffer_handling_mode->getString().c_str()));
    auto countMode = stringToStreamBufferCountMode(v_stream_buffer_count_mode->getString().c_str());
    pCam->TLStream.StreamBufferCountMode.SetValue(countMode);
    if(countMode == Spinnaker::StreamBufferCountMode_Manual) {
      pCam->TLStream.StreamBufferCountManual.SetValue(v_stream_buffer_count->getInt());
    }
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while writing parameters to device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetFullErrorMessage());
  }

  mutex.unlock();
}

bool CaptureSpinnaker::resetBus()
{
    mutex.lock();

    mutex.unlock();
    
  return true;
}

bool CaptureSpinnaker::stopCapture()
{
  if (isCapturing())
  {
    is_capturing = false;
    try {
      pCam->EndAcquisition();
      pCam->DeInit();
      pCam = (int) NULL;
      pSystem->ReleaseInstance();
    }
    catch (Spinnaker::Exception &e)
    {
      fprintf(stderr, "Spinnaker: An error occurred while closing the device (error code: %d, '%s')\n", e.GetError(), e.GetFullErrorMessage());
    }
  }
  
  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i : tmp) {
    i->removeFlags( VARTYPE_FLAG_READONLY );
  }
  
  return true;
}

bool CaptureSpinnaker::startCapture()
{
  using namespace std;
  using namespace Spinnaker;
  using namespace Spinnaker::GenApi;

    mutex.lock();

  pSystem = System::GetInstance();

  cam_id = (unsigned int) v_cam_bus->getInt();

  CameraList camList = pSystem->GetCameras();
  fprintf(stderr, "Spinnaker: Number of cams: %u\n", camList.GetSize());
  
  if(cam_id >= camList.GetSize())
  {
    fprintf(stderr, "Spinnaker: Invalid cam_id: %u\n", cam_id);
    pSystem->ReleaseInstance();

      mutex.unlock();
    return false;
  }

  try
  {
    pCam = camList.GetByIndex(cam_id);
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while opening device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetFullErrorMessage());
    camList.Clear();
    pSystem->ReleaseInstance();
    mutex.unlock();
    return false;
  }

  try
  {
    pCam->Init();
    camList.Clear();
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while initializing camera %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetFullErrorMessage());
    pCam = (int) NULL;
    camList.Clear();
    pSystem->ReleaseInstance();
    mutex.unlock();
    return false;
  }

  try {
    fprintf(stderr, "Opened %s - %s\n", pCam->DeviceModelName.GetValue().c_str(), pCam->DeviceSerialNumber.GetValue().c_str());

    pCam->TriggerMode.SetValue(Spinnaker::TriggerMode_Off);
    pCam->AcquisitionMode.SetValue(Spinnaker::AcquisitionMode_Continuous);

    pCam->TLStream.StreamBufferCountMode.SetValue(Spinnaker::StreamBufferCountMode_Manual);
    pCam->TLStream.StreamBufferCountManual.SetValue(3);

    ColorFormat out_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
    if(out_color == COLOR_RAW8)
    {
      try {
        pCam->PixelFormat.SetValue(Spinnaker::PixelFormat_BayerRG8);
      }
      catch (Spinnaker::Exception &e)
      {
        fprintf(stderr, "An error occurred while configuring device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetFullErrorMessage());
      }
    } else {
      fprintf(stderr, "Spinnaker: Color format not supported: %s\n", Colors::colorFormatToString(COLOR_RAW8).c_str());
    }

    pCam->BeginAcquisition();
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while configuring device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetFullErrorMessage());
    pCam->DeInit();
    pCam = (int) NULL;
    pSystem->ReleaseInstance();
    mutex.unlock();
    return false;
  }

  is_capturing = true;

  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i : tmp) {
    i->addFlags( VARTYPE_FLAG_READONLY );
  }
    
    mutex.unlock();

  writeAllParameterValues();
  readAllParameterValues();

  return true;
}

bool CaptureSpinnaker::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
  mutex.lock();
  target.setTime(src.getTime());

  ColorFormat src_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
  ColorFormat out_color = Colors::stringToColorFormat(v_convert_to_mode->getSelection().c_str());
  if(src_color == out_color) {
    target = src;
  } else if(src_color == COLOR_RAW8 && out_color == COLOR_RGB8) {
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

RawImage CaptureSpinnaker::getFrame()
{
  mutex.lock();

  ColorFormat out_color = Colors::stringToColorFormat(v_capture_mode->getSelection().c_str());
  RawImage result;
  result.setColorFormat(out_color);
  result.setWidth(0);
  result.setHeight(0);
  result.setTime(0.0);
  result.setData(nullptr);

  try {
    pImage = pCam->GetNextImage();
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "Spinnaker: An error occurred while getting the next image (error code: %d, '%s')\n", e.GetError(), e.GetFullErrorMessage());
  }


  if (pImage->IsIncomplete())
  {
    fprintf(stderr, "Spinnaker: Image incomplete. Image Status: %d\n", pImage->GetImageStatus());
    mutex.unlock();
    return result;
  }
  
  timeval tv{};
  gettimeofday(&tv, nullptr);
  result.setTime((double)tv.tv_sec + tv.tv_usec*(1.0E-6));
  result.setWidth((int) pImage->GetWidth());
  result.setHeight((int) pImage->GetHeight());
  result.setData((unsigned char*) pImage->GetData());

    mutex.unlock();
  return result;
}

void CaptureSpinnaker::releaseFrame()
{
    mutex.lock();

  try {
    pImage->Release();
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "Spinnaker: An error occurred while releasing an image (error code: %d, '%s')\n", e.GetError(), e.GetFullErrorMessage());
  }

    mutex.unlock();
}

string CaptureSpinnaker::getCaptureMethodName() const
{
  return "Spinnaker";
}

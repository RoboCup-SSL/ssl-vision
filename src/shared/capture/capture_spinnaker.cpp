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

#ifndef VDATA_NO_QT
CaptureSpinnaker::CaptureSpinnaker(VarList * _settings,int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
#else
CaptureSpinnaker::CaptureSpinnaker(VarList * _settings,int default_camera_id) : CaptureInterface(_settings)
#endif
{
  cam_id = (unsigned int) default_camera_id;
  is_capturing = false;

  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  settings->addChild(capture_settings = new VarList("Capture Settings"));
  settings->addChild(dcam_parameters  = new VarList("Camera Parameters"));

  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild(v_cam_bus          = new VarInt("cam idx",default_camera_id));
  capture_settings->addChild(v_colorout         = new VarStringEnum("color mode", Colors::colorFormatToString(COLOR_RAW8)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RAW8));

  v_expose_auto = new VarStringEnum("Auto Expose", exposureAutoToString(Spinnaker::ExposureAuto_Off));
  v_expose_auto->addItem(exposureAutoToString(Spinnaker::ExposureAuto_Off));
  v_expose_auto->addItem(exposureAutoToString(Spinnaker::ExposureAuto_Once));
  v_expose_auto->addItem(exposureAutoToString(Spinnaker::ExposureAuto_Continuous));
  v_expose_us = new VarDouble("Expose [us]", 2000, 10, 100000);

  v_gain_auto = new VarStringEnum("Auto Gain", gainAutoToString(Spinnaker::GainAuto_Off));
  v_gain_auto->addItem(gainAutoToString(Spinnaker::GainAuto_Off));
  v_gain_auto->addItem(gainAutoToString(Spinnaker::GainAuto_Once));
  v_gain_auto->addItem(gainAutoToString(Spinnaker::GainAuto_Continuous));
  v_gain_db = new VarDouble("Gain [dB]", 0.0, 0.0, 12.0);

  v_white_balance_auto = new VarStringEnum("Auto While Balance", balanceWhiteAutoToString(Spinnaker::BalanceWhiteAuto_Continuous));
  v_white_balance_auto->addItem(balanceWhiteAutoToString(Spinnaker::BalanceWhiteAuto_Off));
  v_white_balance_auto->addItem(balanceWhiteAutoToString(Spinnaker::BalanceWhiteAuto_Once));
  v_white_balance_auto->addItem(balanceWhiteAutoToString(Spinnaker::BalanceWhiteAuto_Continuous));

  v_frame_rate = new VarDouble("Frame Rate", 0.0);
  v_frame_rate->addFlags(VARTYPE_FLAG_READONLY);

  dcam_parameters->addChild(v_expose_auto);
  dcam_parameters->addChild(v_expose_us);
  dcam_parameters->addChild(v_gain_auto);
  dcam_parameters->addChild(v_gain_db);
  dcam_parameters->addChild(v_white_balance_auto);
  dcam_parameters->addChild(v_frame_rate);

  #ifndef VDATA_NO_QT
    mvc_connect(dcam_parameters);
    mutex.unlock();
  #endif
}


CaptureSpinnaker::~CaptureSpinnaker()
{
  capture_settings->deleteAllChildren();
  dcam_parameters->deleteAllChildren();
}

#ifndef VDATA_NO_QT
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
#endif

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

#ifndef VDATA_NO_QT
  mutex.lock();
#endif

  try {
    v_expose_us->setDouble(pCam->ExposureTime.GetValue());
    v_expose_auto->setString(exposureAutoToString(pCam->ExposureAuto.GetValue()));

    v_gain_db->setDouble(pCam->Gain.GetValue());
    v_gain_auto->setString(gainAutoToString(pCam->GainAuto.GetValue()));

    v_white_balance_auto->setString(balanceWhiteAutoToString(pCam->BalanceWhiteAuto.GetValue()));
    v_frame_rate->setDouble(pCam->AcquisitionFrameRate.GetValue());
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while reading parameters of device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetErrorMessage());
  }

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

void CaptureSpinnaker::writeParameterValues(VarList * item)
{
  if(item != dcam_parameters || pCam == nullptr)
    return;

#ifndef VDATA_NO_QT
  mutex.lock();
#endif

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
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while writing parameters to device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetErrorMessage());
  }

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

bool CaptureSpinnaker::resetBus()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    
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
      fprintf(stderr, "Spinnaker: An error occurred while closing the device (error code: %d, '%s')\n", e.GetError(), e.GetErrorMessage());
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

  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  pSystem = System::GetInstance();

  cam_id = (unsigned int) v_cam_bus->getInt();

  CameraList camList = pSystem->GetCameras();
  fprintf(stderr, "Spinnaker: Number of cams: %u\n", camList.GetSize());
  
  if(cam_id >= camList.GetSize())
  {
    fprintf(stderr, "Spinnaker: Invalid cam_id: %u\n", cam_id);
    pSystem->ReleaseInstance();

    #ifndef VDATA_NO_QT
      mutex.unlock();
    #endif
    return false;
  }

  try
  {
    pCam = camList.GetByIndex(cam_id);
    pCam->Init();
    camList.Clear();
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while opening device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetErrorMessage());
    camList.Clear();
    pSystem->ReleaseInstance();
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  }

  try {
    fprintf(stderr, "Opened %s - %s\n", pCam->DeviceModelName.GetValue().c_str(), pCam->DeviceSerialNumber.GetValue().c_str());

    pCam->TriggerMode.SetValue(Spinnaker::TriggerModeEnums::TriggerMode_Off);
    pCam->AcquisitionMode.SetValue(Spinnaker::AcquisitionModeEnums::AcquisitionMode_Continuous);

    ColorFormat out_color = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
    if(out_color == COLOR_RAW8)
    {
      pCam->PixelFormat.SetValue(Spinnaker::PixelFormat_BayerRG8);
    } else {
      fprintf(stderr, "Spinnaker: Color format not supported: %s\n", Colors::colorFormatToString(COLOR_RAW8).c_str());
    }
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "An error occurred while configuring device %d with Spinnaker (error code: %d, '%s')\n", cam_id, e.GetError(), e.GetErrorMessage());
    pCam->DeInit();
    pCam = (int) NULL;
    pSystem->ReleaseInstance();
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    return false;
  }

  pCam->BeginAcquisition();
  is_capturing = true;

  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i : tmp) {
    i->addFlags( VARTYPE_FLAG_READONLY );
  }
    
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif

  writeAllParameterValues();
  readAllParameterValues();

  return true;
}

bool CaptureSpinnaker::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  target = src;

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif

  return true;
}

RawImage CaptureSpinnaker::getFrame()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  ColorFormat out_color = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
  RawImage result;
  result.setColorFormat(out_color);
  result.setWidth(0);
  result.setHeight(0);
  result.setTime(0.0);
  result.setData(nullptr);

  pImage = pCam->GetNextImage();

  if (pImage->IsIncomplete())
  {
    fprintf(stderr, "Spinnaker: Image incomplete. Image Status: %d\n", pImage->GetImageStatus());
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return result;
  }
  
  timeval tv{};
  gettimeofday(&tv, nullptr);
  result.setTime((double)tv.tv_sec + tv.tv_usec*(1.0E-6));
  result.setWidth((int) pImage->GetWidth());
  result.setHeight((int) pImage->GetHeight());
  result.setData((unsigned char*) pImage->GetData());

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
  return result;
}

void CaptureSpinnaker::releaseFrame()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  try {
    pImage->Release();
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "Spinnaker: An error occurred while releasing an image (error code: %d, '%s')\n", e.GetError(), e.GetErrorMessage());
  }

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
}

string CaptureSpinnaker::getCaptureMethodName() const
{
  return "Spinnaker";
}

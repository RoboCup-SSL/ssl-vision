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

  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild(v_cam_bus          = new VarInt("cam idx",default_camera_id));
  capture_settings->addChild(v_colorout         = new VarStringEnum("color mode", Colors::colorFormatToString(COLOR_RAW8)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RAW8));

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
}

CaptureSpinnaker::~CaptureSpinnaker()
{
  capture_settings->deleteAllChildren();
}

void CaptureSpinnaker::readAllParameterValues()
{
}


#ifndef VDATA_NO_QT
void CaptureSpinnaker::changed(VarType * /*group*/) {
}
#endif

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
    
  //grab current parameters:
  cam_id = (unsigned int) v_cam_bus->getInt();

  CameraList camList = pSystem->GetCameras();
  fprintf(stderr, "Spinnaker: Number of cams: %u\n", camList.GetSize());
  
  if(cam_id >= camList.GetSize())
  {
    fprintf(stderr, "Spinnaker: Invalid cam_id: %u\n", cam_id);

    #ifndef VDATA_NO_QT
      mutex.unlock();
    #endif
    return false;
  }
  
  pCam = camList.GetByIndex(cam_id);
  try
  {
    pCam->Init();
  }
  catch (Spinnaker::Exception &e)
  {
    fprintf(stderr, "Spinnaker: An error occurred while opening the device(error code: %d, '%s')\n", e.GetError(), e.GetErrorMessage());
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  }

  camList.Clear();
  
  fprintf(stderr, "Spinnaker: Opened: %s\n", pCam->DeviceSerialNumber.GetDisplayName().c_str());

  INodeMap & nodeMap = pCam->GetNodeMap();

  CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
  CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
  int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
  ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

  ColorFormat out_color = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
  if(out_color == COLOR_RAW8)
  {
    CEnumerationPtr ptrPixelFormat = nodeMap.GetNode("PixelFormat");
    CEnumEntryPtr ptrPixelFormatBayerRg8 = ptrPixelFormat->GetEntryByName("BayerRG8");
    int64_t pixelFormatBayerRg8 = ptrPixelFormatBayerRg8->GetValue();
    ptrPixelFormat->SetIntValue(pixelFormatBayerRg8);
  } else {
    fprintf(stderr, "Spinnaker: Color format not supported: %s\n", Colors::colorFormatToString(COLOR_RAW8).c_str());
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

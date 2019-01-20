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
  \file    capture_bluefox3.cpp
  \brief   C++ Implementation: CaptureBlueFox3
  \author  Nicolai Ommer, (C) 2019
*/
//========================================================================

#include "capture_bluefox3.h"
#include <memory>
#include <iostream>

#ifndef VDATA_NO_QT
CaptureBlueFox3::CaptureBlueFox3(VarList * _settings,int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
#else
CaptureBlueFox3::CaptureBlueFox3(VarList * _settings,int default_camera_id) : CaptureInterface(_settings)
#endif
{
  cam_id = (unsigned int) default_camera_id;
  is_capturing = false;
  pDevMgr = nullptr;
  
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

CaptureBlueFox3::~CaptureBlueFox3()
{
  capture_settings->deleteAllChildren();

  delete pDevMgr;
}

void CaptureBlueFox3::readAllParameterValues()
{
}


#ifndef VDATA_NO_QT
void CaptureBlueFox3::changed(VarType * /*group*/) {
}
#endif

bool CaptureBlueFox3::resetBus()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    
  return true;
}

bool CaptureBlueFox3::stopCapture()
{
  if (isCapturing())
  {
    delete pFI;

    pDevice->close();
    
    is_capturing = false;
  }
  
  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto &i : tmp) {
    i->removeFlags( VARTYPE_FLAG_READONLY );
  }
  
  return true;
}

bool CaptureBlueFox3::startCapture()
{
    using namespace std;
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  if(pDevMgr == nullptr) {
    pDevMgr = new DeviceManager();
  }
    
  //grab current parameters:
  cam_id = (unsigned int) v_cam_bus->getInt();
  
  const unsigned int devCnt = pDevMgr->deviceCount();
  fprintf(stderr, "BlueFox3: Number of cams: %u\n", devCnt);
  
  if(cam_id >= devCnt)
  {
    fprintf(stderr, "BlueFox3: Invalid cam_id: %u\n", cam_id);

    #ifndef VDATA_NO_QT
      mutex.unlock();
    #endif
    return false;
  }
  
  pDevice = (*pDevMgr)[cam_id];
  
  try
  {
    pDevice->open();
  }
  catch(mvIMPACT::acquire::ImpactAcquireException& e)
  {
    // this e.g. might happen if the same device is already opened in another process...
    fprintf(stderr, "BlueFox3: An error occurred while opening the device(error code: %d, '%s')\n", e.getErrorCode(), e.getErrorString().c_str());
    #ifndef VDATA_NO_QT
      mutex.unlock();
    #endif
    return false;
  }
  
  fprintf(stderr, "BlueFox3: Opened: %s with serial ID %s\n", pDevice->family.read().c_str(), pDevice->serial.read().c_str());

  GenICam::ImageFormatControl ifc(pDevice);
  ifc.pixelFormat.writeS("BayerRG8");
  ifc.mvSensorDigitizationBitDepth.writeS("Bpp10");

  ImageProcessing proc(pDevice);
  proc.restoreDefault();

  pFI = new FunctionInterface(pDevice);

  ImageDestination id( pDevice );
  id.restoreDefault();

  ColorFormat out_color = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
  if(out_color == COLOR_RGB8)
  {
    id.pixelFormat.write(idpfBGR888Packed);
  } else if(out_color == COLOR_YUV422_UYVY)
  {
    id.pixelFormat.write(idpfYUV422Packed);
  } else if(out_color == COLOR_RAW8)
  {
    id.pixelFormat.write(idpfRaw);
  }

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

bool CaptureBlueFox3::copyAndConvertFrame(const RawImage & src, RawImage & target)
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

RawImage CaptureBlueFox3::getFrame()
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
  
  // make sure the request queue is always filled
  while((static_cast<TDMR_ERROR>( pFI->imageRequestSingle() ) ) == DMR_NO_ERROR ) {};
  
  int requestNr = pFI->imageRequestWaitFor(-1);

  // check if the image has been captured without any problems.
  if(!pFI->isRequestNrValid(requestNr))
  {
      // If the error code is -2119(DEV_WAIT_FOR_REQUEST_FAILED), the documentation will provide
      // additional information under TDMR_ERROR in the interface reference
      fprintf(stderr, "imageRequestWaitFor failed (%d, %s)\n", requestNr, ImpactAcquireException::getErrorCodeAsString( requestNr ).c_str());
      #ifndef VDATA_NO_QT
	mutex.unlock();
      #endif
      return result;
  }

  const Request* pRequest = pFI->getRequest(requestNr);

  if(pRequest->isOK())
  {
    timeval tv{};
    gettimeofday(&tv, nullptr);
    result.setTime((double)tv.tv_sec + tv.tv_usec*(1.0E-6));
    result.setWidth(pRequest->imageWidth.read());
    result.setHeight(pRequest->imageHeight.read());
    result.setData((unsigned char*)pRequest->imageData.read());
  }
  else
  {
    fprintf(stderr, "BlueFox3: request not OK\n");
  }
  
  lastRequestNr = requestNr;
  
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
  return result;
}

void CaptureBlueFox3::releaseFrame() 
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif

  if(pFI->isRequestNrValid(lastRequestNr))
    pFI->imageRequestUnlock(lastRequestNr);
  
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

string CaptureBlueFox3::getCaptureMethodName() const 
{
  return "BlueFox3";
}

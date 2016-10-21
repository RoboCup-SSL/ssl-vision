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
  \file    capture_bluefox2.cpp
  \brief   C++ Implementation: CaptureBlueFox2
  \author  Andre Ryll, (C) 2016
*/
//========================================================================

#include "capture_bluefox2.h"

#ifndef VDATA_NO_QT
CaptureBlueFox2::CaptureBlueFox2(VarList * _settings,int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
#else
CaptureBlueFox2::CaptureBlueFox2(VarList * _settings,int default_camera_id) : CaptureInterface(_settings)
#endif
{
  cam_id = default_camera_id;
  is_capturing = false;
  
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  settings->addChild(capture_settings = new VarList("Capture Settings"));
  settings->addChild(dcam_parameters  = new VarList("Camera Parameters"));

  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild(v_cam_bus          = new VarInt("cam idx",default_camera_id));
  capture_settings->addChild(v_colorout         = new VarStringEnum("color mode", Colors::colorFormatToString(COLOR_YUV422_UYVY)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));

  //=======================DCAM PARAMETERS===========================
  dcam_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );

  v_expose_us = new VarInt("Expose [us]", 2000, 10, 100000);
  v_expose_overlapped = new VarBool("Expose Overlapped", true);
  v_gain_db = new VarDouble("Gain [dB]", 0.0, 0.0, 12.0);
  
  v_hdr_mode = new VarStringEnum("HDR Mode", hdrModeToString(HDR_MODE_OFF));
  v_hdr_mode->addItem(hdrModeToString(HDR_MODE_OFF));
  v_hdr_mode->addItem(hdrModeToString(HDR_MODE_FIXED0));
  v_hdr_mode->addItem(hdrModeToString(HDR_MODE_FIXED1));
  v_hdr_mode->addItem(hdrModeToString(HDR_MODE_FIXED2));
  v_hdr_mode->addItem(hdrModeToString(HDR_MODE_FIXED3));
  v_hdr_mode->addItem(hdrModeToString(HDR_MODE_FIXED4));
  v_hdr_mode->addItem(hdrModeToString(HDR_MODE_FIXED5));
  
  v_mirror_top_down = new VarBool("Mirror Top/Down");
  v_mirror_left_right = new VarBool("Mirror Left/Right");
  v_wb_red = new VarDouble("WB Red", 1.0, 0.1, 10.0);
  v_wb_green = new VarDouble("WB Green", 1.0, 0.1, 10.0);
  v_wb_blue = new VarDouble("WB Blue", 1.0, 0.1, 10.0);
  v_sharpen = new VarBool("Sharpen", true);
  v_gamma = new VarDouble("Gamma", 1.0, 0.01, 10.0);
  
  v_color_twist_mode = new VarStringEnum("Color Twist", colorTwistModeToString(COLOR_TWIST_MODE_SRGB_D50));
  v_color_twist_mode->addItem(colorTwistModeToString(COLOR_TWIST_MODE_OFF));
  v_color_twist_mode->addItem(colorTwistModeToString(COLOR_TWIST_MODE_ADOBERGB_D50));
  v_color_twist_mode->addItem(colorTwistModeToString(COLOR_TWIST_MODE_SRGB_D50));
  v_color_twist_mode->addItem(colorTwistModeToString(COLOR_TWIST_MODE_WIDE_GAMUT_RGB_D50));
  v_color_twist_mode->addItem(colorTwistModeToString(COLOR_TWIST_MODE_ADOBERGB_D65));
  v_color_twist_mode->addItem(colorTwistModeToString(COLOR_TWIST_MODE_SRGB_D65));
  
  v_aoi_width = new VarInt("AOI width", 752, 0, 752);
  v_aoi_height = new VarInt("AOI height", 480, 0, 480);
  v_aoi_left = new VarInt("AOI left", 0, 0, 752);
  v_aoi_top = new VarInt("AOI top", 0, 0, 480);
  
  v_pixel_clock = new VarStringEnum("Pixel Clock", pixelClockToString(cpc40000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc6000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc8000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc10000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc12000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc20000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc24000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc27000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc32000KHz));
  v_pixel_clock->addItem(pixelClockToString(cpc40000KHz));
  
  dcam_parameters->addChild(v_aoi_width);
  dcam_parameters->addChild(v_aoi_height);
  dcam_parameters->addChild(v_aoi_left);
  dcam_parameters->addChild(v_aoi_top);
  dcam_parameters->addChild(v_pixel_clock);
  dcam_parameters->addChild(v_expose_us);
  dcam_parameters->addChild(v_expose_overlapped);
  dcam_parameters->addChild(v_gain_db);
  dcam_parameters->addChild(v_hdr_mode);
  dcam_parameters->addChild(v_mirror_top_down);
  dcam_parameters->addChild(v_mirror_left_right);
  dcam_parameters->addChild(v_wb_red);
  dcam_parameters->addChild(v_wb_green);
  dcam_parameters->addChild(v_wb_blue);
  dcam_parameters->addChild(v_sharpen);
  dcam_parameters->addChild(v_gamma);
  dcam_parameters->addChild(v_color_twist_mode);
  
  #ifndef VDATA_NO_QT
    mvc_connect(dcam_parameters);
    mutex.unlock();
  #endif
}

#ifndef VDATA_NO_QT
void CaptureBlueFox2::mvc_connect(VarList * group)
{
  vector<VarType *> v=group->getChildren();
  for (unsigned int i=0;i<v.size();i++)
  {
    connect(v[i],SIGNAL(wasEdited(VarType *)),group,SLOT(mvcEditCompleted()));
  }
  connect(group,SIGNAL(wasEdited(VarType *)),this,SLOT(changed(VarType *)));
}

void CaptureBlueFox2::changed(VarType * group)
{
  if (group->getType()==VARTYPE_ID_LIST)
  {
    writeParameterValues( (VarList *)group );
    readParameterValues( (VarList *)group );
  }
}
#endif

void CaptureBlueFox2::readAllParameterValues()
{
  readParameterValues(dcam_parameters);
}

void CaptureBlueFox2::writeAllParameterValues()
{
  writeParameterValues(dcam_parameters);
}

void CaptureBlueFox2::readParameterValues(VarList * item)
{
  if(item != dcam_parameters)
    return;
  
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    
    // TODO: could do a read-out, but why?
//   v_expose_us->setInt(pSettings->expose_us.read());
    
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
}

void CaptureBlueFox2::writeParameterValues(VarList * item)
{
  if(item != dcam_parameters)
    return;

  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    
  pSettings->pixelClock_KHz.write(stringToPixelClock(v_pixel_clock->getString().c_str()));

  pSettings->expose_us.write(v_expose_us->getInt());
  
  if(v_expose_overlapped->getBool())
    pSettings->exposeMode.write(cemOverlapped);
  else
    pSettings->exposeMode.write(cemStandard);
  
  pSettings->gain_dB.write(v_gain_db->getDouble());
  
  pSettings->aoiStartX.write(v_aoi_left->getInt());
  pSettings->aoiStartY.write(v_aoi_top->getInt());
  pSettings->aoiWidth.write(v_aoi_width->getInt());
  pSettings->aoiHeight.write(v_aoi_height->getInt());
  
  HDRMode hdrMode = stringToHdrMode(v_hdr_mode->getString().c_str());
  
  if(hdrMode == HDR_MODE_OFF)
  {
    pSettings->getHDRControl().HDREnable.write(bFalse);
  }
  else
  {
    pSettings->getHDRControl().HDREnable.write(bTrue);
    
    TCameraHDRMode hdr;
    
    switch(hdrMode)
    {
      case HDR_MODE_FIXED0: hdr = cHDRmFixed0; break;
      case HDR_MODE_FIXED1: hdr = cHDRmFixed1; break;
      case HDR_MODE_FIXED2: hdr = cHDRmFixed2; break;
      case HDR_MODE_FIXED3: hdr = cHDRmFixed3; break;
      case HDR_MODE_FIXED4: hdr = cHDRmFixed4; break;
      default: hdr = cHDRmFixed5; break;
    }
    
    pSettings->getHDRControl().HDRMode.write(hdr);
  }
  
  int mm = mmOff;
  if(v_mirror_top_down->getBool())
    mm |= mmTopDown;
  if(v_mirror_left_right->getBool())
    mm |= mmLeftRight;
  
  pImageProc->mirrorOperationMode.write(momGlobal);
  pImageProc->mirrorModeGlobal.write((TMirrorMode)mm);
  
  pImageProc->whiteBalance.write(wbpUser1);
  WhiteBalanceSettings& wb = pImageProc->getWBUserSetting(0);
  wb.WBAoiMode.write(amFull);
  wb.totalGain.write(1.0);
  wb.redGain.write(v_wb_red->getDouble());
  wb.greenGain.write(v_wb_green->getDouble());
  wb.blueGain.write(v_wb_blue->getDouble());
  
  if(v_sharpen->getBool())
    pImageProc->filter.write(ipfSharpen);
  else
    pImageProc->filter.write(ipfOff);
  
  pImageProc->LUTEnable.write(bTrue);
  pImageProc->LUTMode.write(LUTmGamma);
  pImageProc->LUTImplementation.write(LUTiSoftware);
  pImageProc->LUTMappingSoftware.write(LUTm10To10);
  pImageProc->getLUTParameter(0).gamma.write(v_gamma->getDouble());
  pImageProc->getLUTParameter(1).gamma.write(v_gamma->getDouble());
  pImageProc->getLUTParameter(2).gamma.write(v_gamma->getDouble());
  pImageProc->getLUTParameter(3).gamma.write(v_gamma->getDouble());
  
  ColorTwistMode ctMode = stringToColorTwistMode(v_color_twist_mode->getString().c_str());
  
  if(ctMode == COLOR_TWIST_MODE_OFF)
  {
    pImageProc->colorTwistInputCorrectionMatrixEnable.write(bFalse);
    pImageProc->colorTwistEnable.write(bFalse);
    pImageProc->colorTwistOutputCorrectionMatrixEnable.write(bFalse);
  }
  else
  {
    pImageProc->colorTwistInputCorrectionMatrixEnable.write(bTrue);
    pImageProc->colorTwistInputCorrectionMatrixMode.write(cticmmDeviceSpecific);
    pImageProc->colorTwistEnable.write(bTrue);
    pImageProc->colorTwistOutputCorrectionMatrixEnable.write(bTrue);
    
    int ct;
    switch(ctMode)
    {
      case COLOR_TWIST_MODE_ADOBERGB_D50: ct = ctocmmXYZToAdobeRGB_D50; break;
      case COLOR_TWIST_MODE_SRGB_D50: ct = ctocmmXYZTosRGB_D50; break;
      case COLOR_TWIST_MODE_WIDE_GAMUT_RGB_D50: ct = ctocmmXYZToWideGamutRGB_D50; break;
      case COLOR_TWIST_MODE_ADOBERGB_D65: ct = ctocmmXYZToAdobeRGB_D65; break;
      default: ct = ctocmmXYZTosRGB_D65; break;
    }
    
    pImageProc->colorTwistOutputCorrectionMatrixMode.write((TColorTwistOutputCorrectionMatrixMode)ct);
  }

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
}

CaptureBlueFox2::~CaptureBlueFox2()
{
  capture_settings->deleteAllChildren();
  dcam_parameters->deleteAllChildren();
}

bool CaptureBlueFox2::resetBus()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    
  return true;
}

bool CaptureBlueFox2::stopCapture()
{
  if (isCapturing())
  {
    readAllParameterValues();
    
    delete pFI;
    delete pSettings;
    delete pImageProc;
    
    pDevice->close();
    
    is_capturing = false;
  }
  
  vector<VarType *> tmp = capture_settings->getChildren();
  for (unsigned int i=0; i < tmp.size();i++)
  {
    tmp[i]->removeFlags( VARTYPE_FLAG_READONLY );
  }
  
  dcam_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );
  
  return true;
}

bool CaptureBlueFox2::startCapture()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    
  //grab current parameters:
  cam_id = v_cam_bus->getInt();
  
  const unsigned int devCnt = devMgr.deviceCount();
  fprintf(stderr, "BlueFox2: Number of cams: %u\n", devCnt);
  
  if(cam_id >= devCnt)
  {
    fprintf(stderr, "BlueFox2: Invalid cam_id: %u\n", cam_id);

    #ifndef VDATA_NO_QT
      mutex.unlock();
    #endif
    return false;
  }
  
  pDevice = devMgr[cam_id];
  
  try
  {
    pDevice->open();
  }
  catch(mvIMPACT::acquire::ImpactAcquireException& e)
  {
    // this e.g. might happen if the same device is already opened in another process...
    fprintf(stderr, "BlueFox2: An error occurred while opening the device(error code: %d, '%s')\n", e.getErrorCode(), e.getErrorString().c_str());
    #ifndef VDATA_NO_QT
      mutex.unlock();
    #endif
    return false;
  }
  
  fprintf(stderr, "BlueFox2: Opened: %s with serial ID %s\n", pDevice->family.read().c_str(), pDevice->serial.read().c_str());
  
  pFI = new FunctionInterface(pDevice);
  
  ImageDestination id( pDevice );
  id.restoreDefault();
  
  ColorFormat out_color = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
  if(out_color == COLOR_RGB8)
  {
    id.pixelFormat.write(idpfBGR888Packed);
  }
  else
  {
    id.pixelFormat.write(idpfYUV422Packed);
  }
  
  pSettings = new CameraSettingsBlueFOX(pDevice);
  pSettings->restoreDefault();
  
  pImageProc = new ImageProcessing(pDevice);
  pImageProc->restoreDefault();
  
  is_capturing = true;
  
  vector<VarType *> tmp = capture_settings->getChildren();
  for (unsigned int i=0; i < tmp.size();i++) {
    tmp[i]->addFlags( VARTYPE_FLAG_READONLY );
  }
    
  dcam_parameters->removeFlags( VARTYPE_FLAG_HIDE_CHILDREN );

  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
    
  printf("BlueFox2 Info: Restoring Previously Saved Camera Parameters\n");
  writeAllParameterValues();
  readAllParameterValues();
  
  return true;
}

bool CaptureBlueFox2::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    
  ColorFormat src_fmt = src.getColorFormat();
  
  if(target.getData() == 0)
  {
    //allocate target, if it does not exist yet
    target.allocate(src_fmt, src.getWidth(), src.getHeight());
  } 
  else
  {
    target.ensure_allocation(src_fmt, src.getWidth(), src.getHeight());
  }
  target.setTime(src.getTime());
  
  if(src.getColorFormat() == COLOR_RGB8)
  {
    memcpy(target.getData(),src.getData(),src.getNumBytes());
  }
  else
  {
    for(int i = 0; i < src.getNumBytes(); i += 2)
    {
      target.getData()[i+1] = src.getData()[i];
      target.getData()[i] = src.getData()[i+1];
    }
  }
  
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif

  return true;
}

RawImage CaptureBlueFox2::getFrame()
{
  #ifndef VDATA_NO_QT
    mutex.lock();
  #endif
    
  RawImage result;
  result.setColorFormat(capture_format);
  result.setWidth(0);
  result.setHeight(0);
  result.setTime(0.0);
  result.setData(0);
  
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
    timeval tv;
    gettimeofday(&tv,NULL);
    result.setTime((double)tv.tv_sec + tv.tv_usec*(1.0E-6));
    result.setWidth(pRequest->imageWidth.read());
    result.setHeight(pRequest->imageHeight.read());
    ColorFormat out_color = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
    result.setColorFormat(out_color);
    result.setData((unsigned char*)pRequest->imageData.read());
//     fprintf(stderr, "Timestamp_us: %ld\n", pRequest->infoTimeStamp_us.read());
//     fprintf(stderr, "BPP: %u\n", pRequest->imageBytesPerPixel.read());
//     fprintf(stderr, "PixelFormat: %s\n", pRequest->imagePixelFormat.readS().c_str());
  }
  else
  {
    fprintf(stderr, "BlueFox2: request not OK\n");
  }
  
  lastRequestNr = requestNr;
  
  #ifndef VDATA_NO_QT
    mutex.unlock();
  #endif
  return result;
}

void CaptureBlueFox2::releaseFrame() 
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

string CaptureBlueFox2::getCaptureMethodName() const 
{
  return "BlueFox2";
}

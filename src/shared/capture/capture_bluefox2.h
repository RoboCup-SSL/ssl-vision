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
  \file    capture_bluefox2.h
  \brief   C++ Interface: CaptureBlueFox2
  \author  Andre Ryll, (C) 2016
*/
//========================================================================

#ifndef CAPTURE_BLUEFOX2_H
#define CAPTURE_BLUEFOX2_H
#include "captureinterface.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "VarTypes.h"
#include <mvIMPACT_CPP/mvIMPACT_acquire.h>

//#include "conversions.h"
  #include <QMutex>

namespace mvIMPACT {
namespace acquire {
enum TCameraPixelClock;}
}

using namespace mvIMPACT::acquire;

/*!
  \class CaptureBlueFox2
  \brief A capture class for Matrix-Vision BlueFox2 cameras
  \author  Andre Ryll, (C) 2016

  This class provides the ability to use and configure Matrix-Vision
  BlueFox2 cameras using the mvIMPACT driver.

  Overall, it not only provides capture-abilities, but also full
  on-the-fly configuration abilities through the VarTypes system.
 
  If you find your camera not working correctly, or discover a bug,
  please inform the author, as we are aiming for complete camera
  coverage.
*/
  #include <QMutex>
  //if using QT, inherit QObject as a base
class CaptureBlueFox2 : public QObject, public CaptureInterface
{
  Q_OBJECT
  
  public slots:
  void changed(VarType * group);
  
  protected:
  QMutex mutex;
  
  public:

  enum HDRMode
  {
    HDR_MODE_OFF,
    HDR_MODE_FIXED0,
    HDR_MODE_FIXED1,
    HDR_MODE_FIXED2,
    HDR_MODE_FIXED3,
    HDR_MODE_FIXED4,
    HDR_MODE_FIXED5,
  };
  
  static HDRMode stringToHdrMode(const char* s)
  {
    if (strcmp(s,"Off")==0) {
      return HDR_MODE_OFF;
    } else if (strcmp(s,"Fixed0")==0) {
      return HDR_MODE_FIXED0;
    } else if (strcmp(s,"Fixed1")==0) {
      return HDR_MODE_FIXED1;
    } else if (strcmp(s,"Fixed2")==0) {
      return HDR_MODE_FIXED2;
    } else if (strcmp(s,"Fixed3")==0) {
      return HDR_MODE_FIXED3;
    } else if (strcmp(s,"Fixed4")==0) {
      return HDR_MODE_FIXED4;
    } else if (strcmp(s,"Fixed5")==0) {
      return HDR_MODE_FIXED5;
    } else {
      return HDR_MODE_OFF;
    }
  }
  
  static string hdrModeToString(HDRMode mode)
  {
    switch(mode)
    {
      case HDR_MODE_FIXED0: return "Fixed0";
      case HDR_MODE_FIXED1: return "Fixed1";
      case HDR_MODE_FIXED2: return "Fixed2";
      case HDR_MODE_FIXED3: return "Fixed3";
      case HDR_MODE_FIXED4: return "Fixed4";
      case HDR_MODE_FIXED5: return "Fixed5";
      default: return "Off";
    }
  }
  
  enum ColorTwistMode
  {
    COLOR_TWIST_MODE_OFF,
    COLOR_TWIST_MODE_ADOBERGB_D50,
    COLOR_TWIST_MODE_SRGB_D50,
    COLOR_TWIST_MODE_WIDE_GAMUT_RGB_D50,
    COLOR_TWIST_MODE_ADOBERGB_D65,
    COLOR_TWIST_MODE_SRGB_D65,
  };
  
  static ColorTwistMode stringToColorTwistMode(const char* s)
  {
      if (strcmp(s,"Off")==0) {
      return COLOR_TWIST_MODE_OFF;
    } else if (strcmp(s,"AdobeRGB_D50")==0) {
      return COLOR_TWIST_MODE_ADOBERGB_D50;
    } else if (strcmp(s,"sRGB_D50")==0) {
      return COLOR_TWIST_MODE_SRGB_D50;
    } else if (strcmp(s,"WideGamutRGB_D50")==0) {
      return COLOR_TWIST_MODE_WIDE_GAMUT_RGB_D50;
    } else if (strcmp(s,"AdobeRGB_D65")==0) {
      return COLOR_TWIST_MODE_ADOBERGB_D65;
    } else if (strcmp(s,"sRGB_D65")==0) {
      return COLOR_TWIST_MODE_SRGB_D65;
    } else {
      return COLOR_TWIST_MODE_OFF;
    }
  }
  
  static string colorTwistModeToString(ColorTwistMode mode)
  {
    switch(mode)
    {
      case COLOR_TWIST_MODE_ADOBERGB_D50: return "AdobeRGB_D50";
      case COLOR_TWIST_MODE_SRGB_D50: return "sRGB_D50";
      case COLOR_TWIST_MODE_WIDE_GAMUT_RGB_D50: return "WideGamutRGB_D50";
      case COLOR_TWIST_MODE_ADOBERGB_D65: return "AdobeRGB_D65";
      case COLOR_TWIST_MODE_SRGB_D65: return "sRGB_D65";
      default: return "Off";
    }
  }
  
  static TCameraPixelClock stringToPixelClock(const char* s)
  {
    if (strcmp(s,"8MHz")==0) {
      return cpc8000KHz;
    } else if (strcmp(s,"10MHz")==0) {
      return cpc10000KHz;
    } else if (strcmp(s,"12MHz")==0) {
      return cpc12000KHz;
    } else if (strcmp(s,"20MHz")==0) {
      return cpc20000KHz;
    } else if (strcmp(s,"24MHz")==0) {
      return cpc24000KHz;
    } else if (strcmp(s,"27MHz")==0) {
      return cpc27000KHz;
    } else if (strcmp(s,"32MHz")==0) {
      return cpc32000KHz;
    } else if (strcmp(s,"40MHz")==0) {
      return cpc40000KHz;
    } else {
      return cpc6000KHz;
    }
  }

  static string pixelClockToString(TCameraPixelClock pxc)
  {
    switch(pxc)
    {
      case cpc8000KHz: return "8MHz";
      case cpc10000KHz: return "10MHz";
      case cpc12000KHz: return "12MHz";
      case cpc20000KHz: return "20MHz";
      case cpc24000KHz: return "24MHz";
      case cpc27000KHz: return "27MHz";
      case cpc32000KHz: return "32MHz";
      case cpc40000KHz: return "40MHz";
      default: return "6MHz";
    }
  }


protected:
  bool is_capturing;

  //DCAM parameters:
  VarInt* v_expose_us;
  VarBool* v_expose_overlapped;
  VarDouble* v_gain_db;
  VarStringEnum* v_hdr_mode;
  VarBool* v_mirror_top_down;
  VarBool* v_mirror_left_right;
  VarDouble* v_wb_red;
  VarDouble* v_wb_green;
  VarDouble* v_wb_blue;
  VarBool* v_sharpen;
  VarDouble* v_gamma;
  VarStringEnum* v_color_twist_mode;
  VarInt    * v_aoi_width;
  VarInt    * v_aoi_height;
  VarInt    * v_aoi_left;
  VarInt    * v_aoi_top;
  VarStringEnum* v_pixel_clock;
  
  //capture variables:
  VarInt    * v_cam_bus;
  VarStringEnum * v_colorout;
  
  VarList * dcam_parameters;
  VarList * capture_settings;
  
  // BlueFox2 specific data
  DeviceManager* pDevMgr;
  Device* pDevice;
  FunctionInterface* pFI;
  CameraSettingsBlueFOX* pSettings;
  ImageProcessing* pImageProc;
  int lastRequestNr;

  unsigned int cam_id;
  ColorFormat capture_format;

public:
    CaptureBlueFox2(VarList * _settings=0,int default_camera_id=0,QObject * parent=0);
    void mvc_connect(VarList * group);
  ~CaptureBlueFox2();

  /// Initialize the interface and start capture
  virtual bool startCapture();

  /// Stop Capture
  virtual bool stopCapture();

  virtual bool isCapturing() { return is_capturing; };

  /// this gives a raw-image with a pointer directly to the video-buffer
  /// Note that this pointer is only guaranteed to point to a valid
  /// memory location until releaseFrame() is called.
  virtual RawImage getFrame();

  virtual void releaseFrame();

  virtual bool resetBus();

  void readParameterValues(VarList * item);

  void writeParameterValues(VarList * item);

  virtual void readAllParameterValues();

  void writeAllParameterValues();

  virtual bool copyAndConvertFrame(const RawImage & src, RawImage & target);

  virtual string getCaptureMethodName() const;
};

#endif

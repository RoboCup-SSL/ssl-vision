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
  \file    capture_spinnaker.h
  \brief   C++ Interface: CaptureSpinnaker
  \author  Nicolai Ommer, (C) 2019
*/
//========================================================================

#ifndef CAPTURE_SPINNAKER_H
#define CAPTURE_SPINNAKER_H
#include "captureinterface.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "VarTypes.h"
#include <spinnaker/SystemPtr.h>
#include <spinnaker/CameraPtr.h>
#include <spinnaker/ImagePtr.h>

// Unset 'interface' from spinnaker/SpinGenApi/Types.h which conflicts with variables in ssl-vision
#undef interface

  #include <QMutex>


/*!
  \class  CaptureSpinnaker
  \brief  A capture class for FLIR Spinnaker cameras
  \author Nicolai Ommer, (C) 2019

  This class provides the ability to use and configure FLIR cameras
  using the Spinnaker driver.

  If you find your camera not working correctly, or discover a bug,
  please inform the author, as we are aiming for complete camera
  coverage.
*/
  #include <QMutex>
  //if using QT, inherit QObject as a base
class CaptureSpinnaker : public QObject, public CaptureInterface
{
  Q_OBJECT
  
  public slots:
  void changed(VarType * group);
  
  protected:
  QMutex mutex;
  
  public:

protected:
  bool is_capturing;

  //capture variables:
  VarInt    * v_cam_bus;
  VarStringEnum * v_convert_to_mode;

  //DCAM parameters:
  VarStringEnum * v_capture_mode;
  VarStringEnum* v_expose_auto;
  VarDouble* v_expose_us;
  VarStringEnum* v_gain_auto;
  VarDouble* v_gain_db;
  VarDouble* v_gamma;
  VarStringEnum* v_white_balance_auto;
  VarStringEnum* v_stream_buffer_handling_mode;
  VarStringEnum* v_stream_buffer_count_mode;
  VarInt* v_stream_buffer_count;
  VarDouble* v_frame_rate;
  VarDouble* v_frame_rate_result;

  VarList * capture_settings;
  VarList * dcam_parameters;
  
  // Spinnaker specific data
  Spinnaker::SystemPtr pSystem;
  Spinnaker::CameraPtr pCam;
  Spinnaker::ImagePtr pImage;

  unsigned int cam_id;

public:
  explicit CaptureSpinnaker(VarList * _settings = nullptr, int default_camera_id = 0, QObject * parent = nullptr);
  void mvc_connect(VarList * group);
  ~CaptureSpinnaker() override;

  /// Initialize the interface and start capture
  bool startCapture() override;

  /// Stop Capture
  bool stopCapture() override;

  bool isCapturing() override { return is_capturing; };

  /// this gives a raw-image with a pointer directly to the video-buffer
  /// Note that this pointer is only guaranteed to point to a valid
  /// memory location until releaseFrame() is called.
  RawImage getFrame() override;

  void releaseFrame() override;

  bool resetBus() override;

  void readParameterValues(VarList * item);

  void writeParameterValues(VarList * item);

  void readAllParameterValues() override;

  void writeAllParameterValues();

  bool copyAndConvertFrame(const RawImage & src, RawImage & target) override;

  string getCaptureMethodName() const override;

private:

    static string toString(Spinnaker::ExposureAutoEnums e) {
      switch(e)
      {
        case Spinnaker::ExposureAuto_Off:
          return "off";
        case Spinnaker::ExposureAuto_Once:
          return "once";
        case Spinnaker::ExposureAuto_Continuous:
          return "continues";
        default:
          return "off";
      }
    }

    static Spinnaker::ExposureAutoEnums stringToExposureAuto(const char* s) {
      if(strcmp(s, "off") == 0) {
        return Spinnaker::ExposureAuto_Off;
      } else if(strcmp(s, "once") == 0) {
        return Spinnaker::ExposureAuto_Once;
      } else if(strcmp(s, "continues") == 0) {
        return Spinnaker::ExposureAuto_Continuous;
      }
      return Spinnaker::ExposureAuto_Off;
    }


    static string toString(Spinnaker::GainAutoEnums e) {
      switch(e)
      {
        case Spinnaker::GainAuto_Off:
          return "off";
        case Spinnaker::GainAuto_Once:
          return "once";
        case Spinnaker::GainAuto_Continuous:
          return "continues";
        default:
          return "off";
      }
    }

    static Spinnaker::GainAutoEnums stringToGainAuto(const char* s) {
      if(strcmp(s, "off") == 0) {
        return Spinnaker::GainAuto_Off;
      } else if(strcmp(s, "once") == 0) {
        return Spinnaker::GainAuto_Once;
      } else if(strcmp(s, "continues") == 0) {
        return Spinnaker::GainAuto_Continuous;
      }
      return Spinnaker::GainAuto_Off;
    }


    static string toString(Spinnaker::BalanceWhiteAutoEnums e) {
      switch(e)
      {
        case Spinnaker::BalanceWhiteAuto_Off:
          return "off";
        case Spinnaker::BalanceWhiteAuto_Once:
          return "once";
        case Spinnaker::BalanceWhiteAuto_Continuous:
          return "continues";
        default:
          return "off";
      }
    }

    static Spinnaker::BalanceWhiteAutoEnums stringToBalanceWhiteAuto(const char* s) {
      if(strcmp(s, "off") == 0) {
        return Spinnaker::BalanceWhiteAuto_Off;
      } else if(strcmp(s, "once") == 0) {
        return Spinnaker::BalanceWhiteAuto_Once;
      } else if(strcmp(s, "continues") == 0) {
        return Spinnaker::BalanceWhiteAuto_Continuous;
      }
      return Spinnaker::BalanceWhiteAuto_Off;
    }


    static string toString(Spinnaker::StreamBufferHandlingModeEnum e) {
      switch(e)
      {
        case Spinnaker::StreamBufferHandlingMode_NewestOnly:
          return "NewestOnly";
        case Spinnaker::StreamBufferHandlingMode_NewestFirst:
          return "NewestFirst";
        case Spinnaker::StreamBufferHandlingMode_OldestFirstOverwrite:
          return "OldestFirstOverwrite";
        case Spinnaker::StreamBufferHandlingMode_OldestFirst:
          return "OldestFirst";
        default:
          return "OldestFirstOverwrite";
      }
    }

    static Spinnaker::StreamBufferHandlingModeEnum stringToStreamBufferHandlingMode(const char* s) {
      if(strcmp(s, "NewestOnly") == 0) {
        return Spinnaker::StreamBufferHandlingMode_NewestOnly;
      } else if(strcmp(s, "NewestFirst") == 0) {
        return Spinnaker::StreamBufferHandlingMode_NewestFirst;
      } else if(strcmp(s, "OldestFirstOverwrite") == 0) {
        return Spinnaker::StreamBufferHandlingMode_OldestFirstOverwrite;
      } else if(strcmp(s, "OldestFirst") == 0) {
        return Spinnaker::StreamBufferHandlingMode_OldestFirst;
      }
      return Spinnaker::StreamBufferHandlingMode_OldestFirstOverwrite;
    }


    static string toString(Spinnaker::StreamBufferCountModeEnum e) {
      switch(e)
      {
        case Spinnaker::StreamBufferCountMode_Auto:
          return "Auto";
        case Spinnaker::StreamBufferCountMode_Manual:
          return "Manual";
        default:
          return "Auto";
      }
    }

    static Spinnaker::StreamBufferCountModeEnum stringToStreamBufferCountMode(const char* s) {
      if(strcmp(s, "Auto") == 0) {
        return Spinnaker::StreamBufferCountMode_Auto;
      } else if(strcmp(s, "Manual") == 0) {
        return Spinnaker::StreamBufferCountMode_Manual;
      }
      return Spinnaker::StreamBufferCountMode_Auto;
    }
};

#endif

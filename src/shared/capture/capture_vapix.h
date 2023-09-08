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
  \file    capture_vapix.h
  \brief   C++ Interface: CaptureVapix
  \author  Sebastian Olsson, (C) 2023
*/
//========================================================================

#ifndef CAPTURE_VAPIX_H
#define CAPTURE_VAPIX_H

#include "captureinterface.h"
#include "VarTypes.h"
#include "TimeSync.h"

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <QMutex>
#include <curl/curl.h>

// Unset 'interface' from spinnaker/SpinGenApi/Types.h which conflicts with variables in ssl-vision
#undef interface

/*!
  \class  CaptureVapix
  \brief  A capture class for Axis Communications network cameras
  \author Sebastian Olsson, (C) 2023

  This class provides the ability to use and configure Axis Communications
  network cameras using the VAPIX API.

  If you find your camera not working correctly, or discover a bug,
  please inform the author, as we are aiming for complete camera
  coverage.
*/
class CaptureVapix : public QObject, public CaptureInterface {
Q_OBJECT

public slots:

    void changed(__attribute__((unused)) VarType *group);

    void slotResetTriggered();

private:
    bool is_capturing;
    bool reset_parameters = false;

    string default_network_prefix = "192.168.1"

    QMutex mutex;
    TimeSync timeSync;

    // capture parameters
    VarInt *v_cam_bus;
    VarString *v_cam_network;
    VarString *v_cam_ip;
    VarStringEnum *v_convert_to_mode;

    // camera parameters
    VarBool *v_acquisition;
    VarStringEnum *v_capture_mode;
    VarStringEnum *v_expose_auto;
    VarDouble *v_expose_us;
    VarStringEnum *v_gain_auto;
    VarDouble *v_gain_db;
    VarDouble *v_gamma;
    VarBool *v_gamma_enabled;
    VarStringEnum *v_white_balance_auto;
    VarDouble *v_white_balance_red;
    VarDouble *v_white_balance_blue;
    VarInt *v_image_width;
    VarInt *v_image_height;
    VarInt *v_image_offset_x;
    VarInt *v_image_offset_y;
    VarBool *v_frame_rate_enable;
    VarDouble *v_frame_rate;
    VarDouble *v_frame_rate_result;
    VarTrigger *v_trigger_reset;

    VarList *capture_settings;
    VarList *dcam_parameters;

    // VAPIX specific data
    const string cam_image_url = "/axis-cgi/jpg/image.cgi";
    CURL* curl;
    CURLcode result;
    string cam_response;


    Spinnaker::SystemPtr pSystem;
    Spinnaker::CameraPtr pCam;
    Spinnaker::ImagePtr pImage;

public:
    explicit CaptureVapix(VarList *_settings = nullptr, int default_camera_id = 0, QObject *parent = nullptr);

    ~CaptureVapix() override;

    bool startCapture() override;

    bool stopCapture() override;

    bool isCapturing() override { return is_capturing; };

    RawImage getFrame() override;

    void releaseFrame() override;

    bool resetBus() override { return true; };

    void readAllParameterValues() override;

    bool copyAndConvertFrame(const RawImage &src, RawImage &target) override;

    string getCaptureMethodName() const override { return "VAPIX"; };

private:
    string toIpString(string network_prefix, int ip);

    void mvc_connect(VarList *group);

    void readParameterValues();

    void writeParameterValues();

    void reloadParameters();

    void init_camera();

    size_t camImageCallback(void* contents, size_t size, size_t nmemb, void* userptr);

    static string toString(Spinnaker::ExposureAutoEnums e) {
      switch (e) {
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

    static Spinnaker::ExposureAutoEnums stringToExposureAuto(const char *s) {
      if (strcmp(s, "off") == 0) {
        return Spinnaker::ExposureAuto_Off;
      } else if (strcmp(s, "once") == 0) {
        return Spinnaker::ExposureAuto_Once;
      } else if (strcmp(s, "continues") == 0) {
        return Spinnaker::ExposureAuto_Continuous;
      }
      return Spinnaker::ExposureAuto_Off;
    }


    static string toString(Spinnaker::GainAutoEnums e) {
      switch (e) {
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

    static Spinnaker::GainAutoEnums stringToGainAuto(const char *s) {
      if (strcmp(s, "off") == 0) {
        return Spinnaker::GainAuto_Off;
      } else if (strcmp(s, "once") == 0) {
        return Spinnaker::GainAuto_Once;
      } else if (strcmp(s, "continues") == 0) {
        return Spinnaker::GainAuto_Continuous;
      }
      return Spinnaker::GainAuto_Off;
    }


    static string toString(Spinnaker::BalanceWhiteAutoEnums e) {
      switch (e) {
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

    static Spinnaker::BalanceWhiteAutoEnums stringToBalanceWhiteAuto(const char *s) {
      if (strcmp(s, "off") == 0) {
        return Spinnaker::BalanceWhiteAuto_Off;
      } else if (strcmp(s, "once") == 0) {
        return Spinnaker::BalanceWhiteAuto_Once;
      } else if (strcmp(s, "continues") == 0) {
        return Spinnaker::BalanceWhiteAuto_Continuous;
      }
      return Spinnaker::BalanceWhiteAuto_Off;
    }

    static void setCameraValueInt(Spinnaker::GenApi::IInteger &cameraValue, VarInt *varValue) {
      if (IsWritable(cameraValue)) {
        varValue->setMin((int) cameraValue.GetMin());
        varValue->setMax((int) cameraValue.GetMax());
        cameraValue.SetValue(cap((int) varValue->getInt(), varValue->getMin(), varValue->getMax()));
        varValue->removeFlags(VARTYPE_FLAG_READONLY);
      } else {
        varValue->addFlags(VARTYPE_FLAG_READONLY);
      }
    }

    static void setCameraValueFloat(Spinnaker::GenApi::IFloat &cameraValue, VarDouble *varValue) {
      if (IsWritable(cameraValue)) {
        varValue->setMin(cameraValue.GetMin());
        varValue->setMax(cameraValue.GetMax());
        cameraValue.SetValue(cap(varValue->getDouble(), varValue->getMin(), varValue->getMax()));
        varValue->removeFlags(VARTYPE_FLAG_READONLY);
      } else {
        varValue->addFlags(VARTYPE_FLAG_READONLY);
      }
    }

    static void getCameraValueInt(Spinnaker::GenApi::IInteger &cameraValue, VarInt *varValue) {
      if (IsReadable(cameraValue)) {
        varValue->setMin((int) cameraValue.GetMin());
        varValue->setMax((int) cameraValue.GetMax());
        varValue->setInt((int) cameraValue.GetValue());
      }

      if (IsWritable(cameraValue)) {
        varValue->removeFlags(VARTYPE_FLAG_READONLY);
      } else {
        varValue->addFlags(VARTYPE_FLAG_READONLY);
      }
    }

    static void getCameraValueFloat(Spinnaker::GenApi::IFloat &cameraValue, VarDouble *varValue) {
      if (IsReadable(cameraValue)) {
        varValue->setMin(cameraValue.GetMin());
        varValue->setMax(cameraValue.GetMax());
        varValue->setDouble(cameraValue.GetValue());
      }

      if (IsWritable(cameraValue)) {
        varValue->removeFlags(VARTYPE_FLAG_READONLY);
      } else {
        varValue->addFlags(VARTYPE_FLAG_READONLY);
      }
    }

    static double cap(double value, double v_min, double v_max) {
      return max(v_min, min(v_max, value));
    }

    static int cap(int value, int v_min, int v_max) {
      return max(v_min, min(v_max, value));
    }
};

#endif

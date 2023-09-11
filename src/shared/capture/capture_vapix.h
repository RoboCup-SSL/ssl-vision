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
#include <opencv2/opencv.hpp>

/*!
  \class  CaptureVapix
  \brief  A capture class for Axis Communications network cameras
  \author Sebastian Olsson, (C) 2023

  This class provides the ability to use Axis Communications
  network cameras using HTTP requests to the VAPIX API.

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


    QMutex mutex;
    TimeSync timeSync;

    // capture parameters
    VarInt *v_cam_bus;
    //VarString *v_cam_network_prefix;
    VarString *v_cam_ip;
    VarString *v_cam_port;
    VarString *v_cam_username;
    VarString *v_cam_password;
    VarStringEnum *v_convert_to_mode;

    // camera parameters
    // VarBool *v_acquisition;
    VarStringEnum *v_capture_mode;
    // VarStringEnum *v_expose_auto;
    // VarDouble *v_expose_us;
    // VarStringEnum *v_gain_auto;
    // VarDouble *v_gain_db;
    // VarDouble *v_gamma;
    // VarBool *v_gamma_enabled;
    // VarStringEnum *v_white_balance_auto;
    // VarDouble *v_white_balance_red;
    // VarDouble *v_white_balance_blue;
    // VarInt *v_image_width;
    // VarInt *v_image_height;
    // VarInt *v_image_offset_x;
    // VarInt *v_image_offset_y;
    // VarBool *v_frame_rate_enable;
    // VarDouble *v_frame_rate;
    // VarDouble *v_frame_rate_result;
    // VarTrigger *v_trigger_reset;

    VarList *capture_settings;
    VarList *dcam_parameters;

    // VAPIX specific data
    const string image_route = "/axis-cgi/jpg/image.cgi";
    const string video_stream_route = "/axis-media/media.amp";
    cv::VideoCapture camera;
    cv::Mat p_image;
    string cam_response;
    static int num_cams; // Shared cam variable between instances

public:
    explicit CaptureVapix(VarList *_settings = nullptr, int default_camera_id = 0, QObject *parent = nullptr);

    ~CaptureVapix() override;

    bool startCapture() override;

    bool stopCapture() override;

    RawImage getFrame() override;

    bool isCapturing() override { return is_capturing; };

    string getCaptureMethodName() const override { return "VAPIX"; };

    bool copyAndConvertFrame(const RawImage & src, RawImage & target) override;

    bool resetBus() override {return true;};
    void releaseFrame() override;


private:

    // void readParameterValues();

    // void writeParameterValues();

    // void reloadParameters();

    // void init_camera();

    // static void setCameraValueInt(Spinnaker::GenApi::IInteger &cameraValue, VarInt *varValue) {
    //   if (IsWritable(cameraValue)) {
    //     varValue->setMin((int) cameraValue.GetMin());
    //     varValue->setMax((int) cameraValue.GetMax());
    //     cameraValue.SetValue(cap((int) varValue->getInt(), varValue->getMin(), varValue->getMax()));
    //     varValue->removeFlags(VARTYPE_FLAG_READONLY);
    //   } else {
    //     varValue->addFlags(VARTYPE_FLAG_READONLY);
    //   }
    // }

    // static void setCameraValueFloat(Spinnaker::GenApi::IFloat &cameraValue, VarDouble *varValue) {
    //   if (IsWritable(cameraValue)) {
    //     varValue->setMin(cameraValue.GetMin());
    //     varValue->setMax(cameraValue.GetMax());
    //     cameraValue.SetValue(cap(varValue->getDouble(), varValue->getMin(), varValue->getMax()));
    //     varValue->removeFlags(VARTYPE_FLAG_READONLY);
    //   } else {
    //     varValue->addFlags(VARTYPE_FLAG_READONLY);
    //   }
    // }

    // static void getCameraValueInt(Spinnaker::GenApi::IInteger &cameraValue, VarInt *varValue) {
    //   if (IsReadable(cameraValue)) {
    //     varValue->setMin((int) cameraValue.GetMin());
    //     varValue->setMax((int) cameraValue.GetMax());
    //     varValue->setInt((int) cameraValue.GetValue());
    //   }

    //   if (IsWritable(cameraValue)) {
    //     varValue->removeFlags(VARTYPE_FLAG_READONLY);
    //   } else {
    //     varValue->addFlags(VARTYPE_FLAG_READONLY);
    //   }
    // }

    // static void getCameraValueFloat(Spinnaker::GenApi::IFloat &cameraValue, VarDouble *varValue) {
    //   if (IsReadable(cameraValue)) {
    //     varValue->setMin(cameraValue.GetMin());
    //     varValue->setMax(cameraValue.GetMax());
    //     varValue->setDouble(cameraValue.GetValue());
    //   }

    //   if (IsWritable(cameraValue)) {
    //     varValue->removeFlags(VARTYPE_FLAG_READONLY);
    //   } else {
    //     varValue->addFlags(VARTYPE_FLAG_READONLY);
    //   }
    // }

    // static double cap(double value, double v_min, double v_max) {
    //   return max(v_min, min(v_max, value));
    // }

    // static int cap(int value, int v_min, int v_max) {
    //   return max(v_min, min(v_max, value));
    // }
};

#endif

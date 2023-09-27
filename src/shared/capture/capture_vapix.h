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

#include <atomic>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <QMutex>
#include <QThread>
#include <QSemaphore>
#include <opencv2/opencv.hpp>

class RTSPReader : public QThread {
public:
  explicit RTSPReader(QObject* parent = nullptr): QThread(parent) {}

  cv::Mat getFrame() {
    sema.acquire();
    return buffer_a;
  }
  
  bool open(const std::string url) {
    if (!capture.open(url, cv::CAP_FFMPEG)) {
      return false;
    }
    running.store(true);
    return true;
  }

  void run() override {
    while (running.load()) {
      capture.read(buffer_b);
      if (sema.available() == 0) {
        buffer_a = buffer_b;
        buffer_b = cv::Mat();
        sema.release();
      }
    }
  }

  void close() {
    capture.release();
    running.store(false);
  }

  

private:
  
  cv::Mat buffer_a, buffer_b;
  QSemaphore sema;

  cv::VideoCapture capture;
  std::atomic_bool running {false};

};

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

    VarStringEnum *v_capture_mode;

    VarList *capture_settings;
    VarList *dcam_parameters;

    // VAPIX specific data
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
};
#endif

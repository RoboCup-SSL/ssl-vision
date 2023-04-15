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
//
//========================================================================
// Some portions of software interfacing with XIO originally writtten
// by James Bruce as part of the CMVision library.
//      http://www.cs.cmu.edu/~jbruce/cmvision/
// (C) 2004-2006 James R. Bruce, Carnegie Mellon University
// Licenced under the GNU General Public License (GPL) version 2,
//   or alternately by a specific written agreement.
//========================================================================
// (C) 2016 Eric Zavesky, Emoters
// Licenced under the GNU General Public License (GPL) version 3,
//   or alternately by a specific written agreement.
//========================================================================
/*!
 \file    capturev4l.h
 \brief   C++ Interface: CaptureV4L
 \author  Eric Zavesky, (C) 2016 (derived from DC1394 and CMVision)
 */
//========================================================================

#ifndef CAPTUREV4l_H
#define CAPTUREV4l_H

#include "captureinterface.h"
#include "util.h"
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "VarTypes.h"
#include <linux/videodev2.h>
#include <sys/poll.h>
#include <jpeglib.h>

#include <map>

#include <QMutex>

typedef long v4lfeature_t;

// if you get a message like "DQBUF returned error", "DQBUF error: invalid"
// then you need to use a higher value for STREAMBUFS or process frames faster
#define V4L_STREAMBUFS            3

// number of usb devices to scan as potential camera sources
#define MAX_CAM_SCAN                4

// place-holder for friend relationship
class GlobalV4LinstanceManager;

/*!
 \class GlobalV4Linstance
 \brief A singleton provider of a v4l lib context used for capturing with multiple threads
 \author  Eric Zavesky, (C) 2016 (derived from DC1394 interface)
 */
class GlobalV4Linstance {
    friend class GlobalV4LinstanceManager;

public:
    struct image_t {
        unsigned char *data;
        size_t length;
        int index;
        uint8_t field;        // which field of video this is from {0,1}
        timeval timestamp;
    };
    enum private_control_t {    //custom control codes that must be individually caught...
        V4L2_FEATURE_PRIVATE = V4L2_CID_PRIVATE_BASE,
        V4L2_FEATURE_FRAME_RATE
    };
protected:
    GlobalV4Linstance() {
      pollset.fd = -1;
    }

    ~GlobalV4Linstance() {
      while (counter) removeInstance();       //if iterating, release control first
    }

    QMutex mutex;
private:
    pollfd pollset = {};
    char counter = 0;
    char szDevice[128] = {};
    struct v4l2_buffer tempbuf = {};
    image_t img[V4L_STREAMBUFS] = {};

    bool enqueueBuffer(v4l2_buffer &buf);

    bool dequeueBuffer(v4l2_buffer &buf);

    bool waitForFrame(int max_msec = 500);

    bool obtainInstance(int iDevice);

    bool obtainInstance(char *szDevice);

    bool removeInstance(bool bRelock = true);

    static int xioctl(int fd, int request, void *data, const char *error_str);

    bool xioctl(int request, void *data, const char *error_str) const;

public:
    bool getControl(long ctrl_id, long &s);

    bool setControl(long ctrl_id, long s);

    bool checkControl(
            long ctrl_id,
            bool *bEnabled = nullptr,
            bool *bReadOnly = nullptr,
            long *lDefault = nullptr,
            long *lMin = nullptr,
            long *lMax = nullptr
    );

    bool startStreaming(int iWidth_, int iHeight_, uint32_t pixel_format, int framerate, int iInput = 0);

    bool stopStreaming();

    void captureWarm(int iMaxSpin = 1);

    bool captureFrame(RawImage *pImage, uint32_t pixel_format, int iMaxSpin = 1);

    const image_t *captureFrame(int iMaxSpin = 1);

    bool releaseFrame(const image_t *_img);

private:
    void lock() {
      mutex.lock();
    }

    void unlock() {
      mutex.unlock();
    }

// utility functions for testing before ssl-vision
public:
    struct yuyv {
        uchar y1, u, y2, v;
    };
    struct rgb {
        uchar red, green, blue;
    };
    struct yuv {
        uchar y, u, v;
    };

    static bool writeYuyvPPM(GlobalV4Linstance::yuyv *pSrc, int width, int height, const char *filename);

    static bool writeRgbPPM(GlobalV4Linstance::rgb *imgbuf, int width, int height, const char *filename);

private:
    static bool getImageRgb(GlobalV4Linstance::yuyv *pSrc, int width, int height, GlobalV4Linstance::rgb **rgbbuf);

    static void jpegErrorExit(j_common_ptr dinfo);

    static bool getImageFromJPEG(const image_t &in_img, RawImage *out_img);

    static bool getImage(const image_t &in_img, uint32_t pixel_format, RawImage *out_img);

    static GlobalV4Linstance::rgb yuv2rgb(GlobalV4Linstance::yuv p);

};


/*!
 \class GlobalV4LinstanceManager
 \brief A static instance manager to provide global singleton access to GlobalV4Linstance
 \author  Eric Zavesky, (C) 2016
 */
class GlobalV4LinstanceManager {
public:
    static GlobalV4Linstance *obtainInstance(int iDevice);

    static bool removeInstance(GlobalV4Linstance *pDevice);

protected:
    GlobalV4LinstanceManager() {};

    ~GlobalV4LinstanceManager();

private:
    static GlobalV4LinstanceManager *pinstance;
    typedef std::map<int, GlobalV4Linstance *> t_map_v4l;
    t_map_v4l map_instance;
};

/*!
 \class CaptureV4L
 \brief A v4l-based USB/Video For Linux Capture Class
 \author  Eric Zavesky, (C) 2016

 This class mirrors popular v4l implementations giving the most
 compatible options and based on the DC1394 base classes by Stefan.

 Overall, it not only provides capture-abilities, but also full
 on-the-fly configuration abilities through the VarTypes system.

 If you find your camera not working correctly, or discover a bug,
 please inform the author, but also consider contributing yourself,
 so that we can cover as many options as possible.
 */
#include <QMutex>

//if using QT, inherit QObject as a base
class CaptureV4L : public QObject, public CaptureInterface {
Q_OBJECT
public slots:

    void changed(VarType *group);

protected:
    QMutex mutex;

protected:
    bool is_capturing = false;

    //processing variables:
    VarStringEnum *v_colorout;

    //DCAM parameters:
    VarList *P_BRIGHTNESS;
    VarList *P_CONTRAST;
    VarList *P_EXPOSURE;
    VarList *P_SHARPNESS;
    VarList *P_WHITE_BALANCE;
    VarList *P_HUE;
    VarList *P_SATURATION;
    VarList *P_GAMMA;
    VarList *P_GAIN;
    VarList *P_TEMPERATURE;  // white balance temperature
    VarList *P_FRAME_RATE;

    //capture variables:
    VarInt *v_cam_bus;
    VarInt *v_fps;
    VarInt *v_width;
    VarInt *v_height;
    VarInt *v_left;
    VarInt *v_top;
    VarStringEnum *v_colormode;
    std::unique_ptr<VarStringEnum> v_format;
    VarInt *v_buffer_size;

    int width = 0;
    int height = 0;
    int top = 0;
    int left = 0;
    ColorFormat capture_format = COLOR_UNDEFINED;
    uint32_t pixel_format = 0;
    RawImage rawFrame;

    GlobalV4Linstance *camera_instance = nullptr;

    VarList *dcam_parameters;
    VarList *capture_settings;
    VarList *conversion_settings;

public:
    explicit CaptureV4L(VarList *_settings = nullptr, int default_camera_id = 0, QObject *parent = nullptr);

    void mvc_connect(VarList *group);

    ~CaptureV4L() override = default;

    bool startCapture() override;

    bool stopCapture() override;

    bool isCapturing() override { return is_capturing; };

    RawImage getFrame() override;

    void releaseFrame() override;

    bool resetBus() override { return true; };

    void readAllParameterValues() override;

    bool copyAndConvertFrame(const RawImage &src, RawImage &target) override;

    string getCaptureMethodName() const override { return "V4L"; };

private:
    /// This function converts a local variable pointer to enum
    v4lfeature_t getV4LfeatureEnum(VarList *val, bool &valid);

    void readParameterValues(VarList *item);

    void writeParameterValues(VarList *item);

    void readParameterProperty(VarList *item);

    void readAllParameterProperties();

    void writeAllParameterValues();
};

#endif

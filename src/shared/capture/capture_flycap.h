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
 \file    capture_flycap.h
 \brief   C++ Interface: CaptureFlycap
 \author  Spencer Lane, (C) 2017
 */
//========================================================================

#ifndef CAPTURE_FLYCAP_H
#define CAPTURE_FLYCAP_H
#include "FlyCapture2.h"
#include "captureinterface.h"
#include "util.h"
#include <QMutex>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <dc1394/control.h>
#include <dc1394/conversions.h>
#include "VarTypes.h"


typedef struct {
    QMutex *mutex;
    unsigned char* imageBuffer;
    bool isNewBuffer;
    unsigned int width, height;
} ApplicationData;

enum class FlyCapCamType { 
    GIGE,
    GENERIC,
    UNKNOWN
};


ColorFormat pixelFormatToColorFormat(FlyCapture2::PixelFormat format);

//if using QT, inherit QObject as a base
class CaptureFlycap : public QObject, public CaptureInterface {
    Q_OBJECT
    public Q_SLOTS:
    void changed(VarType * group);
protected:
    QMutex mutex;
public:

static dc1394color_filter_t stringToColorFilter(const char * s) {
    if (strcmp(s,"rggb")==0) {
      return DC1394_COLOR_FILTER_RGGB;
    } else if (strcmp(s,"gbrg")==0) {
      return DC1394_COLOR_FILTER_GBRG;
    } else if (strcmp(s,"grbg")==0) {
      return DC1394_COLOR_FILTER_GRBG;
    } else if (strcmp(s,"bggr")==0) {
      return DC1394_COLOR_FILTER_BGGR;
    } else {
      return DC1394_COLOR_FILTER_MIN;
    }
}

static string colorFilterToString(dc1394color_filter_t f) {
  if (f==DC1394_COLOR_FILTER_RGGB) {
    return "rggb";
  } else if (f==DC1394_COLOR_FILTER_GBRG) {
    return "gbrg";
  } else if (f==DC1394_COLOR_FILTER_GRBG) {
    return "grbg";
  } else if (f==DC1394_COLOR_FILTER_BGGR) {
    return "bggr";
  } else {
    return "rggb";
  }
}

static dc1394bayer_method_t stringToBayerMethod(const char * s) {
  if (strcmp(s,"nearest")==0) {
    return DC1394_BAYER_METHOD_NEAREST;
  } else if (strcmp(s,"simple")==0) {
    return DC1394_BAYER_METHOD_SIMPLE;
  } else if (strcmp(s,"bilinear")==0) {
    return DC1394_BAYER_METHOD_BILINEAR;
  } else if (strcmp(s,"hqlinear")==0) {
    return DC1394_BAYER_METHOD_HQLINEAR;
  } else if (strcmp(s,"downsample")==0) {
    return DC1394_BAYER_METHOD_DOWNSAMPLE;
  } else if (strcmp(s,"edgesense")==0) {
    return DC1394_BAYER_METHOD_EDGESENSE;
  } else if (strcmp(s,"vng")==0) {
    return DC1394_BAYER_METHOD_VNG;
  } else if (strcmp(s,"ahd")==0) {
    return DC1394_BAYER_METHOD_AHD;
  } else {
    return DC1394_BAYER_METHOD_MIN;
  }
}

static string bayerMethodToString(dc1394bayer_method_t f) {
  if (f==DC1394_BAYER_METHOD_NEAREST) {
    return "nearest";
  } else if (f==DC1394_BAYER_METHOD_SIMPLE) {
    return "simple";
  } else if (f==DC1394_BAYER_METHOD_BILINEAR) {
    return "bilinear";
  } else if (f==DC1394_BAYER_METHOD_HQLINEAR) {
    return "hqlinear";
  } else if (f==DC1394_BAYER_METHOD_DOWNSAMPLE) {
    return "downsample";
  } else if (f==DC1394_BAYER_METHOD_EDGESENSE) {
    return "edgesense";
  } else if (f==DC1394_BAYER_METHOD_VNG) {
    return "vng";
  } else if (f==DC1394_BAYER_METHOD_AHD) {
    return "ahd";
  } else {
    return "nearest";
  }
}

protected:

    ApplicationData cbData;
    const char* dev_name;

    unsigned char* currentFrame;

    bool is_capturing;
    bool settings_changed;
    string prev_mode_string;
    bool is_connected;

    //processing variables:
    VarStringEnum * v_colorout;
    VarStringEnum * v_debayer_pattern;
    VarStringEnum * v_debayer_method;
    VarInt        * v_debayer_y16;
    VarBool       * v_debayer;

    //DCAM parameters:
    VarList * P_WHITE_BALANCE;

    //capture variables:
    VarInt    * v_cam_bus;
    VarInt    * v_width;
    VarInt    * v_height;
    VarInt    * v_xoffset;
    VarInt    * v_yoffset;
    VarStringEnum * v_videomode;
    VarStringEnum * v_colormode;

    int previous_id;
    unsigned int cam_id;
    ColorFormat capture_format;
    VarList * camera_parameters;
    VarList * capture_settings;
    VarList * conversion_settings;

    // Capture Variables
    FlyCapCamType camera_type;
    FlyCapture2::CameraBase * camera;
    FlyCapture2::PGRGuid guid;
    FlyCapture2::Image * stored_image;

    static std::vector<unsigned int> active_cam_ids;
    static QMutex cam_id_mutex;

public:
    CaptureFlycap(VarList * _settings=0,int default_camera_id=0,QObject * parent=0);
    void mvc_connect(VarList * group);
    ~CaptureFlycap();

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

    void cleanup();

    void readParameterValues(VarList * item);

    void writeParameterValues(VarList * item);

    /// will perform a pure memcpy of an image, independent of vartypes
    /// conversion settings
    bool copyFrame(const RawImage & src, RawImage & target);

    virtual void readAllParameterValues();

    void writeAllParameterValues();

    virtual bool copyAndConvertFrame(const RawImage & src, RawImage & target);

    virtual string getCaptureMethodName() const;

protected:
    /// This function is used internally only
    /// The user should call copyAndConvertFrame with parameters setup through the
    /// VarTypes settings.
    bool convertFrame(const RawImage & src,
                           RawImage & target,
                           ColorFormat output_fmt,
                           bool debayer=true,
                           dc1394color_filter_t bayer_format=DC1394_COLOR_FILTER_RGGB,
                           dc1394bayer_method_t bayer_method=DC1394_BAYER_METHOD_HQLINEAR,
                           int y16bits=16);
};

#endif //  CAPTURE_FLYCAP_H

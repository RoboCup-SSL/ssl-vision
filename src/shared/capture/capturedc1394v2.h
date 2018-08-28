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
  \file    capturedc1394v2.h
  \brief   C++ Interface: CaptureDC1394v2
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef CAPTUREDC1394V2_H
#define CAPTUREDC1394V2_H
#include "captureinterface.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "VarTypes.h"
#include <dc1394/control.h>
#include <dc1394/conversions.h>

//#include "conversions.h"
#ifndef VDATA_NO_QT
  #include <QMutex>
#else
  #include <pthread.h>
#endif


/*!
  \class GlobalCaptureDC1394instance
  \brief A singleton provider of a dc1394 lib context used for capturing with multiple threads
  \author  Stefan Zickler, (C) 2008
*/
class GlobalCaptureDC1394instance
{
  public:
  GlobalCaptureDC1394instance() {
    #ifndef VDATA_NO_QT
    #else
      pthread_mutex_init (&mutex);
    #endif
    counter=0;
    dc_instance=0;
  }
  ~GlobalCaptureDC1394instance() {
    #ifndef VDATA_NO_QT
    #else
      pthread_mutex_destroy (&mutex);
    #endif
  }
  #ifndef VDATA_NO_QT
    QMutex mutex;
  #else
    pthread_mutex_t mutex;
  #endif
  protected:
  char counter;
  dc1394_t* dc_instance;
  public:
  dc1394_t* obtainInstance() {
    lock();
    if (counter == 0) {
      dc_instance=dc1394_new();
    }
    counter++;
    dc1394_t* inst=dc_instance;

    unlock();
    return inst;
  }
  bool removeInstance() {
    lock();
    counter--;
    bool was_last=false;
    if (counter == 0) {
      dc1394_free( dc_instance );
      was_last=true;
    }
    if (counter < 0) {
      fprintf(stderr,"WARNING: Attempting to remove more instances than have been created!\n");
      fflush(stderr);
      counter = 0;
    }
    unlock();
    return was_last;
  }

  private:
  void lock() {
    #ifndef VDATA_NO_QT
      mutex.lock();
    #else
      pthread_mutex_lock();
    #endif
  }
  void unlock() {
    #ifndef VDATA_NO_QT
      mutex.unlock();
    #else
      pthread_mutex_unlock();
    #endif
  }

};


/*!
  \class GlobalCaptureDC1394instanceManager
  \brief A static instance manager to provide global singleton access to GlobalCaptureDC1394instance
  \author  Stefan Zickler, (C) 2008
*/
class GlobalCaptureDC1394instanceManager
  {
  public:
      static dc1394_t* obtainInstance();
      static bool removeInstance();
  protected:
      GlobalCaptureDC1394instanceManager();
      ~GlobalCaptureDC1394instanceManager();
      GlobalCaptureDC1394instanceManager(const GlobalCaptureDC1394instanceManager&);
      GlobalCaptureDC1394instanceManager& operator= (const GlobalCaptureDC1394instanceManager&);
  private:
      static GlobalCaptureDC1394instanceManager* pinstance;
      GlobalCaptureDC1394instance * instance;
 };



/*!
  \class CaptureDC1394v2
  \brief A libdc1394v2-based Firewire Capture Class
  \author  Stefan Zickler, (C) 2008

  This class should provide fairly complete capture abilities for most
  common DCAM-based firewire cameras.
  It supports seemless configuration of
  debayering, firewire 800, and advanced parameter configurations.

  Overall, it not only provides capture-abilities, but also full
  on-the-fly configuration abilities through the VarTypes system.
  
  Using this capture-method, you should no longer be required
  to use third party tools, such as Coriander, to configure your
  firewire cameras before capturing.
  
  If you find your camera not working correctly, or discover a bug,
  please inform the author, as we are aiming for complete camera
  coverage.
*/
#ifndef VDATA_NO_QT
  #include <QMutex>
  //if using QT, inherit QObject as a base
class CaptureDC1394v2 : public QObject, public CaptureInterface
#else
class CaptureDC1394v2 : public CaptureInterface
#endif
{
#ifndef VDATA_NO_QT
  Q_OBJECT
  public slots:
  void changed(VarType * group);
  protected:
  QMutex mutex;
  public:
#endif


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

enum CaptureMode {
  CAPTURE_MODE_AUTO, //auto will attempt native first, then format_7_mode_0, then give up.
  CAPTURE_MODE_NATIVE,
  CAPTURE_MODE_FORMAT_7_MODE_0,
  CAPTURE_MODE_FORMAT_7_MODE_1,
  CAPTURE_MODE_FORMAT_7_MODE_2,
  CAPTURE_MODE_FORMAT_7_MODE_3,
  CAPTURE_MODE_FORMAT_7_MODE_4,
  CAPTURE_MODE_FORMAT_7_MODE_5,
  CAPTURE_MODE_FORMAT_7_MODE_6,
  CAPTURE_MODE_FORMAT_7_MODE_7,
};
#define CAPTURE_MODE_MIN CAPTURE_MODE_AUTO
#define CAPTURE_MODE_MAX CAPTURE_MODE_FORMAT_7_MODE_7

  static CaptureMode stringToCaptureMode(const char * s)
  {
    if (strcmp(s,"auto")==0) {
      return CAPTURE_MODE_AUTO;
    } else if (strcmp(s,"native")==0) {
      return CAPTURE_MODE_NATIVE;
    } else if (strcmp(s,"format7_0")==0) {
      return CAPTURE_MODE_FORMAT_7_MODE_0;
    } else if (strcmp(s,"format7_1")==0) {
      return CAPTURE_MODE_FORMAT_7_MODE_1;
    } else if (strcmp(s,"format7_2")==0) {
      return CAPTURE_MODE_FORMAT_7_MODE_2;
    } else if (strcmp(s,"format7_3")==0) {
      return CAPTURE_MODE_FORMAT_7_MODE_3;
    } else if (strcmp(s,"format7_4")==0) {
      return CAPTURE_MODE_FORMAT_7_MODE_4;
    } else if (strcmp(s,"format7_5")==0) {
      return CAPTURE_MODE_FORMAT_7_MODE_5;
    } else if (strcmp(s,"format7_6")==0) {
      return CAPTURE_MODE_FORMAT_7_MODE_6;
    } else if (strcmp(s,"format7_7")==0) {
      return CAPTURE_MODE_FORMAT_7_MODE_7;
    } else {
      return CAPTURE_MODE_AUTO;
    }
  }

  static string captureModeToString(CaptureMode f)
  {
    if (f==CAPTURE_MODE_AUTO) {
      return ("auto");
    } else if (f==CAPTURE_MODE_NATIVE) {
      return ("native");
    } else if (f==CAPTURE_MODE_FORMAT_7_MODE_0) {
      return ("format7_0");
    } else if (f==CAPTURE_MODE_FORMAT_7_MODE_1) {
      return ("format7_1");
    } else if (f==CAPTURE_MODE_FORMAT_7_MODE_2) {
      return ("format7_2");
    } else if (f==CAPTURE_MODE_FORMAT_7_MODE_3) {
      return ("format7_3");
    } else if (f==CAPTURE_MODE_FORMAT_7_MODE_4) {
      return ("format7_4");
    } else if (f==CAPTURE_MODE_FORMAT_7_MODE_5) {
      return ("format7_5");
    } else if (f==CAPTURE_MODE_FORMAT_7_MODE_6) {
      return ("format7_6");
    } else if (f==CAPTURE_MODE_FORMAT_7_MODE_7) {
      return ("format7_7");
    } else {
      return ("auto");
    }
  }


protected:
  bool is_capturing;

  //processing variables:
  VarBool       * v_debayer;
  VarStringEnum * v_debayer_pattern;
  VarStringEnum * v_debayer_method;
  VarInt        * v_debayer_y16;
  VarStringEnum * v_colorout;

  //DCAM parameters:
  VarList * P_BRIGHTNESS;
  VarList * P_EXPOSURE;
  VarList * P_SHARPNESS;
  VarList * P_WHITE_BALANCE;
  VarList * P_HUE;
  VarList * P_SATURATION;
  VarList * P_GAMMA;
  VarList * P_SHUTTER;
  VarList * P_GAIN;
  VarList * P_IRIS;
  VarList * P_FOCUS;
  VarList * P_TEMPERATURE;
  VarList * P_TRIGGER;
  VarList * P_TRIGGER_DELAY;
  VarList * P_WHITE_SHADING;
  VarList * P_FRAME_RATE;

  //capture variables:
  VarInt    * v_cam_bus;
  VarInt    * v_fps;
  VarInt    * v_width;
  VarInt    * v_height;
  VarInt    * v_left;
  VarInt    * v_top;
  VarStringEnum * v_colormode;
  VarStringEnum * v_format;
  VarBool   * v_use1394B;
  VarBool   * v_use_iso_800;
  VarInt    * v_buffer_size;

  unsigned int cam_id;
  int width;
  int height;
  int top;
  int left;
  ColorFormat capture_format;
  int ring_buffer_size;
  dc1394camera_list_t * cam_list;

  dc1394_t * dc1394_instance;
  dc1394framerate_t dcfps;
  dc1394video_mode_t dcformat;
  dc1394featureset_t features;
  dc1394video_frame_t * frame;
  //dc1394camera_t **cameras;
  dc1394camera_t * camera;

  VarList * dcam_parameters;
  VarList * capture_settings;
  VarList * conversion_settings;

public:
  #ifndef VDATA_NO_QT
    CaptureDC1394v2(VarList * _settings=0,int default_camera_id=0,QObject * parent=0);
    void mvc_connect(VarList * group);
  #else
    CaptureDC1394v2(VarList * _settings=0,int default_camera_id=0);
  #endif
  ~CaptureDC1394v2();

  /// Initialize the interface and start capture
  virtual bool startCapture();

  /// Stop Capture
  virtual bool stopCapture();

  virtual bool isCapturing() { return is_capturing; };

  /// This function converts a local variable pointer
  /// to a DC1394 feature enum
  dc1394feature_t getDC1394featureEnum(VarList * val, bool & valid);
  dc1394feature_t getDC1394featureEnum(VarList * val) {
    bool b=true;
    return getDC1394featureEnum(val,b);
  }

  /// This function converts a DC1394 feature into a
  /// local variable Pointer
  VarList * getVariablePointer(dc1394feature_t val);

  /// this gives a raw-image with a pointer directly to the video-buffer
  /// Note that this pointer is only guaranteed to point to a valid
  /// memory location until releaseFrame() is called.
  virtual RawImage getFrame();

  virtual void releaseFrame();

  virtual bool resetBus();

  void cleanup();

  void readParameterValues(VarList * item);

  void writeParameterValues(VarList * item);

  void readParameterProperty(VarList * item);

  void readAllParameterProperties();

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

#endif

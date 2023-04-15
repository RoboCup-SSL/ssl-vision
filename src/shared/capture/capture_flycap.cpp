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
// Some portions of software interfacing with XIO originally writtten
// by James Bruce as part of the CMVision library.
//      http://www.cs.cmu.edu/~jbruce/cmvision/
// (C) 2004-2006 James R. Bruce, Carnegie Mellon University
// Licenced under the GNU General Public License (GPL) version 2,
//   or alternately by a specific written agreement.
//========================================================================
/*!
 \file    capture_flycap.cpp
 \brief   C++ Interface: CaptureFlycap
 \author  Spencer Lane, (C) 2016
 */
//========================================================================

#include "capture_flycap.h"
#include "capturedc1394v2.h"
#include "FlyCapture2.h"
#include "conversions.h"
#include "colors.h"
#include "VarTypes.h"
#include <iostream>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      //open
#include <unistd.h>     //close

using namespace FlyCapture2;

std::vector<unsigned int> CaptureFlycap::active_cam_ids;
QMutex CaptureFlycap::cam_id_mutex;

ColorFormat pixelFormatToColorFormat(PixelFormat pixel_format)
{
   ColorFormat format;
   //fprintf(stderr, "Pixel Format: %i\n",(int)pixel_format);
   switch(pixel_format) {
     case PIXEL_FORMAT_MONO8: format = COLOR_MONO8; break;
     case PIXEL_FORMAT_411YUV8: format = COLOR_YUV411; break;//< YUV 4:1:1.
     case PIXEL_FORMAT_444YUV8: format = COLOR_YUV444; break;//< YUV 4:4:4.
     case PIXEL_FORMAT_RGB8: format = COLOR_RGB8; break;//< R = G = B = 8 bits.
     case PIXEL_FORMAT_MONO16: format = COLOR_MONO16; break;//< 16 bits of mono information.
     case PIXEL_FORMAT_RGB16: format = COLOR_RGB16; break;//< R = G = B = 16 bits.
     case PIXEL_FORMAT_RAW8: format = COLOR_RAW8; break;//< 8 bit raw data output of sensor.
     case PIXEL_FORMAT_RAW16: format = COLOR_RAW16; break;//< 16 bit raw data output of sensor.
     case PIXEL_FORMAT_422YUV8: format = COLOR_YUV422_UYVY; break;
     case UNSPECIFIED_PIXEL_FORMAT: format = COLOR_UNDEFINED; break; //< Unspecified pixel format.
     default: format = COLOR_UNDEFINED; break;
   }
   //fprintf(stderr,"Color format: %i\n",(int)format);

   return format;
}

PixelFormat colorFormatToPixelFormat(ColorFormat color_format)
{
   PixelFormat format;
   //fprintf(stderr, "Pixel Format: %i\n",(int)pixel_format);
   switch(color_format) {
     case COLOR_MONO8: format = PIXEL_FORMAT_MONO8; break;
     case COLOR_YUV411: format = PIXEL_FORMAT_411YUV8; break;//< YUV 4:1:1.
     case COLOR_YUV444: format = PIXEL_FORMAT_444YUV8; break;//< YUV 4:4:4.
     case COLOR_RGB8: format = PIXEL_FORMAT_RGB8; break;//< R = G = B = 8 bits.
     case COLOR_MONO16: format = PIXEL_FORMAT_MONO16; break;//< 16 bits of mono information.
     case COLOR_RGB16: format = PIXEL_FORMAT_RGB16; break;//< R = G = B = 16 bits.
     case COLOR_RAW8: format = PIXEL_FORMAT_RAW8; break;//< 8 bit raw data output of sensor.
     case COLOR_RAW16: format = PIXEL_FORMAT_RAW16; break;//< 16 bit raw data output of sensor.
     case COLOR_YUV422_UYVY: format = PIXEL_FORMAT_422YUV8; break;
     case COLOR_UNDEFINED: format = UNSPECIFIED_PIXEL_FORMAT; break; //< Unspecified pixel format.
     default: format = UNSPECIFIED_PIXEL_FORMAT; break;
   }
  return format;
}

Mode videoModeFromString(string modestring)
{
  Mode mode;
  if (modestring == "4") {
    mode = FlyCapture2::MODE_4;
  } else if (modestring == "7") {
    mode = FlyCapture2::MODE_7;
  } else {
    mode = FlyCapture2::MODE_4;
  }
  return mode;
}

/*
     case PIXEL_FORMAT_S_MONO16: format = COLOR_MONO16; // < 16 bits of signed mono information.  // Invalid
     case PIXEL_FORMAT_S_RGB16: format = COLOR_RGBA8; //< R = G = B = 16 bits signed.  // Invalid
     case PIXEL_FORMAT_MONO12: format = COLOR_RGBA8; //< 12 bits of mono information.   // Invalid
     case PIXEL_FORMAT_RAW12: format = COLOR_RGBA8; //< 12 bit raw data output of sensor.  // Invalid
     case PIXEL_FORMAT_BGR: format = COLOR_RGBA8; //< 24 bit BGR.  // Invalid
     case PIXEL_FORMAT_BGRU: format = COLOR_RGBA8; //< 32 bit BGRU.   // Invalid
     case PIXEL_FORMAT_RGB: format = COLOR_RGBA8; //< 24 bit RGB.  // Invalid
     case PIXEL_FORMAT_RGBU: format = COLOR_RGBA8; //< 32 bit RGBU.  // Invalid
     case PIXEL_FORMAT_BGR16: format = COLOR_RGBA8; //< R = G = B = 16 bits.  // Invalid
     case PIXEL_FORMAT_BGRU16: format = COLOR_RGBA8; //< 64 bit BGRU.  // Invalid
     case PIXEL_FORMAT_422YUV8_JPEG: format = COLOR_RGBA8; //< JPEG compressed stream.  // Invalid

*/

CaptureFlycap::CaptureFlycap(VarList * _settings,int default_camera_id, QObject * parent) : QObject(parent),CaptureInterface(_settings)
{
    mutex.lock();

    settings->addChild(conversion_settings = new VarList("Conversion Settings"));
    settings->addChild(capture_settings = new VarList("Capture Settings"));
    settings->addChild(camera_parameters  = new VarList("Camera Parameters"));

    //=======================CONVERSION SETTINGS=======================
    conversion_settings->addChild(v_colorout=new VarStringEnum(
      "convert to mode",Colors::colorFormatToString(COLOR_YUV422_UYVY)));
    v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));
    v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));

    conversion_settings->addChild(v_debayer=new VarBool("de-bayer",false));
    conversion_settings->addChild(v_debayer_pattern=new VarStringEnum("de-bayer pattern",colorFilterToString(DC1394_COLOR_FILTER_MIN)));
    for (int i = DC1394_COLOR_FILTER_MIN; i <= DC1394_COLOR_FILTER_MAX; i++) {
      v_debayer_pattern->addItem(colorFilterToString((dc1394color_filter_t)i));
    }
    conversion_settings->addChild(v_debayer_method=new VarStringEnum("de-bayer method",bayerMethodToString(DC1394_BAYER_METHOD_MIN)));
    for (int i = DC1394_BAYER_METHOD_MIN; i <= DC1394_BAYER_METHOD_MAX; i++) {
      v_debayer_method->addItem(bayerMethodToString((dc1394bayer_method_t)i));
    }
    conversion_settings->addChild(v_debayer_y16=new VarInt("de-bayer y16 bits",16));
    //dcam_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );

    //=======================CAPTURE SETTINGS==========================
    //TODO: set default camera id
    capture_settings->addChild(v_cam_bus          = new VarInt("Cam Index",-1));
    capture_settings->addChild(v_width            = new VarInt("Width",960));
    capture_settings->addChild(v_height           = new VarInt("Height",600));
    capture_settings->addChild(v_xoffset          = new VarInt("X Offset",0));
    capture_settings->addChild(v_yoffset          = new VarInt("Y Offset",0));
    capture_settings->addChild(v_colormode        = new VarStringEnum("Color Mode",Colors::colorFormatToString(COLOR_YUV422_UYVY)));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_MONO8));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV444));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB8));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_MONO16));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB16));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB8));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_RAW16));

    //v_buffer_size->addFlags(VARTYPE_FLAG_READONLY);

    //camera_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );
    camera_parameters->addChild(v_videomode = new VarStringEnum("Mode","4"));
    v_videomode->addItem("4");
    v_videomode->addItem("7");

    camera_parameters->addChild(new VarDouble("Brightness", 2.93));
    camera_parameters->addChild(new VarDouble("Shutter", 0.6635));
    camera_parameters->addChild(new VarDouble("Gain", 17.998));
    camera_parameters->addChild(new VarDouble("Gamma", 1.250));
    camera_parameters->addChild(new VarDouble("Frame Rate", 60.250));

    camera_parameters->addChild(P_WHITE_BALANCE = new VarList("White Balance"));
    P_WHITE_BALANCE->addChild(new VarBool("Enabled",true));
    P_WHITE_BALANCE->addChild(new VarInt("Red Value",637));
    P_WHITE_BALANCE->addChild(new VarInt("Blue Value",1023));

    mvc_connect(capture_settings);
    mvc_connect(camera_parameters);
    mvc_connect(P_WHITE_BALANCE);

    is_capturing = false;
    is_connected = false;

    currentFrame = NULL;
    camera = NULL;
    stored_image = NULL;
    camera_type = FlyCapCamType::UNKNOWN;
    settings_changed = true;
    previous_id = v_cam_bus->getInt();
    if (previous_id >= 0) {
      cam_id = (unsigned int)v_cam_bus->getInt();
    } else {
      cam_id = 0;
    }

    mutex.unlock();
}

void CaptureFlycap::mvc_connect(VarList * group) {
    vector<VarType *> v=group->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
        connect(v[i],SIGNAL(wasEdited(VarType *)),group,SLOT(mvcEditCompleted()));
    }
    connect(group,SIGNAL(wasEdited(VarType *)),this,SLOT(changed(VarType *)));
}

void CaptureFlycap::changed(VarType * group) {
    if (group->getType()==VARTYPE_ID_LIST) {
      if (group->getName() == "Camera Settings")
        settings_changed = true;
      writeParameterValues( (VarList *)group );
      readParameterValues( (VarList *)group );
    }
}

void CaptureFlycap::readAllParameterValues() {
    vector<VarType *> v=camera_parameters->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
        if (v[i]->getType()==VARTYPE_ID_LIST) {
          readParameterValues((VarList *)v[i]);
        }
    }
}

void CaptureFlycap::writeAllParameterValues() {
    vector<VarType *> v=camera_parameters->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
        if (v[i]->getType()==VARTYPE_ID_LIST) {
          writeParameterValues((VarList *)v[i]);
        }
    }
}

void CaptureFlycap::readParameterValues(VarList * item) {
  mutex.lock();

  mutex.unlock();
}

void CaptureFlycap::writeParameterValues(VarList * item) {
  if (!is_connected) {
    mutex.lock();
  }

  Error error;
  string item_name = item->getName();
  if (item_name == "Camera Parameters") {
    vector<VarType *> children = item->getChildren();
    for (unsigned int i=0;i<children.size();i++) {
      bool is_property=false;
      Property prop;

      string param_name = children[i]->getName();
      if (param_name == "Video Mode") {
        if (is_connected && !is_capturing) {
          Mode mode = videoModeFromString(v_videomode->getString());
	  if (camera_type == FlyCapCamType::GIGE){
            error = ((GigECamera*)camera)->SetGigEImagingMode(mode);
            if (error != PGRERROR_OK) {
              error.PrintErrorTrace();
              mutex.unlock();
              return;
            }
	  }
        }
      } else if (param_name == "Brightness") {
        is_property = true;
        prop.type = FlyCapture2::BRIGHTNESS;
      } else if (param_name == "Shutter") {
        is_property = true;
        prop.type = FlyCapture2::SHUTTER;
      }
      else if (param_name == "Gain") {
        is_property = true;
        prop.type = FlyCapture2::GAIN;
      }
      else if (param_name == "Gamma") {
        is_property = true;
        prop.type = FlyCapture2::GAMMA;
      }
      else if (param_name == "Frame Rate") {
        is_property = true;
        prop.type = FlyCapture2::FRAME_RATE;
      }
      else if (param_name == "White Balance") {
        is_property = true;
        prop.type = FlyCapture2::WHITE_BALANCE;
      }
      if (is_property) {
        prop.onePush = false;
        prop.onOff = true;
        prop.autoManualMode = false;
        prop.absControl = true;

        if (is_connected) {
          if (param_name == "White Balance")  {
            vector<VarType *> grandchildren = children[i]->getChildren();
            VarBool* vOnOff = (VarBool*)grandchildren[0];
            VarInt* vRed = (VarInt*)grandchildren[1];
            VarInt* vBlue = (VarInt*)grandchildren[2];
            prop.onOff = vOnOff->getBool();
            prop.valueA = vRed->getInt();
            prop.valueB = vBlue->getInt();
          }
          else {
            VarDouble* current_double = (VarDouble *)children[i];
            double absvalue = current_double->getDouble();
            prop.absValue = (float)absvalue;
          }

          error = camera->SetProperty(&prop);
          if (error != PGRERROR_OK) {
            mutex.unlock();
            error.PrintErrorTrace();
            return;
          }
        }
      }
    }
  }

  if (settings_changed) {
    if (is_connected && camera_type == FlyCapCamType::GIGE) {
      GigEImageSettings imageSettings;
      imageSettings.offsetX = v_xoffset->getInt();
      imageSettings.offsetY = v_yoffset->getInt();
      imageSettings.height = v_height->getInt();
      imageSettings.width = v_width->getInt();
      imageSettings.pixelFormat = colorFormatToPixelFormat(Colors::stringToColorFormat(v_colormode->getString().c_str()));

      error = ((GigECamera*)camera)->SetGigEImageSettings(&imageSettings);
      if (error != PGRERROR_OK) {
        error.PrintErrorTrace();
        mutex.unlock();
        return;
      }

      settings_changed = false;
    }
  }

  if(!is_connected){
    mutex.unlock();
  }
}

CaptureFlycap::~CaptureFlycap() {
}

bool CaptureFlycap::resetBus() {
  mutex.lock();

  BusManager busMgr;
  busMgr.FireBusReset(&guid);

  mutex.unlock();
  return true;
}

bool CaptureFlycap::stopCapture() {
  cam_id_mutex.lock();
  if (!isCapturing()) {
    cam_id_mutex.unlock();
    return true;
  }

  for (std::vector<unsigned int>::iterator it = active_cam_ids.begin();
       it != active_cam_ids.end();
       ++it) {
    if (*it == cam_id) {
      active_cam_ids.erase(it);
      break;
    }
  }

  is_capturing = false;
  camera->StopCapture();
  camera->Disconnect();
  is_connected = false;

  delete camera;
  cam_id_mutex.unlock();
  return true;
}

bool CaptureFlycap::startCapture() {
  mutex.lock();
  cam_id_mutex.lock();

  BusManager busMgr;
  Error error;
  unsigned int numCameras;

  int current_id = v_cam_bus->getInt();

  if (current_id >= 0) {
    cam_id = (unsigned int)current_id;
  } else {
    fprintf(stderr, "Flycapture: Invalid cam_id: %d\n", current_id);
    mutex.unlock();
    cam_id_mutex.unlock();
    return false;
  }
  error = busMgr.GetNumOfCameras(&numCameras);
  if (error != PGRERROR_OK)
  {
    error.PrintErrorTrace();
    mutex.unlock();
    cam_id_mutex.unlock();
    return false;
  }

  if(cam_id >= numCameras)
  {
    fprintf(stderr, "Flycapture: Invalid cam_id: %u\n", cam_id);

    mutex.unlock();
    cam_id_mutex.unlock();
    return false;
  }

  bool id_active = false;

  for (size_t i = 0; i < active_cam_ids.size(); i++) {
    if (active_cam_ids[i] == cam_id) {
      id_active = true;
      break;
    }
  }

  if (id_active) {
    fprintf(stderr, "Flycapture: cam_id %u is already in use \n", cam_id);

    mutex.unlock();
    cam_id_mutex.unlock();
    return false;
  }

  error = busMgr.GetCameraFromIndex(v_cam_bus->getInt(), &guid);
  if (error != PGRERROR_OK) {
    error.PrintErrorTrace();
    delete camera;

    mutex.unlock();
    cam_id_mutex.unlock();
    return false;
  }

  // First try to connect as if this is a GigE camera
  camera = new GigECamera();

  error = camera->Connect(&guid);
  if (error != PGRERROR_OK) {
    error.PrintErrorTrace();
    delete camera;

    // If we cannot connect as a GigE camera, fall back to using a
    // normal camera
    camera = new Camera();
    error = camera->Connect(&guid);
    if (error != PGRERROR_OK){
      error.PrintErrorTrace();
      delete camera;
      mutex.unlock();
      cam_id_mutex.unlock();
      return false;
    } else {
      camera_type = FlyCapCamType::GENERIC;
    }
  } else {
    camera_type = FlyCapCamType::GIGE;
  }

  fprintf(stderr,"Connected");
  is_connected = true;

  CameraInfo camInfo;
  error = camera->GetCameraInfo(&camInfo);
  if (error != PGRERROR_OK) {
    error.PrintErrorTrace();
    delete camera;
    mutex.unlock();
    cam_id_mutex.unlock();
    return false;
  }

  fprintf(stderr," to %s, Serial Number: %u\n",
          camInfo.modelName,
          camInfo.serialNumber);

  Property auto_exposure;
  auto_exposure.onOff = false;
  auto_exposure.onePush = false;
  auto_exposure.type = FlyCapture2::AUTO_EXPOSURE;
  error = camera->SetProperty(&auto_exposure);
  if (error != PGRERROR_OK) {
    mutex.unlock();
    cam_id_mutex.unlock();
    delete camera;
    error.PrintErrorTrace();
    return false;
  }
  writeAllParameterValues();

  error = camera->StartCapture();
  if (error != PGRERROR_OK) {
    error.PrintErrorTrace();
    delete camera;
    mutex.unlock();
    cam_id_mutex.unlock();
    return false;
  }

  is_capturing = true;

  active_cam_ids.push_back(cam_id);
  mutex.unlock();
  cam_id_mutex.unlock();

  return true;
}

bool CaptureFlycap::copyFrame(const RawImage & src, RawImage & target) {
    return convertFrame(src,target,src.getColorFormat());
}

bool CaptureFlycap::copyAndConvertFrame(const RawImage & src,
                                        RawImage & target) {
  return convertFrame(src,target,
                      Colors::stringToColorFormat(v_colorout->getSelection().c_str()),
                      v_debayer->getBool(),
                      stringToColorFilter(v_debayer_pattern->getSelection().c_str()),
                      stringToBayerMethod(v_debayer_method->getSelection().c_str()),
                      v_debayer_y16->getInt());
}

bool CaptureFlycap::convertFrame(const RawImage & src,
                           RawImage & target,
                           ColorFormat output_fmt,
                           bool debayer,
                           dc1394color_filter_t bayer_format,
                           dc1394bayer_method_t bayer_method,
                           int y16bits) {
  mutex.lock();

  int width = v_width->getInt();
  int height = v_height->getInt();

  ColorFormat src_fmt=src.getColorFormat();
  if (target.getData()==0) {
    //allocate target, if it does not exist yet
    target.allocate(output_fmt,src.getWidth(),src.getHeight());
  } else {
    target.ensure_allocation(output_fmt,src.getWidth(),src.getHeight());
  }
  target.setTime(src.getTime());
  target.setTimeCam ( src.getTimeCam() );
  if (output_fmt==src_fmt) {
    //just do a memcpy
    memcpy(target.getData(),src.getData(),src.getNumBytes());
  } else {
    //do some more fancy conversion
    if ((src_fmt==COLOR_MONO8 || src_fmt==COLOR_RAW8) && output_fmt==COLOR_RGB8) {
      //check whether to debayer or simply average to a grey rgb image
      if (debayer) {
        //de-bayer
        if ( dc1394_bayer_decoding_8bit( src.getData(), target.getData(), src.getWidth(), src.getHeight(), bayer_format, bayer_method) != DC1394_SUCCESS ) {
            mutex.unlock();
          return false;
        }
      } else {
        dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, 0,
                       DC1394_COLOR_CODING_MONO8, 8);
        //Conversions::y2rgb (src.getData(), target.getData(), src.getNumPixels());
      }
    } else if ((src_fmt==COLOR_MONO16 || src_fmt==COLOR_RAW16)) {
      //check whether to debayer or simply average to a grey rgb image
      if (debayer && output_fmt==COLOR_RGB16) {
        //de-bayer
        if ( dc1394_bayer_decoding_16bit( (uint16_t *)src.getData(), (uint16_t *)target.getData(), src.getWidth(), src.getHeight(), bayer_format, bayer_method, y16bits) != DC1394_SUCCESS ) {
          fprintf(stderr,"Error in 16bit Bayer Conversion");

            mutex.unlock();
          return false;
        }
      } else if (debayer==false && output_fmt==COLOR_RGB8) {

       dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, 0,
                       ((src_fmt==COLOR_MONO16) ? DC1394_COLOR_CODING_MONO16 : DC1394_COLOR_CODING_RAW16), y16bits);
        //Conversions::y162rgb (src.getData(), target.getData(), src.getNumPixels(), y16bits);
      } else {
      fprintf(stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",Colors::colorFormatToString(src_fmt).c_str(),Colors::colorFormatToString(output_fmt).c_str());
      mutex.unlock();
      return false;
      }
    } else if (src_fmt==COLOR_YUV411 && output_fmt==COLOR_RGB8) {
      dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, 0,
                       DC1394_COLOR_CODING_YUV411, 8);
      //Conversions::uyyvyy2rgb (src.getData(), target.getData(), src.getNumPixels());
    } else if (src_fmt==COLOR_YUV422_UYVY && output_fmt==COLOR_RGB8) {
        dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, DC1394_BYTE_ORDER_UYVY,
                       DC1394_COLOR_CODING_YUV422, 8);
    } else if (src_fmt==COLOR_YUV422_YUYV && output_fmt==COLOR_RGB8) {
        dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, DC1394_BYTE_ORDER_YUYV,
                       DC1394_COLOR_CODING_YUV422, 8);
    } else if (src_fmt==COLOR_YUV444 && output_fmt==COLOR_RGB8) {
      dc1394_convert_to_RGB8(src.getData(),target.getData(), width, height, 0,
                       DC1394_COLOR_CODING_YUV444, 8);
    } else {
      fprintf(stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",Colors::colorFormatToString(src_fmt).c_str(),Colors::colorFormatToString(output_fmt).c_str());
      mutex.unlock();
      return false;
    }
  }

  mutex.unlock();
  return true;
}

RawImage CaptureFlycap::getFrame() {
  mutex.lock();
  Error error;
  Image image;
  RawImage result;
  result.setColorFormat(capture_format);
  error = camera->RetrieveBuffer(&image);
  if (error != PGRERROR_OK) {
    error.PrintErrorTrace();
    mutex.unlock();
    return result;
  }

  stored_image = new Image;
  stored_image->DeepCopy(&image);

  result.setData(stored_image->GetData());
  result.setHeight(stored_image->GetRows());
  result.setWidth(stored_image->GetCols());
  result.setColorFormat(pixelFormatToColorFormat(stored_image->GetPixelFormat()));

  mutex.unlock();
  return result;
}

void CaptureFlycap::releaseFrame() {
  mutex.lock();
  delete stored_image;
  mutex.unlock();
}

string CaptureFlycap::getCaptureMethodName() const {
    return "Flycap";
}


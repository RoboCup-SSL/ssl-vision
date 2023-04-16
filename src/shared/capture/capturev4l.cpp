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
// (C) 2016 Eric Zavesky, Emoters
// Licenced under the GNU General Public License (GPL) version 3,
//   or alternately by a specific written agreement.
//========================================================================
/*!
 \file    capturev4l.cpp
 \brief   C++ Interface: CaptureV4L
 \author  Eric Zavesky, (C) 2016 (derived from DC1394 and CMVision)
 */
//========================================================================

//==================================================================
// A note about the capture format.... (from Eric)
// - There appears to be a different method of byte packing in the V4L
//   library versus standard YUYV and UYUV packing used in ssl-vision
//   http://linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-YUYV.html
// - Additional some, very basic cameras only support YUYV and not UYVY,
//   which is the expected 4:2:2 format within ssl-vision for the YUV lookup.
// - Thus, we make the extra hop into the common RGB space immediately
//   at capture instead of deferring this to a convsersion later.
//   Committs earlier in the life of this code indicate other attempts,
//   which may be helpful to future developers.
// - One ramification is a slower copy/decode process.  On a desktop or
//   laptop, this doesn't seem to influce frame rate.
// - This is not ideal, but the problem is left to future developers
//   to ferret out proper settings for the video format (see v4l2_format)
//   and colorspaces within ssl-vision and V4L.  Until then, it works.
//==================================================================


#include "capturev4l.h"
#include "conversions.h"

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <cerrno>
#include <fcntl.h>      //open
#include <unistd.h>     //close

namespace {
    uint32_t stringToPixelFormat(const std::string &format) {
      if (format == "YUYV") {
        return V4L2_PIX_FMT_YUYV;
      } else if (format == "MJPEG") {
        return V4L2_PIX_FMT_MJPEG;
      } else {
        return V4L2_PIX_FMT_YUYV;
      }
    }

    std::string pixelFormatToString(const uint32_t format) {
      switch (format) {
        case V4L2_PIX_FMT_YUYV:
          return "YUYV";
        case V4L2_PIX_FMT_MJPEG:
          return "MJPEG";
        default:
          return "UNKNOWN";
      }
    }
} // namespace

//======================= Singleton Manager =======================

GlobalV4LinstanceManager *GlobalV4LinstanceManager::pinstance = nullptr;

GlobalV4Linstance *GlobalV4LinstanceManager::obtainInstance(int iDevice) {
  if (!pinstance) pinstance = new GlobalV4LinstanceManager();
  if (pinstance->map_instance.find(iDevice) == pinstance->map_instance.end()) {
    pinstance->map_instance[iDevice] = new GlobalV4Linstance();
  }
  if (!pinstance->map_instance[iDevice]->obtainInstance(iDevice)) return nullptr;
  return pinstance->map_instance[iDevice];
}

bool GlobalV4LinstanceManager::removeInstance(GlobalV4Linstance *pDevice) {
  for (auto it = pinstance->map_instance.begin();
       it != pinstance->map_instance.end(); it++) {
    if (it->second == pDevice) {
      if (pDevice->removeInstance()) {
        delete pDevice;
      }
      pinstance->map_instance.erase(it);
      return true;
    }
  }
  return false;
}

GlobalV4LinstanceManager::~GlobalV4LinstanceManager() {
  for (auto & it : pinstance->map_instance) {
    delete it.second;
  }
  map_instance.clear();
}

//======================= Actual V4L device interface =======================

bool GlobalV4Linstance::obtainInstance(int iDevice) {
  char szDeviceLocal[128];
  snprintf(szDeviceLocal, 128, "/dev/video%d", iDevice);
  return obtainInstance(szDeviceLocal);
}

bool GlobalV4Linstance::obtainInstance(char *szDevice_) {
  lock();
  bool bSuccess = (pollset.fd > -1);

  if (counter == 0) {
    if (pollset.fd < 0) {
      counter = 1;
      removeInstance(false);
    }

    pollset.fd = open(szDevice_, O_RDWR | O_NONBLOCK);
    if (pollset.fd > -1) bSuccess = true;

    if (bSuccess) {
      pollset.events = POLLIN;

      // test capture capabilities
      v4l2_capability cap{};
      if (xioctl(pollset.fd, VIDIOC_QUERYCAP, &cap, "VideoQueryCap") == 0) {
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
            !(cap.capabilities & V4L2_CAP_STREAMING)) {
          bSuccess = false;
        }
      } else {
        bSuccess = false;
      }
    }

    if (!bSuccess) {
      if (pollset.fd > -1) close(pollset.fd);
      pollset.fd = -1;
    } else {
      strcpy(szDevice, szDevice_);
    }
  }
  if (bSuccess)
    counter++;

  unlock();
  return bSuccess;
}

bool GlobalV4Linstance::removeInstance(bool bRelock) {
  if (bRelock) lock();
  counter--;
  bool was_last = false;
  if (counter == 0) {
    if (pollset.fd >= 0) {
      stopStreaming();
      close(pollset.fd);
    }
    pollset.fd = -1;
    memset(szDevice, 0, sizeof(char) * 128);
    was_last = true;
  }
  if (counter < 0) {
    fprintf(stderr, "WARNING: Attempting to remove more instances than have been created!\n");
    fflush(stderr);
    counter = 0;
  }
  if (bRelock) unlock();
  return was_last;
}

bool GlobalV4Linstance::xioctl(int request, void *data, const char *error_str) const {
  int ret = xioctl(pollset.fd, request, data, error_str);
  return (ret == 0);
}

int GlobalV4Linstance::xioctl(int fd, int request, void *data,
                              const char *error_str) {
  // try the ioctl, which should succeed in the common case
  int ret = ioctl(fd, request, data);
  if (ret >= 0) return (ret);

  // retry if we were interrupted (up to a few times)
  int n = 0;
  while (ret != 0 && errno == EINTR && n < 8) {
    ret = ioctl(fd, request, data);
    n++;
  }

  // report error
  if (ret != 0 && error_str) {
    fprintf(stderr, "GlobalV4Linstance: %s returned %d (%s)\n",
            error_str, ret, strerror(errno));
  }

  return (ret);
}

bool GlobalV4Linstance::captureFrame(RawImage *pImage, uint32_t pixel_format, int iMaxSpin) {
  const image_t *_img = captureFrame(iMaxSpin);       //low-level fetch
  if (!_img || _img->data == nullptr) return false;
  bool bSuccess = false;

  if (_img) {
    if (pImage) {                                       //allow null to stoke the capture
      lock();

      //mid-level copy to RGB
      if (_img->data && getImage(*_img, pixel_format, pImage)) {
        long time_cam = _img->timestamp.tv_sec * (long) 1e9 +
                        _img->timestamp.tv_usec * (long) 1e3;
        pImage->setTimeCam(time_cam);
        bSuccess = true;
      }
      unlock();
    }
    if (!releaseFrame(_img))                            //maybe we shouldn't return an error?
      return false;
  }

  return bSuccess;
}

const GlobalV4Linstance::image_t *GlobalV4Linstance::captureFrame(int iMaxSpin) {
  if (!waitForFrame(300)) {
    fprintf(stderr, "GlobalV4Linstance: error waiting for frame '%s'\n", szDevice);
    return nullptr;
  }

  do {
    // get the frame
    dequeueBuffer(tempbuf);

    // poll to see if a another frame is already available
    // if so, break out now
    if (!waitForFrame(0)) break;

    // otherwise, drop this frame
    enqueueBuffer(tempbuf);
  } while (iMaxSpin--);

  uint32_t i = tempbuf.index;
  img[i].timestamp = tempbuf.timestamp;
  img[i].field = (tempbuf.field == V4L2_FIELD_BOTTOM);
  return (&(img[i]));
}

bool GlobalV4Linstance::releaseFrame(const GlobalV4Linstance::image_t *_img) {
  if (!_img) return (false);
  tempbuf.index = _img->index;
  return (enqueueBuffer(tempbuf));
}

//because there is some delay before camera actually starts, spin
// here for the first frame to come out
void GlobalV4Linstance::captureWarm(int iMaxSpin) {
  const GlobalV4Linstance::image_t *_img = nullptr;
  while (!_img) {
    _img = captureFrame(iMaxSpin);
  }
  if (_img->data)
    releaseFrame(_img);
}

bool GlobalV4Linstance::waitForFrame(int max_msec) {
  if (pollset.fd == -1) return false;
  int n = poll(&pollset, 1, max_msec);
  return (n == 1 && (pollset.revents & POLLIN) != 0);
}

bool GlobalV4Linstance::startStreaming(int iWidth_, int iHeight_, uint32_t pixel_format, int framerate, int iInputIdx) {

  // Set video format
  v4l2_format fmt{};
  fmt.fmt.pix.width = iWidth_;
  fmt.fmt.pix.height = iHeight_;
  fmt.fmt.pix.pixelformat = pixel_format;
  fmt.fmt.pix.field = V4L2_FIELD_ALTERNATE;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (!xioctl(VIDIOC_S_FMT, &fmt, "SetFormat")) {
    printf("Warning: Could not set format, '%s'; was device previously started?\n", szDevice);
  }

  // Set Input and Controls (for more advanced v4l as well)
  // http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-input.html
  // vid.setInput(0); // main capture (cable or camera video)
  // vid.setInput(1); // Component video
  // vid.setInput(2); // S-video
  xioctl(VIDIOC_S_INPUT, &iInputIdx, "SetInput");

  // Request mmap-able capture buffers
  struct v4l2_requestbuffers req{};
  req.count = V4L_STREAMBUFS;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (!xioctl(VIDIOC_REQBUFS, &req, "Request Buffers") || req.count != V4L_STREAMBUFS) {
    printf("REQBUFS returned error, count %d\n", req.count);
    return (false);
  }

  struct v4l2_streamparm parm{};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.timeperframe.numerator = 1;
  parm.parm.capture.timeperframe.denominator = framerate;
  if (!xioctl(VIDIOC_S_PARM, &parm, "SetParm")) {
    printf("Warning: Could not set parameters, '%s'; was device previously started?\n", szDevice);
  }

  // set up individual buffers
  mzero(img, V4L_STREAMBUFS);
  mzero(tempbuf);
  tempbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  tempbuf.memory = V4L2_MEMORY_MMAP;

  for (int i = 0; i < (int) req.count; i++) {
    tempbuf.index = i;
    if (!xioctl(VIDIOC_QUERYBUF, &tempbuf, "Allocate query buffer")) {
      printf("QUERYBUF returned error, '%s'\n", szDevice);
      return (false);
    }
    img[i].index = i;
    img[i].length = tempbuf.length;
    img[i].data = static_cast<unsigned char *>(
            mmap(nullptr, tempbuf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                 pollset.fd, tempbuf.m.offset));

    if (img[i].data == MAP_FAILED) {
      printf("mmap() returned error %d (%s)\n", errno, strerror(errno));
      return (false);
    }
  }

  //enqueue buffer that we just memmapped/allocated
  for (unsigned i = 0; i < req.count; i++) {
    tempbuf.index = i;
    if (!enqueueBuffer(tempbuf)) {
      printf("Error queueing initial buffers, '%s'\n", szDevice);
      return (false);
    }
  }

  //start actual stream
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  return xioctl(VIDIOC_STREAMON, &type, "StreamOn");
}

bool GlobalV4Linstance::stopStreaming() {
  bool bSuccess = false;
  if (pollset.fd != -1) {
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bSuccess = xioctl(VIDIOC_STREAMOFF, &type, nullptr);

    for (auto & i : img) {
      if (i.data) {
        munmap(i.data, i.length);
        i.data = nullptr;
        i.length = 0;
      }
    }
  }

  //don't close the stream just yet

  return bSuccess;
}

bool GlobalV4Linstance::enqueueBuffer(v4l2_buffer &buf) {
  return xioctl(VIDIOC_QBUF, &buf, "EnqueueBuffer");
}

bool GlobalV4Linstance::dequeueBuffer(v4l2_buffer &buf) {
  return xioctl(VIDIOC_DQBUF, &buf, "DequeueBuffer");
}

bool GlobalV4Linstance::checkControl(long ctrl_id, bool *bEnabled, bool *bReadOnly,
                                     long *lDefault, long *lMin, long *lMax) {
  v4l2_queryctrl queryctrl{};
  queryctrl.id = ctrl_id;

  if (xioctl(VIDIOC_QUERYCTRL, &queryctrl, "CheckControl")) {
    if (bEnabled) (*bEnabled) = !(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED);
    if (bReadOnly) (*bReadOnly) = queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY;
    if (lMin) (*lMin) = queryctrl.minimum;
    if (lMax) (*lMax) = queryctrl.maximum;
    if (lDefault) (*lDefault) = queryctrl.default_value;
    return true;
  }
  return false;
}


bool GlobalV4Linstance::getControl(long ctrl_id, long &s) {
  v4l2_control ctrl{};
  ctrl.id = ctrl_id;

  if (xioctl(VIDIOC_G_CTRL, &ctrl, "GetControl")) {
    s = ctrl.value;
    return (true);
  } else {
    return (false);
  }
}

bool GlobalV4Linstance::setControl(long ctrl_id, long s) {
  v4l2_control ctrl{};
  ctrl.id = ctrl_id;
  ctrl.value = (int) s;

  return xioctl(VIDIOC_S_CTRL, &ctrl, "SetControl");
}

//======================= Low level utility functions ====================
bool GlobalV4Linstance::writeYuyvPPM(GlobalV4Linstance::yuyv *pSrc, int width, int height, const char *filename) {
  GlobalV4Linstance::rgb *bufrgb = nullptr;
  if (!getImageRgb(pSrc, width, height, &bufrgb)) return false;
  int wrote = writeRgbPPM(bufrgb, width, height, filename);
  delete[](bufrgb);

  return (wrote > 0);
}

bool GlobalV4Linstance::writeRgbPPM(GlobalV4Linstance::rgb *imgbuf, int width, int height, const char *filename) {
  // open output file
  FILE *out = fopen(filename, "wb");
  if (!out) return (false);

  // write the image
  fprintf(out, "P6\n%d %d\n%d\n", width, height, 255);
  fwrite(imgbuf, 3, width * height, out);

  return (fclose(out) == 0);
}

bool GlobalV4Linstance::getImageRgb(GlobalV4Linstance::yuyv *pSrc, int width, int height, GlobalV4Linstance::rgb **rgbbuf) {
  if (!rgbbuf) return false;
  if ((*rgbbuf) == nullptr)
    (*rgbbuf) = new rgb[width * height];

  int size = width * height;
  GlobalV4Linstance::rgb *pDest = (*rgbbuf);

  // Convert YUYV to RGB
  for (int iP = 0; iP < size >> 1; iP++) {
    int uv2r = ((pSrc->v * 1436) >> 10) - 179;
    int uv2g = ((pSrc->u * 352 + pSrc->v * 731) >> 10) - 135;
    int uv2b = ((pSrc->u * 1814) >> 10) - 226;
    pDest->red = bound(pSrc->y1 + uv2r, 0, 255);
    pDest->green = bound(pSrc->y1 - uv2g, 0, 255);
    pDest->blue = bound(pSrc->y1 + uv2b, 0, 255);
    pDest++;
    pDest->red = bound(pSrc->y2 + uv2r, 0, 255);
    pDest->green = bound(pSrc->y2 - uv2g, 0, 255);
    pDest->blue = bound(pSrc->y2 + uv2b, 0, 255);
    pDest++;
    pSrc++;
  }
  return true;
}

void GlobalV4Linstance::jpegErrorExit(j_common_ptr dinfo) {
  char jpegLastErrorMsg[JMSG_LENGTH_MAX];
  /* Create the message */
  (*(dinfo->err->format_message))(dinfo, jpegLastErrorMsg);

  /* Jump to the catch */
  throw std::runtime_error(jpegLastErrorMsg);
}

bool GlobalV4Linstance::getImageFromJPEG(
        const GlobalV4Linstance::image_t &in_img, RawImage *out_img) {
  struct jpeg_decompress_struct dinfo{};
  struct jpeg_error_mgr jerr{};
  dinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = jpegErrorExit;

  try {
    jpeg_create_decompress(&dinfo);
    jpeg_mem_src(&dinfo, in_img.data, in_img.length);
    if (jpeg_read_header(&dinfo, true) != JPEG_HEADER_OK) {
      std::cout << "Warning: bad jpeg header" << std::endl;
      return false;
    }
    dinfo.output_components = 3;
    jpeg_start_decompress(&dinfo);

    int row_stride = out_img->getWidth() * 3;
    for (int row = 0; row < out_img->getHeight(); ++row) {
      JSAMPROW row_ptr = &out_img->getData()[row_stride * row];
      jpeg_read_scanlines(&dinfo, &row_ptr, 1);
    }
    // freeing jpeg_decompress_struct memory
    jpeg_destroy_decompress(&dinfo);
  }
  catch (std::runtime_error &e) {
    jpeg_destroy_decompress(&dinfo);
    return false;
  }
  return true;
}

bool GlobalV4Linstance::getImage(const GlobalV4Linstance::image_t &in_img,
                                 const uint32_t pixel_format,
                                 RawImage *out_img) {
  switch (pixel_format) {
    case V4L2_PIX_FMT_YUYV: {
      unsigned char *pDest = out_img->getData();
      return getImageRgb(reinterpret_cast<GlobalV4Linstance::yuyv *>(in_img.data),
                         out_img->getWidth(), out_img->getHeight(),
                         reinterpret_cast<GlobalV4Linstance::rgb **>(&pDest));
    }
    case V4L2_PIX_FMT_MJPEG:
      return getImageFromJPEG(in_img, out_img);
    default:
      return false;
  }
}

inline GlobalV4Linstance::rgb GlobalV4Linstance::yuv2rgb(GlobalV4Linstance::yuv p) {
  GlobalV4Linstance::rgb r{};

  r.red = bound(p.y + ((p.v * 1436) >> 10) - 179, 0, 255);
  r.green = bound(p.y - ((p.u * 352 + p.v * 731) >> 10) + 135, 0, 255);
  r.blue = bound(p.y + ((p.u * 1814) >> 10) - 226, 0, 255);

  return r;
}



//======================= GUI & API Definitions =======================

CaptureV4L::CaptureV4L(VarList *_settings, int default_camera_id, QObject *parent) : QObject(parent), CaptureInterface(_settings) {
  mutex.lock();

  settings->addChild(conversion_settings = new VarList("Conversion Settings"));
  settings->addChild(capture_settings = new VarList("Capture Settings"));
  settings->addChild(dcam_parameters = new VarList("Camera Parameters"));

  //=======================CONVERSION SETTINGS=======================
  conversion_settings->addChild(v_colorout = new VarStringEnum("convert to mode", Colors::colorFormatToString(COLOR_RGB8)));
  v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));

  dcam_parameters->addFlags(VARTYPE_FLAG_HIDE_CHILDREN);

  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild(v_cam_bus = new VarInt("cam idx", default_camera_id));
  capture_settings->addChild(v_fps = new VarInt("framerate", 30));
  capture_settings->addChild(v_width = new VarInt("width", 640));
  capture_settings->addChild(v_height = new VarInt("height", 480));
  capture_settings->addChild(v_left = new VarInt("left", 0));
  capture_settings->addChild(v_top = new VarInt("top", 0));
  capture_settings->addChild(v_colormode = new VarStringEnum("capture mode", Colors::colorFormatToString(COLOR_RGB8)));
  v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB8));

  v_format.reset(new VarStringEnum("pixel format", pixelFormatToString(V4L2_PIX_FMT_YUYV)));
  v_format->addItem(pixelFormatToString(V4L2_PIX_FMT_YUYV));
  v_format->addItem(pixelFormatToString(V4L2_PIX_FMT_MJPEG));
  capture_settings->addChild(v_format.get());
  capture_settings->addChild(v_buffer_size = new VarInt("ringbuffer size", V4L_STREAMBUFS));
  v_buffer_size->addFlags(VARTYPE_FLAG_READONLY);

  //=======================DCAM PARAMETERS===========================
  // http://linuxtv.org/downloads/legacy/video4linux/API/V4L2_API/spec/ch01s08.html
  dcam_parameters->addChild(P_BRIGHTNESS = new VarList("brightness"));
  P_BRIGHTNESS->addChild(new VarBool("enabled"));
  P_BRIGHTNESS->addChild(new VarBool("default"));
  P_BRIGHTNESS->addChild(new VarInt("value"));
  mvc_connect(P_BRIGHTNESS);

  dcam_parameters->addChild(P_SHARPNESS = new VarList("sharpness"));
  P_SHARPNESS->addChild(new VarBool("enabled"));
  P_SHARPNESS->addChild(new VarInt("defulat"));
  P_SHARPNESS->addChild(new VarInt("value"));
  mvc_connect(P_SHARPNESS);

  dcam_parameters->addChild(P_WHITE_BALANCE = new VarList("white balance"));
  P_WHITE_BALANCE->addChild(new VarBool("enabled"));
  P_WHITE_BALANCE->addChild(new VarBool("auto"));
  mvc_connect(P_WHITE_BALANCE);

  dcam_parameters->addChild(P_HUE = new VarList("hue"));
  P_HUE->addChild(new VarBool("enabled"));
  P_HUE->addChild(new VarBool("default"));
  P_HUE->addChild(new VarInt("value"));
  mvc_connect(P_HUE);

  dcam_parameters->addChild(P_SATURATION = new VarList("saturation"));
  P_SATURATION->addChild(new VarBool("enabled"));
  P_SATURATION->addChild(new VarBool("default"));
  P_SATURATION->addChild(new VarInt("value"));
  mvc_connect(P_SATURATION);

  dcam_parameters->addChild(P_GAMMA = new VarList("gamma"));
  P_GAMMA->addChild(new VarBool("enabled"));
  P_GAMMA->addChild(new VarBool("default"));
  P_GAMMA->addChild(new VarInt("value"));
  mvc_connect(P_GAMMA);

  dcam_parameters->addChild(P_EXPOSURE = new VarList("shutter/exposure"));
  P_EXPOSURE->addChild(new VarBool("enabled"));
  P_EXPOSURE->addChild(new VarBool("default"));
  P_EXPOSURE->addChild(new VarInt("value"));
  mvc_connect(P_EXPOSURE);

  dcam_parameters->addChild(P_CONTRAST = new VarList("contrast"));
  P_CONTRAST->addChild(new VarBool("enabled"));
  P_CONTRAST->addChild(new VarBool("default"));
  P_CONTRAST->addChild(new VarInt("value"));
  mvc_connect(P_CONTRAST);

  dcam_parameters->addChild(P_TEMPERATURE = new VarList("temperature"));
  P_TEMPERATURE->addChild(new VarBool("enabled"));
  P_TEMPERATURE->addChild(new VarBool("default"));
  P_TEMPERATURE->addChild(new VarInt("value"));
  mvc_connect(P_TEMPERATURE);

  dcam_parameters->addChild(P_GAIN = new VarList("gain"));
  P_GAIN->addChild(new VarBool("enabled"));
  P_GAIN->addChild(new VarBool("default"));
  P_GAIN->addChild(new VarInt("value"));
  mvc_connect(P_GAIN);

  dcam_parameters->addChild(P_FRAME_RATE = new VarList("frame rate"));
  P_FRAME_RATE->addChild(new VarBool("enabled"));
  P_FRAME_RATE->addChild(new VarBool("auto"));
  P_FRAME_RATE->addChild(new VarInt("value"));
  mvc_connect(P_FRAME_RATE);

  vector<VarType *> v = dcam_parameters->getChildren();
  for (auto & i : v) {
    if (i->getType() == VARTYPE_ID_LIST) {
      VarBool *temp;
      ((VarList *) i)->addChild(temp = new VarBool("was_read", false));
      temp->addFlags(VARTYPE_FLAG_HIDDEN);
    }
  }

  mvc_connect(P_FRAME_RATE);

  mutex.unlock();
}

void CaptureV4L::mvc_connect(VarList *group) {
  vector<VarType *> v = group->getChildren();
  for (auto & i : v) {
    connect(i, SIGNAL(wasEdited(VarType * )), group, SLOT(mvcEditCompleted()));
  }
  connect(group, SIGNAL(wasEdited(VarType * )), this, SLOT(changed(VarType * )));
}

void CaptureV4L::changed(VarType *group) {
  if (group->getType() == VARTYPE_ID_LIST) {
    writeParameterValues((VarList *) group);
    readParameterValues((VarList *) group);
  }
}


void CaptureV4L::readAllParameterValues() {
  vector<VarType *> v = dcam_parameters->getChildren();
  for (auto & i : v) {
    if (i->getType() == VARTYPE_ID_LIST) {
      readParameterValues((VarList *) i);
    }
  }
}

void CaptureV4L::readAllParameterProperties() {
  vector<VarType *> v = dcam_parameters->getChildren();
  for (auto & i : v) {
    if (i->getType() == VARTYPE_ID_LIST) {
      readParameterProperty((VarList *) i);
    }
  }
}

void CaptureV4L::writeAllParameterValues() {
  vector<VarType *> v = dcam_parameters->getChildren();
  for (auto & i : v) {
    if (i->getType() == VARTYPE_ID_LIST) {
      writeParameterValues((VarList *) i);
    }
  }
}

void CaptureV4L::readParameterValues(VarList *item) {
  if (!camera_instance) return;
  mutex.lock();
  bool valid = true;
  v4lfeature_t feature = getV4LfeatureEnum(item, valid);
  if (!valid) {
    printf("INVALID FEATURE: %s\n", item->getName().c_str());
    mutex.unlock();
    return;
  }
  VarInt *vint = nullptr;
  VarBool *venabled = nullptr;
  VarBool *vwasread = nullptr;

  vector<VarType *> children = item->getChildren();
  for (auto & i : children) {
    if (i->getType() == VARTYPE_ID_BOOL && i->getName() == "was_read") vwasread = (VarBool *) i;
    if (i->getType() == VARTYPE_ID_BOOL && i->getName() == "enabled") venabled = (VarBool *) i;
    if (i->getType() == VARTYPE_ID_INT && i->getName() == "value") vint = (VarInt *) i;
  }

  long lValue;
  if (vwasread != nullptr && vwasread->getBool()) {             //only proceed if no failure in past
    if (feature > GlobalV4Linstance::V4L2_FEATURE_PRIVATE) {       //custom/private features
      printf("UNIMPLEMENTED FEATURE (readParameterValues): %s\n", item->getName().c_str());
    } else {
      if (!camera_instance->getControl(feature, lValue)) {
        //feature doesn't exist
        printf("NON-READ FEATURE (readParameterValues): %s\n", item->getName().c_str());
        if (vwasread != nullptr) vwasread->setBool(false);
      } else {
        //check for switchability:
        if (vwasread != nullptr) vwasread->setBool(true);
        if (camera_instance->checkControl(feature)) {
          venabled->setBool(true);
        } else {
          venabled->setBool(false);
        }
        vint->setInt((int) lValue);
      }

      //update render flags:
      if (vint) {
        if (venabled->getBool()) {
          vint->removeFlags(VARTYPE_FLAG_READONLY);
        } else {
          vint->addFlags(VARTYPE_FLAG_READONLY);
        }
      }
    }
  }

  mutex.unlock();
}

void CaptureV4L::writeParameterValues(VarList *item) {
  mutex.lock();
  bool valid = true;
  v4lfeature_t feature = getV4LfeatureEnum(item, valid);
  if (!valid) {
    mutex.unlock();
    return;
  }
  VarInt *vint = nullptr;
  VarBool *venabled = nullptr;
  VarBool *vwasread = nullptr;
  VarBool *vdefault = nullptr;

  vector<VarType *> children = item->getChildren();
  for (auto & i : children) {
    if (i->getType() == VARTYPE_ID_BOOL && i->getName() == "was_read") vwasread = (VarBool *) i;
    if (i->getType() == VARTYPE_ID_BOOL && i->getName() == "enabled") venabled = (VarBool *) i;
    if (i->getType() == VARTYPE_ID_BOOL && i->getName() == "default") vdefault = (VarBool *) i;
    if (i->getType() == VARTYPE_ID_INT && i->getName() == "value") vint = (VarInt *) i;
  }

  //new: only apply parameters which were previously read from the camera
  if (feature > GlobalV4Linstance::V4L2_FEATURE_PRIVATE) {       //custom/private features
    printf("UNIMPLEMENTED FEATURE (writeParameterValues): %s\n", item->getName().c_str());
  } else {
    if (vwasread != nullptr && vwasread->getBool()) {
      printf("ATTEMPTING TO SET (writeParameterValues): %s\n", item->getName().c_str());
      if (vint != nullptr) {
        if (vdefault && vdefault->getBool())
          vint->resetToDefault();
        long lValue = static_cast<long>(vint->getInt());
        if (!camera_instance->setControl(feature, lValue)) {
          //feature doesn't exist or broke after initial read
          if (vwasread != nullptr) vwasread->setBool(false);
          venabled->setBool(false);
        }
      }
    }
  }
  mutex.unlock();
}


void CaptureV4L::readParameterProperty(VarList *item) {
  if (!camera_instance) return;
  mutex.lock();
  bool valid = true;
  v4lfeature_t feature = getV4LfeatureEnum(item, valid);
  if (!valid) {
    fprintf(stderr, "ERROR: INVALID PROPERTY ENCOUNTERED DURING READOUT: %s\n", item->getName().c_str());
    mutex.unlock();
    return;
  }
  VarInt *vint = nullptr;
  VarBool *venabled = nullptr;

  vector<VarType *> children = item->getChildren();
  for (auto & i : children) {
    if (i->getType() == VARTYPE_ID_BOOL && i->getName() == "enabled") venabled = (VarBool *) i;
    if (i->getType() == VARTYPE_ID_INT && i->getName() == "value") vint = (VarInt *) i;
  }

  long lDefault = 0;
  if (feature > GlobalV4Linstance::V4L2_FEATURE_PRIVATE) {       //custom/private features
    printf("UNIMPLEMENTED FEATURE (readParameterProperty): %s\n", item->getName().c_str());
  } else {
    if (!camera_instance->getControl(feature, lDefault)) {
#ifdef NDEBUG
      fprintf(stderr, "V4L PROP FEATURE IS *NOT* PRESENT: %s\n", item->getName().c_str());
#endif
      //feature doesn't exist
      item->addFlags(VARTYPE_FLAG_READONLY | VARTYPE_FLAG_HIDE_CHILDREN);
    } else {
#ifdef NDEBUG
      fprintf(stderr, "V4L PROP FEATURE IS PRESENT: %s\n", item->getName().c_str());
#endif
      item->removeFlags(VARTYPE_FLAG_READONLY | VARTYPE_FLAG_HIDE_CHILDREN);
      //check for switchability:
      bool bReadOnly, bEnabled;
      long lMin, lMax;
      if (camera_instance->checkControl(feature, &bEnabled, &bReadOnly, &lDefault, &lMin, &lMax)) {
        venabled->setBool(bEnabled);
        if (bReadOnly)
          venabled->removeFlags(VARTYPE_FLAG_READONLY);
        else
          venabled->addFlags(VARTYPE_FLAG_READONLY);
        if (vint != nullptr) {
          vint->setMin((int) lMin);
          vint->setMax((int) lMax);
          vint->setDefault((int) lDefault);
        }
      } else {
        venabled->setBool(false);
        venabled->addFlags(VARTYPE_FLAG_READONLY);
      }
    }
  }
  mutex.unlock();
}

bool CaptureV4L::stopCapture() {
  if (isCapturing()) {
    is_capturing = false;
    readAllParameterValues();

    if (!camera_instance->stopStreaming()) {
      printf("CaptureV4L Error: Could not stop streaming");
    }
    GlobalV4LinstanceManager::removeInstance(camera_instance);
    camera_instance = nullptr;
    rawFrame.clear();
  }

  vector<VarType *> tmp = capture_settings->getChildren();
  for (auto & i : tmp) {
    i->removeFlags(VARTYPE_FLAG_READONLY);
  }
  dcam_parameters->addFlags(VARTYPE_FLAG_HIDE_CHILDREN);

  return true;
}

/// This function converts a local dcam_parameters-manager variable ID
/// to a v4l feature enum
// http://v4l-test.sourceforge.net/spec/x542.htm
v4lfeature_t CaptureV4L::getV4LfeatureEnum(VarList *val, bool &valid) {
  v4lfeature_t res = V4L2_CID_BASE;
  if (val == P_BRIGHTNESS) {
    res = V4L2_CID_BRIGHTNESS;
  } else if (val == P_EXPOSURE) {
    res = V4L2_CID_EXPOSURE;
  } else if (val == P_CONTRAST) {
    res = V4L2_CID_CONTRAST;
  } else if (val == P_SHARPNESS) {
    res = V4L2_CID_SHARPNESS;
  } else if (val == P_WHITE_BALANCE) {
    res = V4L2_CID_AUTO_WHITE_BALANCE;
  } else if (val == P_HUE) {
    res = V4L2_CID_HUE;
  } else if (val == P_SATURATION) {
    res = V4L2_CID_SATURATION;
  } else if (val == P_GAMMA) {
    res = V4L2_CID_GAMMA;
  } else if (val == P_GAIN) {
    res = V4L2_CID_GAIN;
  } else if (val == P_TEMPERATURE) {
    res = V4L2_CID_WHITE_BALANCE_TEMPERATURE;
  } else if (val == P_FRAME_RATE) {
    res = GlobalV4Linstance::V4L2_FEATURE_FRAME_RATE;
  } else {
    valid = false;
  }
  return res;
}

bool CaptureV4L::startCapture() {
  mutex.lock();

  camera_instance = GlobalV4LinstanceManager::obtainInstance(v_cam_bus->getInt());
  if (!camera_instance) {
    fprintf(stderr, "CaptureV4L: unable to obtain instance of camera id %d in startCapture!\n", v_cam_bus->getInt());
    mutex.unlock();
    return false;
  }

  // Grab current parameters
  width = v_width->getInt();
  height = v_height->getInt();
  left = v_left->getInt();
  top = v_top->getInt();
  capture_format = Colors::stringToColorFormat(v_colormode->getString().c_str());
  pixel_format = stringToPixelFormat(v_format->getString());
  int fps = min(v_fps->getInt(), 60);

  // Check configuration parameters:
  if (v_fps->getInt() > 60) {
    fprintf(stderr, "CaptureV4L Error: The library does not support frame rates higher than 60 fps");
  }

  if (!camera_instance->startStreaming(width, height, pixel_format, fps)) {
    fprintf(stderr, "CaptureV4L Error: unable to setup capture. Maybe selected combination of Format/Resolution is not supported?\n");
    mutex.unlock();
    return false;
  }

  rawFrame.ensure_allocation(capture_format, width, height);

  vector<VarType *> l = capture_settings->getChildren();
  for (auto & i : l) {
    i->addFlags(VARTYPE_FLAG_READONLY);
  }

  dcam_parameters->removeFlags(VARTYPE_FLAG_HIDE_CHILDREN);
  mutex.unlock();

  readAllParameterProperties();
  printf("CaptureV4L Info: Restoring Previously Saved Camera Parameters\n");
  writeAllParameterValues();
  readAllParameterValues();

  mutex.lock();
  camera_instance->captureWarm();

  is_capturing = true;

  mutex.unlock();

  return true;
}

bool CaptureV4L::copyAndConvertFrame(const RawImage &src, RawImage &target) {
  mutex.lock();
  ColorFormat output_fmt = Colors::stringToColorFormat(v_colorout->getSelection().c_str());
  if (src.getData() == nullptr) {
    mutex.unlock();
    return false;
  }
  target.setTime(src.getTime());
  target.setTimeCam(src.getTimeCam());

  ColorFormat src_fmt = src.getColorFormat();

  target.ensure_allocation(output_fmt, src.getWidth(), src.getHeight());
  if (output_fmt == src_fmt) {
    // We have to copy the data here to avoid a potential double free
    // If we do 'target = src', both RawImages own the same data, and if both try to free/delete it,
    // i.e. due to an ensure_allocation(), a double free error will occur
    target.deepCopyFromRawImage(src, false);
  } else {
    if (src_fmt == COLOR_RGB8 && output_fmt == COLOR_YUV422_UYVY) {
      Conversions::rgb2uyvy(src.getData(), target.getData(), width, height);
    } else if (src_fmt == COLOR_RGB8 && output_fmt == COLOR_YUV422_YUYV) {
      Conversions::rgb2yuyv(src.getData(), target.getData(), width, height);
    } else {
      fprintf(stderr, "Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",
              Colors::colorFormatToString(src_fmt).c_str(),
              Colors::colorFormatToString(output_fmt).c_str());
      mutex.unlock();
      return false;
    }
  }
  mutex.unlock();
  return true;
}

RawImage CaptureV4L::getFrame() {
  mutex.lock();

  if (!camera_instance->captureFrame(&rawFrame, pixel_format)) {
    fprintf(stderr, "CaptureV4L Warning: Frame not ready, camera %d\n", v_cam_bus->getInt());
    mutex.unlock();
    return RawImage{};
  }

  mutex.unlock();
  return rawFrame;
}

void CaptureV4L::releaseFrame() {
  mutex.lock();
  mutex.unlock();
}



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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      //open
#include <unistd.h>     //close
#include <jpeglib.h>

namespace {
uint32_t stringToPixelFormat(const std::string &format) {
  if (format == "YUYV") {
    return V4L2_PIX_FMT_YUYV;
  } else if (format == "MJPEG") {
    return V4L2_PIX_FMT_MJPEG;
  } else {
    // TODO(dschwab): If we update to c++14/17 then we can return a nullopt
    // and let the caller decide what the default type should be.
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

GlobalV4LinstanceManager* GlobalV4LinstanceManager::pinstance = NULL;

GlobalV4Linstance* GlobalV4LinstanceManager::obtainInstance(int iDevice) {
    if (!pinstance) pinstance=new GlobalV4LinstanceManager();
    if (pinstance->map_instance.find(iDevice)==pinstance->map_instance.end()) {
        pinstance->map_instance[iDevice] = new GlobalV4Linstance();
    }
    if (!pinstance->map_instance[iDevice]->obtainInstance(iDevice)) return NULL;
    return pinstance->map_instance[iDevice];
}
bool GlobalV4LinstanceManager::removeInstance(GlobalV4Linstance *pDevice) {
    for (t_map_v4l::iterator it=pinstance->map_instance.begin();
         it!=pinstance->map_instance.end(); it++) {
        if (it->second == pDevice) {
            if (pDevice->removeInstance()) {
                delete pDevice;
            }
            pinstance->map_instance.erase(it);
            return true;
        }
    }
    return false;       // didn't find it
}
bool GlobalV4LinstanceManager::removeInstance(int iDevice) {
    if (pinstance->map_instance.find(iDevice)!=pinstance->map_instance.end()) return false;
    return removeInstance(pinstance->map_instance[iDevice]);
}
GlobalV4LinstanceManager::GlobalV4LinstanceManager() {
    //instance=new GlobalV4Linstance();
}
GlobalV4LinstanceManager::~GlobalV4LinstanceManager() {
    for (t_map_v4l::iterator it=pinstance->map_instance.begin();
         it!=pinstance->map_instance.end(); it++) {
        delete it->second;
    }
    map_instance.clear();
}

int GlobalV4LinstanceManager::enumerateInstances(int *id_list, int max_id)
{
    int iFoundDevices = 0;
    for (int cam_id=0; cam_id < max_id; cam_id++) {
        GlobalV4Linstance *pDevice = GlobalV4LinstanceManager::obtainInstance(cam_id);
        if (pDevice) {
            id_list[iFoundDevices++] = cam_id;
            GlobalV4LinstanceManager::removeInstance(pDevice);
        }
    }
    return iFoundDevices;
}


//======================= Actual V4L device interface =======================

bool GlobalV4Linstance::obtainInstance(int iDevice)
{
    char szDeviceLocal[128];
    snprintf(szDeviceLocal, 128, "/dev/video%d", iDevice);
    return obtainInstance(szDeviceLocal);
}

bool GlobalV4Linstance::obtainInstance(char *szDevice_)
{
    lock();
    bool bSuccess = (pollset.fd > -1);
    //fprintf(stderr,"WARNING: obtainInstance ENTER : counter : %d!\n", counter);

    if (counter == 0) {
        if (pollset.fd < 0) {
            counter = 1;
            removeInstance(false);
        }
        
        pollset.fd = open(szDevice_, O_RDWR|O_NONBLOCK);
        if(pollset.fd > -1) bSuccess = true;
        
        if (bSuccess) {
            pollset.events = POLLIN;
            
            // test capture capabilities
            v4l2_capability cap;
            if(xioctl(pollset.fd, VIDIOC_QUERYCAP, &cap, "VideoQueryCap") == 0){
                if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
                   !(cap.capabilities & V4L2_CAP_STREAMING)){
                    bSuccess = false;
                }
            }else{
                bSuccess = false;
            }
        }
        
        // set cropping to default (no effect if not supported)
        if(bSuccess && false){
            v4l2_cropcap cropcap;
            v4l2_crop crop;
            mzero(cropcap);
            mzero(crop);
            cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            xioctl(pollset.fd, VIDIOC_CROPCAP, &cropcap, "CropCap");
            crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            crop.c = cropcap.defrect;
            xioctl(pollset.fd, VIDIOC_S_CROP, &crop, "SetCrop");
        }
        if (!bSuccess ) {
            if(pollset.fd > -1) close(pollset.fd);
            pollset.fd = -1;
        }
        else {
            strncpy(szDevice, szDevice_, 128);
        }
    }
    if (bSuccess)
        counter++;
    //fprintf(stderr,"WARNING: obtainInstance EXIT : counter : %d!\n", counter);
    
    unlock();
    return bSuccess;
}

bool GlobalV4Linstance::removeInstance(bool bRelock)
{
    if (bRelock) lock();
    counter--;
    bool was_last=false;
    if (counter == 0) {
        if(pollset.fd >= 0) {
            stopStreaming();
            close(pollset.fd);
        }
        pollset.fd = -1;
        memset(szDevice, 0, sizeof(char)*128);
        was_last=true;
    }
    if (counter < 0) {
        fprintf(stderr,"WARNING: Attempting to remove more instances than have been created!\n");
        fflush(stderr);
        counter = 0;
    }
    if (bRelock) unlock();
    return was_last;
}

bool GlobalV4Linstance::xioctl(int request, void *data, const char *error_str)
{
    int ret = xioctl(pollset.fd,request,data, error_str);
    return (ret == 0);
}

int GlobalV4Linstance::xioctl(int fd,int request,void *data,
                              const char *error_str)
{
    // try the ioctl, which should succeed in the common case
    int ret = ioctl(fd,request,data);
    if(ret >= 0) return(ret);
    
    // retry if we were interrupted (up to a few times)
    int n=0;
    while(ret!=0 && errno==EINTR && n<8){
        ret = ioctl(fd,request,data);
        n++;
    }
    
    // report error
    if(ret != 0 && error_str) {
        fprintf(stderr,"GlobalV4Linstance: %s returned %d (%s)\n",
                error_str,ret,strerror(errno));
    }
    
    return(ret);
}

bool GlobalV4Linstance::captureFrame(RawImage *pImage, uint32_t pixel_format, int iMaxSpin)
{
    const image_t *_img = captureFrame(iMaxSpin);       //low-level fetch
    if (!_img || _img->data==NULL) return false;
    bool bSuccess = false;

    if (_img) {
        if (pImage) {                                       //allow null to stoke the capture
            unsigned char *pDest = pImage->getData();
            if (!pDest) {                                   //we don't want low-level to reallocate!
                fprintf(stderr,"GlobalV4Linstance: RawImage not pre-allocated in captureFrame in device '%s'\n",
                        szDevice);
            }
            lock();

            //mid-level copy to RGB
            if (_img->data && getImage(*_img, pixel_format, pImage)) {
              // just copy timestamp from v4l buffer
              // http://www.linuxtv.org/downloads/v4l-dvb-apis/buffer.html
              // http://linux.die.net/man/2/gettimeofday
              /*
              timeval tv;
              pImage->setTime(0.0);
              //TODO: we could copy the timestamp from the tempbuf/image itself
              gettimeofday(&tv,NULL);
              pImage->setTime((double)tv.tv_sec + tv.tv_usec*(1.0E-6));
              */
              pImage->setTime((double)_img->timestamp.tv_sec +
                              _img->timestamp.tv_usec * (1.0E-6));
              bSuccess = true;
            }
            unlock();
        }
        if ( !releaseFrame(_img))                            //maybe we shouldn't return an error?
            return false;
    }
    
    return bSuccess;
}


const GlobalV4Linstance::image_t *GlobalV4Linstance::captureFrame(int iMaxSpin)
{
    if(!waitForFrame(300)) {
        fprintf(stderr,"GlobalV4Linstance: error waiting for frame '%s'\n", szDevice);
        return(NULL);
    }
    
    do {
        // get the frame
        dequeueBuffer(tempbuf);
        
        // poll to see if a another frame is already available
        // if so, break out now
        if(!waitForFrame(0)) break;
        
        // otherwise, drop this frame
        enqueueBuffer(tempbuf);
    }
    while(iMaxSpin--);
    
    int i = tempbuf.index;
    img[i].timestamp = tempbuf.timestamp;
    img[i].field = (tempbuf.field == V4L2_FIELD_BOTTOM);
    return(&(img[i]));
}

bool GlobalV4Linstance::releaseFrame(const GlobalV4Linstance::image_t *_img)
{
    if(!_img) return(false);
    tempbuf.index = _img->index;
    return(enqueueBuffer(tempbuf));
}

//because there is some delay before camera actually starts, spin
// here for the first frame to come out
void GlobalV4Linstance::captureWarm(int iMaxSpin)
{
    const GlobalV4Linstance::image_t *_img = NULL;
    while (!_img) {
        _img = captureFrame(iMaxSpin);
    }
    if (_img->data)
        releaseFrame(_img);
}

bool GlobalV4Linstance::waitForFrame(int max_msec)
{
    if (pollset.fd==-1) return false;
    int n = poll(&pollset,1,max_msec);
    return(n==1 && (pollset.revents & POLLIN)!=0);
}

bool GlobalV4Linstance::startStreaming(int iWidth_, int iHeight_, uint32_t pixel_format, int framerate, int iInputIdx)
{
    struct v4l2_requestbuffers req;
    
    // Set video format
    v4l2_format fmt;
    mzero(fmt);
    fmt.fmt.pix.width       = iWidth_;
    fmt.fmt.pix.height      = iHeight_;
    fmt.fmt.pix.pixelformat = pixel_format;
    fmt.fmt.pix.field       = V4L2_FIELD_ALTERNATE;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (!xioctl(VIDIOC_S_FMT, &fmt, "SetFormat")) {
        printf("Warning: Could not set format, '%s'; was device previously started?\n", szDevice);
 //       return(false);
    }
    
    // Set Input and Controls (for more advanced v4l as well)
    // http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-input.html
    // vid.setInput(0); // main capture (cable or camera video)
    // vid.setInput(1); // Component video
    // vid.setInput(2); // S-video
    xioctl(VIDIOC_S_INPUT,&iInputIdx,"SetInput");
    /*
    v4l2_std_id id = V4L2_STD_NTSC_M;               //TODO: modify/detect for other cameras
    xioctl(VIDIOC_S_STD,&id,"SetStandard");
    
    long lControlVal = static_cast<long>(0.5f*(1 << 16));
    setControl(V4L2_CID_BRIGHTNESS, lControlVal);
    setControl(V4L2_CID_HUE, lControlVal);
    setControl(V4L2_CID_CONTRAST, lControlVal);
    lControlVal = static_cast<long>(0.9f*(1 << 16));
    setControl(V4L2_CID_SATURATION, lControlVal);
     */

    // Request mmap-able capture buffers
    mzero(req);
    req.count  = V4L_STREAMBUFS;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if(!xioctl(VIDIOC_REQBUFS, &req, "Request Buffers") || req.count != V4L_STREAMBUFS) {
        printf("REQBUFS returned error, count %d\n", req.count);
        return(false);
    }

    struct v4l2_streamparm parm;
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = framerate;
    if (!xioctl(VIDIOC_S_PARM, &parm, "SetParm")) {
        printf("Warning: Could not set parameters, '%s'; was device previously started?\n", szDevice);
    }
    
    // set up individual buffers
    mzero(img,V4L_STREAMBUFS);
    mzero(tempbuf);
    tempbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tempbuf.memory = V4L2_MEMORY_MMAP;
    
    for(unsigned i=0; i<req.count; i++){
        tempbuf.index = i;
        if(!xioctl(VIDIOC_QUERYBUF, &tempbuf, "Allocate query buffer")) {
            printf("QUERYBUF returned error, '%s'\n", szDevice);
            return(false);
        }
        img[i].index = i;
        img[i].length = tempbuf.length;
        img[i].data = static_cast<unsigned char*>(
              mmap(NULL, tempbuf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                   pollset.fd, tempbuf.m.offset));
        
        if(img[i].data == MAP_FAILED){
            printf("mmap() returned error %d (%s)\n",errno,strerror(errno));
            return(false);
        }
    }
    
    //enqueue buffer that we just memmapped/allocated
    for(unsigned i=0; i<req.count; i++){
        tempbuf.index = i;
        if(!enqueueBuffer(tempbuf)){
            printf("Error queueing initial buffers, '%s'\n", szDevice);
            return(false);
        }
    }
    
    //start actual stream
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    return xioctl(VIDIOC_STREAMON, &type, "StreamOn");
}

bool GlobalV4Linstance::stopStreaming()
{
    bool bSuccess = false;
    if(pollset.fd != -1){
        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bSuccess = xioctl(VIDIOC_STREAMOFF, &type, NULL);
        
        for(int i=0; i<V4L_STREAMBUFS; i++){
            if(img[i].data){
                munmap(img[i].data, img[i].length);
                img[i].data = NULL;
                img[i].length = 0;
            }
        }
    }
    
    //don't close the stream just yet
    
    return bSuccess;
}

bool GlobalV4Linstance::enqueueBuffer(v4l2_buffer &buf)
{
    return xioctl(VIDIOC_QBUF, &buf, "EnqueueBuffer");
}

bool GlobalV4Linstance::dequeueBuffer(v4l2_buffer &buf)
{
    return xioctl(VIDIOC_DQBUF, &buf, "DequeueBuffer");
}

bool GlobalV4Linstance::checkControl(int ctrl_id, bool *bEnabled, bool *bReadOnly,
                                     long *lDefault, long *lMin, long *lMax) {
    v4l2_queryctrl queryctrl;
    mzero(queryctrl);
    queryctrl.id = ctrl_id;
    
    if(xioctl(VIDIOC_QUERYCTRL, &queryctrl, "CheckControl")) {
        if (bEnabled) (*bEnabled) = !(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED);
        if (bReadOnly) (*bReadOnly) = queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY;
        if (lMin) (*lMin) = queryctrl.minimum;
        if (lMax) (*lMax) = queryctrl.maximum;
        if (lDefault) (*lDefault) = queryctrl.default_value;
        return true;
    }
    return false;
}


bool GlobalV4Linstance::getControl(int ctrl_id, long &s)
{
    v4l2_control ctrl;
    mzero(ctrl);
    ctrl.id = ctrl_id;
    
    if(xioctl(VIDIOC_G_CTRL, &ctrl, "GetControl")) {
        s = ctrl.value;
        return(true);
    }else{
        return(false);
    }
}

bool GlobalV4Linstance::setControl(int ctrl_id, long s)
{
    v4l2_control ctrl;
    mzero(ctrl);
    ctrl.id = ctrl_id;
    ctrl.value = s;
    
    if(xioctl(VIDIOC_S_CTRL, &ctrl, "SetControl")){
        return(true);
    }else{
        return(false);
    }
}

//======================= Low level utility functions ====================
bool GlobalV4Linstance::writeYuyvPPM(GlobalV4Linstance::yuyv *pSrc, int width, int height, const char *filename)
{
    GlobalV4Linstance::rgb *bufrgb = NULL;
    if (!getImageRgb(pSrc, width,height,&bufrgb)) return false;
    int wrote;
    wrote = writeRgbPPM(bufrgb,width,height,filename);
    delete[](bufrgb);
    
    return(wrote > 0);
}

bool GlobalV4Linstance::writeRgbPPM(GlobalV4Linstance::rgb *imgbuf, int width, int height, const char *filename)
{
    // open output file
    FILE *out = fopen(filename,"wb");
    if(!out) return(false);
    
    // write the image
    fprintf(out,"P6\n%d %d\n%d\n",width,height,255);
    int result=fwrite(imgbuf,3,width*height,out);
    (void)result; //get the compiler to shut up.
    
    return(fclose(out) == 0);
}

bool GlobalV4Linstance::getImageRgb(GlobalV4Linstance::yuyv *pSrc, int width, int height, GlobalV4Linstance::rgb **rgbbuf)
{
    if (!rgbbuf) return false;
    if ((*rgbbuf)==NULL)
        (*rgbbuf) = new rgb[width * height];
    
    int size = width*height;
    GlobalV4Linstance::rgb *pDest = (*rgbbuf);
    GlobalV4Linstance::yuv pxCopy;
    for (int iP=0; iP<size; iP++) {
        pxCopy.y = (iP&1)? pSrc->y2 : pSrc->y1;
        pxCopy.u = pSrc->u;
        pxCopy.v = pSrc->v;
        (*pDest) = yuv2rgb(pxCopy);
        pDest++;                //advance destination always
        if (iP&1) pSrc++;       //avoid odd field advancement
    }
    return true;
}

bool GlobalV4Linstance::getImageFromJPEG(
    const GlobalV4Linstance::image_t &in_img, RawImage *out_img) {
  struct jpeg_decompress_struct dinfo;
  struct jpeg_error_mgr jerr;
  dinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&dinfo);
  jpeg_mem_src(&dinfo, in_img.data, in_img.length);
  jpeg_read_header(&dinfo, true);
  dinfo.output_components = 3;
  jpeg_start_decompress(&dinfo);

  int row_stride = out_img->getWidth() * 3;
  for (int row = 0; row < out_img->getHeight(); ++row) {
    JSAMPROW row_ptr = &out_img->getData()[row_stride * row];
    jpeg_read_scanlines(&dinfo, &row_ptr, 1);
  }
  // freeing jpeg_decompress_struct memory
  jpeg_destroy_decompress(&dinfo);
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
    return 0;
  };
}

GlobalV4Linstance::rgb GlobalV4Linstance::yuv2rgb(GlobalV4Linstance::yuv p)
{
    GlobalV4Linstance::rgb r;
    int y,u,v;
    
    y = p.y;
    u = p.v*2 - 255;
    v = p.u*2 - 255;
    
    r.red   = bound(y + u                     ,0,255);
    r.green = bound((int)(y - 0.51*u - 0.19*v),0,255);
    r.blue  = bound(y + v                     ,0,255);
    
    return(r);
}



//======================= GUI & API Definitions =======================

CaptureV4L::CaptureV4L(VarList * _settings,int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
{
    mzero(cam_list, MAX_CAM_SCAN);
    cam_count = 0;
    cam_id=default_camera_id;
    is_capturing=false;

    mutex.lock();
    
    cam_count = GlobalV4LinstanceManager::enumerateInstances(cam_list, MAX_CAM_SCAN);
    if (cam_count==0) {
        fprintf(stderr,"CaptureV4L Error: can't find cameras");
    }
    
    if (cam_id > cam_list[cam_count-1]) {
        static bool bMaxWarningShown = false;
        if (!bMaxWarningShown) {
            fprintf(stderr,"CaptureV4L Error: no camera found with index %d. Max index is: %d\n",cam_id,cam_list[cam_count-1]);
            bMaxWarningShown = true;
        }
    }
    else {
        camera_instance = GlobalV4LinstanceManager::obtainInstance(cam_id);
        if (!camera_instance) {
            fprintf(stderr,"CaptureV4L: unable to obtain instance of camera id %d!\n", cam_id);
        }
    }
    
    settings->addChild(conversion_settings = new VarList("Conversion Settings"));
    settings->addChild(capture_settings = new VarList("Capture Settings"));
    settings->addChild(dcam_parameters  = new VarList("Camera Parameters"));
    
    //=======================CONVERSION SETTINGS=======================
    conversion_settings->addChild(v_colorout=new VarStringEnum("convert to mode",Colors::colorFormatToString(COLOR_RGB8)));
    v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB8));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_RGB16));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_RAW8));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_RAW16));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_MONO8));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_MONO16));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV411));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV422_YUYV));
//    v_colorout->addItem(Colors::colorFormatToString(COLOR_YUV444));
    
    dcam_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );
    
    //=======================CAPTURE SETTINGS==========================
    capture_settings->addChild(v_cam_bus          = new VarInt("cam idx",default_camera_id));
    capture_settings->addChild(v_fps              = new VarInt("framerate",30));
    capture_settings->addChild(v_width            = new VarInt("width",640));
    capture_settings->addChild(v_height           = new VarInt("height",480));
    capture_settings->addChild(v_left            = new VarInt("left",0));
    capture_settings->addChild(v_top           = new VarInt("top",0));
    capture_settings->addChild(v_colormode        = new VarStringEnum("capture mode",Colors::colorFormatToString(COLOR_RGB8)));
    v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB8));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_RGB16));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_RAW8));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_RAW16));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_MONO8));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_MONO16));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV411));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV422_UYVY));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV422_YUYV));
//    v_colormode->addItem(Colors::colorFormatToString(COLOR_YUV444));

    v_format.reset(new VarStringEnum("pixel format", pixelFormatToString(V4L2_PIX_FMT_YUYV)));
    v_format->addItem(pixelFormatToString(V4L2_PIX_FMT_YUYV));
    v_format->addItem(pixelFormatToString(V4L2_PIX_FMT_MJPEG));
    capture_settings->addChild(v_format.get());
    capture_settings->addChild(v_buffer_size      = new VarInt("ringbuffer size",V4L_STREAMBUFS));
    v_buffer_size->addFlags(VARTYPE_FLAG_READONLY);

    // we could do a better job of enumerating formats here...
    // http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html
    // http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-framesizes.html#v4l2-frmsizeenum

    
    // v4l2_frmsizeenum
//    printf("sensor supported frame size:\n");
//    fsize.index = 0;
//    while (ioctl(fd_v4l, VIDIOC_ENUM_FRAMESIZES, &fsize) >= 0) {
//    	printf(" %dx%d\n", fsize.discrete.width,
//                 fsize.discrete.height);
//    	fsize.index++;
//    }

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
    
    vector<VarType *> v=dcam_parameters->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
        if (v[i]->getType()==VARTYPE_ID_LIST) {
            VarBool * temp;
            ((VarList *)v[i])->addChild(temp = new VarBool("was_read",false));
            temp->addFlags(VARTYPE_FLAG_HIDDEN);
        }
    }
    
    mvc_connect(P_FRAME_RATE);
    
    mutex.unlock();
}

void CaptureV4L::mvc_connect(VarList * group) {
    vector<VarType *> v=group->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
        connect(v[i],SIGNAL(wasEdited(VarType *)),group,SLOT(mvcEditCompleted()));
    }
    connect(group,SIGNAL(wasEdited(VarType *)),this,SLOT(changed(VarType *)));
}

void CaptureV4L::changed(VarType * group) {
    if (group->getType()==VARTYPE_ID_LIST) {
        writeParameterValues( (VarList *)group );
        readParameterValues( (VarList *)group );
    }
}


void CaptureV4L::readAllParameterValues() {
    vector<VarType *> v=dcam_parameters->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
        if (v[i]->getType()==VARTYPE_ID_LIST) {
            readParameterValues((VarList *)v[i]);
        }
    }
}

void CaptureV4L::readAllParameterProperties()
{
    vector<VarType *> v=dcam_parameters->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
        if (v[i]->getType()==VARTYPE_ID_LIST) {
            readParameterProperty((VarList *)v[i]);
        }
    }
}

void CaptureV4L::writeAllParameterValues() {
    vector<VarType *> v=dcam_parameters->getChildren();
    for (unsigned int i=0;i<v.size();i++) {
        if (v[i]->getType()==VARTYPE_ID_LIST) {
            writeParameterValues((VarList *)v[i]);
        }
    }
}

void CaptureV4L::readParameterValues(VarList * item) {
    if (!camera_instance) return;
    mutex.lock();
    bool valid=true;
    v4lfeature_t feature=getV4LfeatureEnum(item,valid);
    if (valid==false) {
        printf("INVALID FEATURE: %s\n",item->getName().c_str());
        mutex.unlock();
        return;
    }
    VarInt * vint=0;
    VarBool * venabled=0;
    VarBool * vwasread=0;
    
    vector<VarType *> children=item->getChildren();
    for (unsigned int i=0;i<children.size();i++) {
        if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="was_read") vwasread=(VarBool *)children[i];
        if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="enabled") venabled=(VarBool *)children[i];
        if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value") vint=(VarInt *)children[i];
    }
    
    long lValue;
    if (vwasread!=0 && vwasread->getBool()==true) {             //only proceed if no failure in past
        if (feature > GlobalV4Linstance::V4L2_FEATURE_PRIVATE) {       //custom/private features
            printf("UNIMPLEMENTED FEATURE (readParameterValues): %s\n",item->getName().c_str());
        }
        else {
            if (!camera_instance->getControl(feature,lValue)) {
                //feature doesn't exist
                printf("NON-READ FEATURE (readParameterValues): %s\n",item->getName().c_str());
                if (vwasread!=0) vwasread->setBool(false);
            } else {
                //check for switchability:
                if (vwasread!=0) vwasread->setBool(true);
                if (camera_instance->checkControl(feature)) {
                    venabled->setBool(true);
                } else {
                    venabled->setBool(false);
                }
                vint->setInt(lValue);
            }

            //update render flags:
            if (vint) {
                if (venabled->getBool() ) {
                    vint->removeFlags( VARTYPE_FLAG_READONLY );
                } else {
                    vint->addFlags( VARTYPE_FLAG_READONLY );
                }
            }
        }
    }
    
    
    mutex.unlock();
}

void CaptureV4L::writeParameterValues(VarList * item) {
    mutex.lock();
    bool valid=true;
    v4lfeature_t feature=getV4LfeatureEnum(item,valid);
    if (valid==false) {
        mutex.unlock();
        return;
    }
    VarInt * vint=0;
    VarBool * venabled=0;
    VarBool * vwasread=0;
    VarBool * vdefault=0;
    
    vector<VarType *> children=item->getChildren();
    for (unsigned int i=0;i<children.size();i++) {
        if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="was_read") vwasread=(VarBool *)children[i];
        if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="enabled") venabled=(VarBool *)children[i];
        if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="default") vdefault=(VarBool *)children[i];
        if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value") vint=(VarInt *)children[i];
    }
    
    //new: only apply parameters which were previously read from the camera
    if (feature > GlobalV4Linstance::V4L2_FEATURE_PRIVATE) {       //custom/private features
        printf("UNIMPLEMENTED FEATURE (writeParameterValues): %s\n",item->getName().c_str());
    }
    else {
        if (vwasread!=0 && vwasread->getBool()==true) {
            printf("ATTEMPTING TO SET (writeParameterValues): %s\n",item->getName().c_str());
            if (vint!=0) {
                if (vdefault && vdefault->getBool())
                    vint->resetToDefault();
                long lValue = static_cast<long>(vint->getInt());
                if (!camera_instance->setControl(feature,lValue)) {
                    //feature doesn't exist or broke after initial read
                    if (vwasread!=0) vwasread->setBool(false);
                    venabled->setBool(false);
                }
            }
        }
    }
    mutex.unlock();
}


void CaptureV4L::readParameterProperty(VarList * item) {
    if (!camera_instance) return;
    mutex.lock();
    bool debug=false;
    bool valid=true;
    v4lfeature_t feature=getV4LfeatureEnum(item,valid);
    if (valid==false) {
        fprintf(stderr,"ERROR: INVALID PROPERTY ENCOUNTERED DURING READOUT: %s\n", item->getName().c_str());
        mutex.unlock();
        return;
    }
    VarInt * vint=0;
    VarBool * venabled=0;
    
    vector<VarType *> children=item->getChildren();
    for (unsigned int i=0;i<children.size();i++) {
        if (children[i]->getType()==VARTYPE_ID_BOOL && children[i]->getName()=="enabled") venabled=(VarBool *)children[i];
        if (children[i]->getType()==VARTYPE_ID_INT && children[i]->getName()=="value") vint=(VarInt *)children[i];
    }
    
    long lDefault = 0;
    if (feature > GlobalV4Linstance::V4L2_FEATURE_PRIVATE) {       //custom/private features
        printf("UNIMPLEMENTED FEATURE (readParameterProperty): %s\n",item->getName().c_str());
    }
    else {
        if (!camera_instance->getControl(feature, lDefault)) {
            if (debug) fprintf(stderr,"V4L PROP FEATURE IS *NOT* PRESENT: %s\n", item->getName().c_str());
            //feature doesn't exist
            item->addFlags(VARTYPE_FLAG_READONLY|VARTYPE_FLAG_HIDE_CHILDREN);
        }
        else {
            if (debug) fprintf(stderr,"V4L PROP FEATURE IS PRESENT: %s\n", item->getName().c_str());
            item->removeFlags(VARTYPE_FLAG_READONLY|VARTYPE_FLAG_HIDE_CHILDREN);
            //check for switchability:
            bool bReadOnly, bEnabled;
            long lMin, lMax;
            if (camera_instance->checkControl(feature, &bEnabled, &bReadOnly, &lDefault, &lMin, &lMax)) {
                venabled->setBool(bEnabled);
                if (bReadOnly)
                    venabled->removeFlags( VARTYPE_FLAG_READONLY);
                else
                    venabled->addFlags( VARTYPE_FLAG_READONLY);
                if (vint!=0) {
                    vint->setMin(lMin);
                    vint->setMax(lMax);
                    vint->setDefault(lDefault);
                }
            }
            else {
                venabled->setBool(false);
                venabled->addFlags( VARTYPE_FLAG_READONLY);
            }
        }
    }
    mutex.unlock();
}


CaptureV4L::~CaptureV4L()
{
    GlobalV4LinstanceManager::removeInstance(camera_instance);
    rawFrame.clear();           //release memory from local image
}

bool CaptureV4L::resetBus() {
    mutex.lock();

    cam_count = GlobalV4LinstanceManager::enumerateInstances(cam_list, MAX_CAM_SCAN);
    if (cam_count==0) {
        fprintf(stderr,"CaptureV4L Error: can't find cameras");
        mutex.unlock();
        return false;
    }
    
    mutex.unlock();
    return true;
    
}

bool CaptureV4L::stopCapture()
{
    if (isCapturing()) {
        readAllParameterValues();

        vector<VarType *> tmp = capture_settings->getChildren();
        for (unsigned int i=0; i < tmp.size();i++) {
            tmp[i]->removeFlags( VARTYPE_FLAG_READONLY );
        }
        dcam_parameters->addFlags( VARTYPE_FLAG_HIDE_CHILDREN );
    }
    cleanup();
    
    return true;
}


void CaptureV4L::cleanup()
{
    mutex.lock();
    
    //TODO: cleanup/free any memory buffers.
    if (camera_instance && is_capturing)
        camera_instance->stopStreaming();
    is_capturing=false;
    mutex.unlock();
    
}

/// This function converts a local dcam_parameters-manager variable ID
/// to a v4l feature enum
// http://v4l-test.sourceforge.net/spec/x542.htm
v4lfeature_t CaptureV4L::getV4LfeatureEnum(VarList * val, bool & valid)
{
    v4lfeature_t res=V4L2_CID_BASE;
    if (val== P_BRIGHTNESS) {
        res= V4L2_CID_BRIGHTNESS;
    } else if (val== P_EXPOSURE) {
        res= V4L2_CID_EXPOSURE;
    } else if (val== P_CONTRAST) {
        res= V4L2_CID_CONTRAST;
    } else if (val== P_SHARPNESS) {
        res= V4L2_CID_SHARPNESS;
    } else if (val== P_WHITE_BALANCE) {
        res= V4L2_CID_AUTO_WHITE_BALANCE;
    } else if (val== P_HUE) {
        res= V4L2_CID_HUE;
    } else if (val== P_SATURATION) {
        res= V4L2_CID_SATURATION;
    } else if (val== P_GAMMA) {
        res= V4L2_CID_GAMMA;
    } else if (val== P_GAIN) {
        res= V4L2_CID_GAIN;
    } else if (val== P_TEMPERATURE) {
        res= V4L2_CID_WHITE_BALANCE_TEMPERATURE;
    } else if (val== P_FRAME_RATE) {
        res= GlobalV4Linstance::V4L2_FEATURE_FRAME_RATE;
    } else {
        valid=false;
    }
    return res;
}

/// This function converts a v4l feature into a variable ID
// http://v4l-test.sourceforge.net/spec/x542.htm
VarList * CaptureV4L::getVariablePointer(v4lfeature_t val)
{
    VarList * res=0;
    switch (val) {
        case V4L2_CID_BRIGHTNESS: res=P_BRIGHTNESS; break;
        case V4L2_CID_CONTRAST: res=P_CONTRAST; break;
        case V4L2_CID_SATURATION: res=P_SATURATION; break;
        case V4L2_CID_SHARPNESS: res=P_SHARPNESS; break;
        case V4L2_CID_HUE: res=P_HUE; break;
        case V4L2_CID_AUTO_WHITE_BALANCE: res=P_WHITE_BALANCE; break;  //bool
        case V4L2_CID_GAMMA: res=P_GAMMA; break;
        case V4L2_CID_EXPOSURE: res=P_EXPOSURE; break;
        case V4L2_CID_GAIN: res=P_GAIN; break;
        case V4L2_CID_WHITE_BALANCE_TEMPERATURE: res=P_TEMPERATURE; break;
        case GlobalV4Linstance::V4L2_FEATURE_FRAME_RATE: res=P_FRAME_RATE; break;
        default: res=0; break;
    }
    return res;
}

bool CaptureV4L::startCapture()
{
    if (!v_cam_bus) return false;
    mutex.lock();
    //disable any previous activity on that camera:
    if (is_capturing)
        camera_instance->stopStreaming();
    
    //dynamically fetch the camera ID and reconnect now
    int new_cam_id = v_cam_bus->getInt();
    if (cam_id != new_cam_id) {
        cam_id = new_cam_id;
        GlobalV4LinstanceManager::removeInstance(camera_instance);
        camera_instance = NULL;
        if (cam_id > cam_list[cam_count-1]) {
            static bool bMaxWarningShown = false;
            if (!bMaxWarningShown) {
                fprintf(stderr,"CaptureV4L Error: no camera found with index %d. Max index is: %d\n",cam_id,cam_list[cam_count-1]);
                bMaxWarningShown = true;
            }
        }
        else {
            camera_instance = GlobalV4LinstanceManager::obtainInstance(cam_id);
        }
    }
    if (!camera_instance) {
        fprintf(stderr,"CaptureV4L: unable to obtain instance of camera id %d in startCapture!\n", cam_id);
        mutex.unlock();
        return false;
    }
    
    //grab current parameters:
    width=v_width->getInt();
    height=v_height->getInt();
    left=v_left->getInt();
    top=v_top->getInt();
    capture_format=Colors::stringToColorFormat(v_colormode->getString().c_str());
    pixel_format = stringToPixelFormat(v_format->getString());
    int fps=v_fps->getInt();
    //CaptureMode mode=stringToCaptureMode(v_format->getString().c_str());
    ring_buffer_size=v_buffer_size->getInt();
    
    //Check configuration parameters:
    if (fps > 60 ) {
        fprintf(stderr,"CaptureV4L Error: The library does not support framerates higher than 60 fps (does your camera?).");
        mutex.unlock();
        return false;
    }
    
    
    /* ---- TODO: adapt to do checking based on available USB mode?
     
     
    dc1394_video_set_transmission(camera,DC1394_OFF);
    
    dc1394video_modes_t supported_modes;
    bool know_modes=false;
    if (dc1394_video_get_supported_modes(camera,&supported_modes) == DC1394_SUCCESS) {
        know_modes=true;
    } else {
        fprintf(stderr,"CaptureV4L Warning: unable to query supported camera modes!\n");
    }
    
    bool native_unavailable = false;
    if (mode==CAPTURE_MODE_AUTO || mode==CAPTURE_MODE_NATIVE) {
        if (width==160 && height==120) {
            if (capture_format==COLOR_YUV444) {
                dcformat=DC1394_VIDEO_MODE_160x120_YUV444;
            } else {
                native_unavailable = true;
            }
        } else if (width==320 && height==240) {
            if (capture_format==COLOR_YUV422_UYVY) {
                dcformat=DC1394_VIDEO_MODE_320x240_YUV422;
            } else {
                native_unavailable = true;
            }
        } else if (width==640 && height==480) {
            if (capture_format==COLOR_YUV411) {
                dcformat=DC1394_VIDEO_MODE_640x480_YUV411;
            } else if (capture_format==COLOR_YUV422_UYVY) {
                dcformat=DC1394_VIDEO_MODE_640x480_YUV422;
            } else if (capture_format==COLOR_RGB8) {
                dcformat=DC1394_VIDEO_MODE_640x480_RGB8;
            } else if (capture_format==COLOR_MONO8) {
                dcformat=DC1394_VIDEO_MODE_640x480_MONO8;
            } else if (capture_format==COLOR_MONO16) {
                dcformat=DC1394_VIDEO_MODE_640x480_MONO16;
            } else {
                native_unavailable = true;
            }
        } else if (width==800 && height==600) {
            if (capture_format==COLOR_YUV422_UYVY) {
                dcformat=DC1394_VIDEO_MODE_800x600_YUV422;
            } else if (capture_format==COLOR_RGB8) {
                dcformat=DC1394_VIDEO_MODE_800x600_RGB8;
            } else if (capture_format==COLOR_MONO8) {
                dcformat=DC1394_VIDEO_MODE_800x600_MONO8;
            } else if (capture_format==COLOR_MONO16) {
                dcformat=DC1394_VIDEO_MODE_800x600_MONO16;
            } else {
                native_unavailable = true;
            }
        } else if (width==1024 && height==768) {
            if (capture_format==COLOR_YUV422_UYVY) {
                dcformat=DC1394_VIDEO_MODE_1024x768_YUV422;
            } else if (capture_format==COLOR_RGB8) {
                dcformat=DC1394_VIDEO_MODE_1024x768_RGB8;
            } else if (capture_format==COLOR_MONO8) {
                dcformat=DC1394_VIDEO_MODE_1024x768_MONO8;
            } else if (capture_format==COLOR_MONO16) {
                dcformat=DC1394_VIDEO_MODE_1024x768_MONO16;
            } else {
                native_unavailable = true;
            }
        } else if (width==1280 && height==960) {
            if (capture_format==COLOR_YUV422_UYVY) {
                dcformat=DC1394_VIDEO_MODE_1280x960_YUV422;
            } else if (capture_format==COLOR_RGB8) {
                dcformat=DC1394_VIDEO_MODE_1280x960_RGB8;
            } else if (capture_format==COLOR_MONO8) {
                dcformat=DC1394_VIDEO_MODE_1280x960_MONO8;
            } else if (capture_format==COLOR_MONO16) {
                dcformat=DC1394_VIDEO_MODE_1280x960_MONO16;
            } else {
                native_unavailable = true;
            }
        } else if (width==1600 && height==1200) {
            if (capture_format==COLOR_YUV422_UYVY) {
                dcformat=DC1394_VIDEO_MODE_1600x1200_YUV422;
            } else if (capture_format==COLOR_RGB8) {
                dcformat=DC1394_VIDEO_MODE_1600x1200_RGB8;
            } else if (capture_format==COLOR_MONO8) {
                dcformat=DC1394_VIDEO_MODE_1600x1200_MONO8;
            } else if (capture_format==COLOR_MONO16) {
                dcformat=DC1394_VIDEO_MODE_1600x1200_MONO16;
            } else {
                native_unavailable = true;
            }
        } else {
            native_unavailable = true;
        }
        if (native_unavailable==false && know_modes) {
            bool found_mode=false;
            for (unsigned int i=0;i<supported_modes.num;i++) {
                if (supported_modes.modes[i]==dcformat) {
                    found_mode=true;
                    break;
                }
            }
            if (found_mode==false) {
                native_unavailable=true;
            }
        }
        
        if (native_unavailable==true) {
            if (mode==CAPTURE_MODE_AUTO) {
                printf("CaptureV4L Info: Selected format/resolution not supported as FORMAT 0\n");
                printf("CaptureV4L Info: Selecting lowest available Format 7 mode\n");
                if (know_modes) {
                    bool found=false;
                    for (unsigned int i=0;i<supported_modes.num;i++) {
                        if (supported_modes.modes[i] >= DC1394_VIDEO_MODE_FORMAT7_MIN && supported_modes.modes[i]<=DC1394_VIDEO_MODE_FORMAT7_MAX) {
                            if (found==false || supported_modes.modes[i] < dcformat) {
                                found=true;
                                dcformat=supported_modes.modes[i];
                            }
                        }
                    }
                    if (found==false) {
                        fprintf(stderr,"CaptureV4L Error: No Format 7 modes available!");
                        fprintf(stderr,"CaptureV4L Error: Maybe try selecting a supported Format 0 resolution/framerate?");
#ifndef VDATA_NO_QT
                        mutex.unlock();
#endif
                        return false;
                    }
                } else {
                    dcformat=DC1394_VIDEO_MODE_FORMAT7_0;
                }
                
            } else {
                fprintf(stderr,"CaptureV4L Error: Selected color format/resolution not natively supported!");
                fprintf(stderr,"CaptureV4L Error: Maybe try switching to auto or a format7 mode.");
#ifndef VDATA_NO_QT
                mutex.unlock();
#endif
                return false;
            }
        }
    } else {
        if (mode==CAPTURE_MODE_FORMAT_7_MODE_0) dcformat = DC1394_VIDEO_MODE_FORMAT7_0;
        if (mode==CAPTURE_MODE_FORMAT_7_MODE_1) dcformat = DC1394_VIDEO_MODE_FORMAT7_1;
        if (mode==CAPTURE_MODE_FORMAT_7_MODE_2) dcformat = DC1394_VIDEO_MODE_FORMAT7_2;
        if (mode==CAPTURE_MODE_FORMAT_7_MODE_3) dcformat = DC1394_VIDEO_MODE_FORMAT7_3;
        if (mode==CAPTURE_MODE_FORMAT_7_MODE_4) dcformat = DC1394_VIDEO_MODE_FORMAT7_4;
        if (mode==CAPTURE_MODE_FORMAT_7_MODE_5) dcformat = DC1394_VIDEO_MODE_FORMAT7_5;
        if (mode==CAPTURE_MODE_FORMAT_7_MODE_6) dcformat = DC1394_VIDEO_MODE_FORMAT7_6;
        if (mode==CAPTURE_MODE_FORMAT_7_MODE_7) dcformat = DC1394_VIDEO_MODE_FORMAT7_7;
    }
    
    
    if (know_modes) {
        //check whether capture mode is supported:
        bool found=false;
        for (unsigned int i=0;i<supported_modes.num;i++) {
            if (supported_modes.modes[i]==dcformat) {
                found=true;
                break;
            }
        }
        if (found==false) {
            fprintf(stderr,"CaptureV4L Error: Selected mode (format/resolution/framerate) is not supported by camera!\n");
#ifndef VDATA_NO_QT
            mutex.unlock();
#endif
            cleanup();
            return false;
        } else {
        }
    }
    
    
    if (dc1394_video_set_mode(camera,dcformat) !=  DC1394_SUCCESS) {
        fprintf(stderr,"CaptureV4L Error: unable to set capture mode\n");
#ifndef VDATA_NO_QT
        mutex.unlock();
#endif
        cleanup();
        return false;
    }
    //verify mode:
    dc1394video_mode_t check_mode;
    if (dc1394_video_get_mode(camera,&check_mode) ==  DC1394_SUCCESS) {
        if (check_mode!=dcformat) {
            fprintf(stderr,"CaptureV4L Error: Unable to set selected mode\n");
#ifndef VDATA_NO_QT
            mutex.unlock();
#endif
            cleanup();
            return false;
        }
    }
     */

    
    //TODO: map cature_format to VFL format
    //  http://linuxtv.org/downloads/v4l-dvb-apis/yuv-formats.html
    
    if (!camera_instance->startStreaming(width, height, pixel_format, fps)) {
        fprintf(stderr,"CaptureV4L Error: unable to setup capture. Maybe selected combination of Format/Resolution is not supported?\n");
        mutex.unlock();
        cleanup();
        return false;
    }
    
    vector<VarType *> l = capture_settings->getChildren();
    for (unsigned int i = 0; i < l.size(); i++) {
        l[i]->addFlags(VARTYPE_FLAG_READONLY);
    }
    
    vector<VarType *> tmp = capture_settings->getChildren();
    for (unsigned int i=0; i < tmp.size();i++) {
        tmp[i]->addFlags( VARTYPE_FLAG_READONLY );
    }
    
    dcam_parameters->removeFlags( VARTYPE_FLAG_HIDE_CHILDREN );
    mutex.unlock();

    readAllParameterProperties();
    printf("CaptureV4L Info: Restoring Previously Saved Camera Parameters\n");
    writeAllParameterValues();
    readAllParameterValues();
    
    mutex.lock();
    camera_instance->captureWarm();
    
    //now we can allow upstream/external to capture
    is_capturing=true;

    mutex.unlock();

    return true;
}

bool CaptureV4L::copyFrame(const RawImage & src, RawImage & target)
{
    return convertFrame(src,target,src.getColorFormat());
}

bool CaptureV4L::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
    return convertFrame(src, target, Colors::stringToColorFormat(v_colorout->getSelection().c_str()));
}


bool CaptureV4L::convertFrame(const RawImage & src, RawImage & target, ColorFormat output_fmt, int y16bits)
{
    mutex.lock();
    ColorFormat src_fmt=src.getColorFormat();
    if (target.getData()==0) {
        //allocate target, if it does not exist yet
        target.allocate(output_fmt,src.getWidth(),src.getHeight());
    } else {
        target.ensure_allocation(output_fmt,src.getWidth(),src.getHeight());
        //target.setWidth(src.getWidth());
        //target.setHeight(src.getHeight());
        //target.setFormat(output_fmt);
    }
    target.setTime(src.getTime());
    if (output_fmt==src_fmt) {
        //just do a memcpy
        memcpy(target.getData(),src.getData(),src.getNumBytes());
    } else {
        //do some more fancy conversion
        /*if ((src_fmt==COLOR_MONO8 || src_fmt==COLOR_RAW8) && output_fmt==COLOR_RGB8) {
            Conversions::y2rgb (src.getData(), target.getData(), width, height);
        } else if ((src_fmt==COLOR_MONO16 || src_fmt==COLOR_RAW16)) {
            if (output_fmt==COLOR_RGB8) {
                Conversions::y162rgb (src.getData(), target.getData(), width, height, y16bits);
            }
            else {
                fprintf(stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",
                        Colors::colorFormatToString(src_fmt).c_str(),
                        Colors::colorFormatToString(output_fmt).c_str());
#ifndef VDATA_NO_QT
                mutex.unlock();
#endif
                return false;
            }
        } else */ if (src_fmt==COLOR_RGB8 && output_fmt==COLOR_YUV422_UYVY) {
            Conversions::rgb2uyvy (src.getData(), target.getData(), width, height);
        } else if (src_fmt==COLOR_RGB8 && output_fmt==COLOR_YUV422_YUYV) {
            Conversions::rgb2yuyv (src.getData(), target.getData(), width, height);
        } else {
            fprintf(stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",
                    Colors::colorFormatToString(src_fmt).c_str(),
                    Colors::colorFormatToString(output_fmt).c_str());
            mutex.unlock();
            return false;
        }
    }
    mutex.unlock();
    return true;
}

RawImage CaptureV4L::getFrame()
{
    mutex.lock();
    // capture a frame and write it
    rawFrame.ensure_allocation(capture_format, width, height);
    if (!camera_instance || !is_capturing || !camera_instance->captureFrame(&rawFrame, pixel_format)) {
        fprintf (stderr, "CaptureV4L Warning: Frame not ready, camera %d\n", cam_id);
        mutex.unlock();
        RawImage badImage;
        return badImage;
    }

#ifndef NDEBUG
    //strictly for debugging
    const char *szOutput = NULL;
    if (szOutput) {
        GlobalV4Linstance::writeRgbPPM(reinterpret_cast<GlobalV4Linstance::rgb*>(rawFrame.getData()),
                                       width, height, szOutput);
    }
#endif
    
    /*printf("B: %d w: %d h: %d bytes: %d pad: %d pos: %d %d depth: %d bpp %d coding: %d  behind %d id %d\n",frame->data_in_padding ? 1 : 0, frame->size[0],frame->size[1],frame->image_bytes,frame->padding_bytes, frame->position[0],frame->position[1],frame->data_depth,frame->packets_per_frame,frame->color_coding,frame->frames_behind,frame->id);*/

    mutex.unlock();
    return rawFrame;
}

void CaptureV4L::releaseFrame() {
    mutex.lock();
    
    //frame management done at low-level now...
    mutex.unlock();
}

string CaptureV4L::getCaptureMethodName() const {
    return "V4L";
}



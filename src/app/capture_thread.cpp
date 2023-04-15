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
  \file    capture_thread.cpp
  \brief   C++ Implementation: CaptureThread
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "capture_thread.h"
#include <capture_splitter.h>
#include <iostream>
#include <iomanip>

CaptureThread::CaptureThread(int cam_id)
{
  camId=cam_id;
  affinity=0;
  settings=new VarList("Image Capture");

  settings->addChild( (VarType*) (control= new VarList("Capture Control")));
  control->addChild( (VarType*) (c_start  = new VarTrigger("start capture","Start")));
  control->addChild( (VarType*) (c_stop   = new VarTrigger("stop capture","Stop")));
  control->addChild( (VarType*) (c_reset  = new VarTrigger("reset bus","Reset")));
  control->addChild( (VarType*) (c_auto_refresh= new VarBool("auto refresh params",true)));
  // timings should only be printed on demand for a short period of time by temporally activating this flag
  control->addChild( (VarType*) (c_print_timings = new VarBool("print timings",false)));
  control->addChild( (VarType*) (c_refresh= new VarTrigger("re-read params","Refresh")));
  control->addChild( (VarType*) (captureModule= new VarStringEnum("Capture Module",camId < 1 ? "Read from files" : "None")));
  captureModule->addFlags(VARTYPE_FLAG_NOLOAD_ENUM_CHILDREN);
  captureModule->addItem("None");
  captureModule->addItem("Read from files");
  captureModule->addItem("Generator");
  settings->addChild( (VarType*) (fromfile = new VarList("Read from files")));
  settings->addChild( (VarType*) (generator = new VarList("Generator")));
  settings->addFlags( VARTYPE_FLAG_AUTO_EXPAND_TREE );
  c_stop->addFlags( VARTYPE_FLAG_READONLY );
  c_refresh->addFlags( VARTYPE_FLAG_READONLY );
  connect(c_start,SIGNAL(wasEdited(VarType *)),this,SLOT(init()));
  connect(c_stop,SIGNAL(wasEdited(VarType *)),this,SLOT(stop()));
  connect(c_reset,SIGNAL(wasEdited(VarType *)),this,SLOT(reset()));
  connect(c_refresh,SIGNAL(wasEdited(VarType *)),this,SLOT(refresh()));
  connect(captureModule,SIGNAL(hasChanged(VarType *)),this,SLOT(selectCaptureMethod()));
  stack = 0;
  counter=new FrameCounter();
  capture=nullptr;
  captureFiles = new CaptureFromFile(fromfile, camId);
  captureGenerator = new CaptureGenerator(generator);

#ifdef DC1394
  captureModule->addItem("DC 1394");
  dc1394 = new VarList("DC1394");
  settings->addChild(dc1394);
  captureDC1394 = new CaptureDC1394v2(dc1394, camId);
#endif

#ifdef V4L
  captureModule->addItem("Video 4 Linux");
  v4l = new VarList("Video 4 Linux");
  settings->addChild(v4l);
  captureV4L = new CaptureV4L(v4l, camId);
#endif

#ifdef PYLON
  captureModule->addItem("Basler GigE");
  basler = new VarList("Basler GigE");
  settings->addChild(basler);
  captureBasler = new CaptureBasler(basler, camId);
#endif

#ifdef FLYCAP
  flycap = new VarList("Flycapture");
  captureModule->addItem("Flycapture");
  settings->addChild(flycap);
  captureFlycap = new CaptureFlycap(flycap, camId);
#endif

#ifdef MVIMPACT2
  captureModule->addItem("BlueFox2");
  bluefox2 = new VarList("BlueFox2");
  settings->addChild(bluefox2);
  captureBlueFox2 = new CaptureBlueFox2(bluefox2, camId);
#endif

#ifdef MVIMPACT3
  captureModule->addItem("BlueFox3");
  bluefox3 = new VarList("BlueFox3");
  settings->addChild(bluefox3);
  captureBlueFox3 = new CaptureBlueFox3(bluefox3, camId);
#endif

#ifdef SPINNAKER
  captureModule->addItem("Spinnaker");
  spinnaker = new VarList("Spinnaker");
  settings->addChild(spinnaker);
  captureSpinnaker = new CaptureSpinnaker(spinnaker, camId);
#endif

#ifdef CAMERA_SPLITTER
  splitter = new VarList("Splitter");
  captureModule->addItem("Splitter");
  settings->addChild(splitter);
  captureSplitter = new CaptureSplitter(splitter, camId);
#endif

  selectCaptureMethod();
  _kill =false;
  rb=0;
}

void CaptureThread::setAffinityManager(AffinityManager * _affinity) {
  affinity=_affinity;
}

void CaptureThread::setStack(VisionStack * _stack) {
  stack_mutex.lock();
  stack=_stack;
  stack_mutex.unlock();
}

VarList * CaptureThread::getSettings() {
  return settings;
}

CaptureThread::~CaptureThread()
{
  delete captureFiles;
  delete captureGenerator;
  delete counter;

#ifdef DC1394
  delete captureDC1394;
#endif

#ifdef V4L
  delete captureV4L;
#endif

#ifdef PYLON
  delete captureBasler;
#endif

#ifdef FLYCAP
  delete captureFlycap;
#endif

#ifdef MVIMPACT2
  delete captureBlueFox2;
#endif

#ifdef MVIMPACT3
  delete captureBlueFox3;
#endif

#ifdef SPINNAKER
  delete captureSpinnaker;
#endif

#ifdef CAMERA_SPLITTER
  delete captureSplitter;
#endif
}

void CaptureThread::setFrameBuffer(FrameBuffer * _rb) {
  rb=_rb;
}

FrameBuffer * CaptureThread::getFrameBuffer() const {
  return rb;
}

VisionStack * CaptureThread::getStack() const {
  return stack;
}

void CaptureThread::selectCaptureMethod() {
  capture_mutex.lock();
  CaptureInterface * old_capture=capture;
  CaptureInterface * new_capture=nullptr;
  if(captureModule->getString() == "Read from files") {
    new_capture = captureFiles;
  } else if(captureModule->getString() == "Generator") {
    new_capture = captureGenerator;
  }
#ifdef DC1394
  else if(captureModule->getString() == "DC 1394") {
    new_capture = captureDC1394;
  }
#endif
#ifdef V4L
  else if(captureModule->getString() == "Video 4 Linux") {
    new_capture = captureV4L;
  }
#endif
#ifdef FLYCAP
  else if(captureModule->getString() == "Flycapture") {
    new_capture = captureFlycap;
  }
#endif
#ifdef PYLON
  else if (captureModule->getString() == "Basler GigE") {
    new_capture = captureBasler;
  }
#endif
#ifdef MVIMPACT2
  else if(captureModule->getString() == "BlueFox2") {
    new_capture = captureBlueFox2;
  }
#endif
#ifdef MVIMPACT3
  else if(captureModule->getString() == "BlueFox3") {
    new_capture = captureBlueFox3;
  }
#endif
#ifdef SPINNAKER
  else if(captureModule->getString() == "Spinnaker") {
    new_capture = captureSpinnaker;
  }
#endif
#ifdef CAMERA_SPLITTER
  else if(captureModule->getString() == "Splitter") {
    new_capture = captureSplitter;
  }
#endif

  if (old_capture!=nullptr && new_capture!=old_capture && old_capture->isCapturing()) {
    capture_mutex.unlock();
    stop();
    capture_mutex.lock();
  }
  capture=new_capture;
  capture_mutex.unlock();
}

void CaptureThread::kill() {
 _kill=true;
  while(isRunning()) {
    usleep(100);
  }
}

bool CaptureThread::init() {
  capture_mutex.lock();
  bool res = (capture != nullptr) && capture->startCapture();
  if (res==true) {
    c_start->addFlags( VARTYPE_FLAG_READONLY );
    c_reset->addFlags( VARTYPE_FLAG_READONLY );
    c_refresh->removeFlags( VARTYPE_FLAG_READONLY );
    c_stop->removeFlags( VARTYPE_FLAG_READONLY );
  }
  capture_mutex.unlock();
  return res;
}

bool CaptureThread::stop() {
  capture_mutex.lock();
  bool res = (capture != nullptr) && capture->stopCapture();
  if (res==true) {
    c_stop->addFlags( VARTYPE_FLAG_READONLY );
    c_refresh->addFlags( VARTYPE_FLAG_READONLY );
    c_start->removeFlags( VARTYPE_FLAG_READONLY );
    c_reset->removeFlags( VARTYPE_FLAG_READONLY );
  }
  capture_mutex.unlock();
  return res;
}

bool CaptureThread::reset() {
  capture_mutex.lock();
  bool res = (capture != nullptr) && capture->resetBus();
  capture_mutex.unlock();
  return res;
}

void CaptureThread::refresh() {
  capture_mutex.lock();
  if(capture != nullptr) {
    capture->readAllParameterValues();
  }
  capture_mutex.unlock();
}


void CaptureThread::run() {
    CaptureStats * stats;
    bool changed;

    if (affinity!=0) {
      affinity->demandCore(camId);
    }

    while(true) {
      if (rb!=0) {
        int idx=rb->curWrite();
        FrameData * d=rb->getPointer(idx);
        if ((stats=(CaptureStats *)d->map.get("capture_stats")) == 0) {
          stats=(CaptureStats *)d->map.insert("capture_stats",new CaptureStats());
        }
        capture_mutex.lock();
        if ((capture != nullptr) && (capture->isCapturing())) {
          auto t_start = std::chrono::steady_clock::now();
          RawImage pic_raw=capture->getFrame();
          auto t_getFrame = std::chrono::steady_clock::now();
          pic_raw.setTime(GetTimeSec());
          d->time = pic_raw.getTime();
          d->time_cam=pic_raw.getTimeCam();
          bool bSuccess = capture->copyAndConvertFrame( pic_raw,d->video);
          auto t_convert = std::chrono::steady_clock::now();
          capture_mutex.unlock();

          if (bSuccess) {           //only on a good frame read do we proceed
              counter->count();
              stats->total=d->number=counter->getTotal();
              stats->fps_capture=counter->getFPS(changed);

              stack_mutex.lock();
              if (stack!=0) {
                stack->process(d);
                stack->postProcess(d);
              }
              stack_mutex.unlock();
              rb->nextWrite(true);

            auto t_process = std::chrono::steady_clock::now();

              if(c_print_timings->getBool())
              {
                auto getFrame_duration = std::chrono::duration_cast<std::chrono::microseconds>(t_getFrame - t_start);
                std::cout << std::setw(13) << std::left << "getFrame"
                          << std::setw(5) << std::right << getFrame_duration.count() << " μs" << std::endl;
                auto convert_duration = std::chrono::duration_cast<std::chrono::microseconds>(t_convert - t_getFrame);
                std::cout << std::setw(13) << std::left << "copy&convert"
                          << std::setw(5) << std::right << convert_duration.count() << " μs" << std::endl;
                auto process_duration = std::chrono::duration_cast<std::chrono::microseconds>(t_process - t_convert);
                std::cout << std::setw(13) << std::left << "process"
                          << std::setw(5) << std::right << process_duration.count() << " μs" << std::endl;
                auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(t_process - t_start);
                std::cout << std::setw(13) << std::left << "total"
                          << std::setw(5) << std::right << total_duration.count() << " μs" << std::endl << std::endl;
              }

              if (changed) {
                if (c_auto_refresh->getBool()==true) {
                  capture_mutex.lock();
                  if ((capture != 0) && (capture->isCapturing())) capture->readAllParameterValues();
                  capture_mutex.unlock();
                }
                stack_mutex.lock();
                stack->updateTimingStatistics();
                stack_mutex.unlock();
              }
          }

          capture_mutex.lock();
          if ((capture != nullptr) && (capture->isCapturing())) {
            capture->releaseFrame();
          }
          capture_mutex.unlock();
        } else {
          stats->total=d->number=counter->getTotal();
          stats->fps_capture=counter->getFPS(changed);
          //we are not capturing...chill this thread out...
          capture_mutex.unlock();
          usleep(5000);
        }
        if (_kill) {
          capture_mutex.lock();
          if(capture != nullptr) {
            capture->stopCapture();
            //make sure to read latest params from camera to be saved to file...
            if (capture->isCapturing()) capture->readAllParameterValues();
          }
          capture_mutex.unlock();
          return;
        }
      }
    }
}

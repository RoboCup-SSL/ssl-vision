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

CaptureThread::CaptureThread(int cam_id)
{
  camId=cam_id;
  settings=new VarList("Image Capture");

  settings->addChild( (VarData*) (control= new VarList("Capture Control")));
  control->addChild( (VarData*) (c_start  = new VarTrigger("start capture","Start")));
  control->addChild( (VarData*) (c_stop   = new VarTrigger("stop capture","Stop")));
  control->addChild( (VarData*) (c_reset  = new VarTrigger("reset bus","Reset")));
  control->addChild( (VarData*) (c_auto_refresh= new VarBool("auto refresh params",true)));
  control->addChild( (VarData*) (c_refresh= new VarTrigger("re-read params","Refresh")));
  control->addChild( (VarData*) (captureModule= new VarStringEnum("Capture Module","DC 1394")));
  captureModule->addItem("DC 1394");
  captureModule->addItem("Read from files");
  settings->addChild( (VarData*) (dc1394 = new VarList("DC1394")));
  settings->addChild( (VarData*) (fromfile = new VarList("Read from files")));
  settings->addRenderFlags( DT_FLAG_AUTO_EXPAND_TREE );
  c_stop->addRenderFlags( DT_FLAG_READONLY );
  c_refresh->addRenderFlags( DT_FLAG_READONLY );
  connect(c_start,SIGNAL(wasEdited(VarData *)),this,SLOT(init()));
  connect(c_stop,SIGNAL(wasEdited(VarData *)),this,SLOT(stop()));
  connect(c_reset,SIGNAL(wasEdited(VarData *)),this,SLOT(reset()));
  connect(c_refresh,SIGNAL(wasEdited(VarData *)),this,SLOT(refresh()));

  stack = 0;
  counter=new FrameCounter();
  capture = 0;
  captureDC1394 = new CaptureDC1394v2(dc1394,camId);
  captureFiles = new CaptureFromFile(fromfile);
  _kill =false;
  rb=0;
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
  delete captureDC1394;
  delete captureFiles;
  delete counter;
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

void CaptureThread::kill() {
 _kill=true; 
  while(isRunning()) {
    usleep(100);
  }
}

bool CaptureThread::init() {
  capture_mutex.lock();
  if(captureModule->getString() == "Read from files")
    capture = captureFiles;
  else
    capture = captureDC1394;
  bool res = capture->startCapture();
  if (res==true) {
    c_start->addRenderFlags( DT_FLAG_READONLY );
    c_reset->addRenderFlags( DT_FLAG_READONLY );
    c_refresh->removeRenderFlags( DT_FLAG_READONLY );
    c_stop->removeRenderFlags( DT_FLAG_READONLY );
  }
  capture_mutex.unlock();
  return res;
}

bool CaptureThread::stop() {
  capture_mutex.lock();
  bool res = capture->stopCapture();
  if (res==true) {
    c_stop->addRenderFlags( DT_FLAG_READONLY );
    c_refresh->addRenderFlags( DT_FLAG_READONLY );
    c_start->removeRenderFlags( DT_FLAG_READONLY );
    c_reset->removeRenderFlags( DT_FLAG_READONLY );
  }
  capture_mutex.unlock();
  return res;
}

bool CaptureThread::reset() {
  capture_mutex.lock();
  bool res = capture->resetBus();
  capture_mutex.unlock();
  return res;
}

void CaptureThread::refresh() {
  capture_mutex.lock();
  capture->readAllParameterValues();
  capture_mutex.unlock();
}


void CaptureThread::run() {
    CaptureStats * stats;
    bool changed;

    while(true) {
      if (rb!=0) {
        int idx=rb->curWrite();
        FrameData * d=rb->getPointer(idx);
        if ((stats=(CaptureStats *)d->map.get("capture_stats")) == 0) {
          stats=(CaptureStats *)d->map.insert("capture_stats",new CaptureStats());
        }
        capture_mutex.lock();
        if ((capture != 0) && (capture->isCapturing())) {
          RawImage pic_raw=capture->getFrame();
          capture->copyAndConvertFrame( pic_raw,d->video);
          capture_mutex.unlock();

          counter->count();
          stats->total=d->number=counter->getTotal();
          stats->fps_capture=counter->getFPS(changed);

          stack_mutex.lock();
          if (stack!=0) {
            stack->process(d);
            stack->postProcess(d);
            stack_mutex.unlock();
            rb->nextWrite(true);
          } else {
            stack_mutex.unlock();
            rb->nextWrite(true);
          }

          if (changed) {
            if (c_auto_refresh->getBool()==true) capture->readAllParameterValues();
            stack_mutex.lock();
            stack->updateTimingStatistics();
            stack_mutex.unlock();
          }
          capture_mutex.lock();
          capture->releaseFrame();
          capture_mutex.unlock();

        } else {
          stats->total=d->number=counter->getTotal();
          stats->fps_capture=counter->getFPS(changed);
          //we are not capturing...chill this thread out...
          capture_mutex.unlock();
          usleep(100);
        }
        if (_kill) {
          if(capture != 0)
            capture->stopCapture();
          return;
        }
      }
    }
}

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
  \file    capture_thread.h
  \brief   C++ Interface: CaptureThread
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H
#ifdef DC1394
#include "capturedc1394v2.h"
#endif
#include "capturefromfile.h"
#include "capturev4l.h"
#include "capture_generator.h"
#include "capture_splitter.h"
#include <QThread>
#include "ringbuffer.h"
#include "framedata.h"
#include "framecounter.h"
#include "visionstack.h"
#include "capturestats.h"
#include "affinity_manager.h"

#ifdef MVIMPACT2
#include "capture_bluefox2.h"
#endif

#ifdef MVIMPACT3
#include "capture_bluefox3.h"
#endif

#ifdef FLYCAP
#include "capture_flycap.h"
#endif

#ifdef PYLON
#include "capture_basler.h"
#endif

#ifdef SPINNAKER
#include "capture_spinnaker.h"
#endif

/*!
  \class   CaptureThread
  \brief   A thread for capturing and processing video data
  \author  Stefan Zickler, (C) 2008
*/
class CaptureThread : public QThread
{
Q_OBJECT
protected:
  QMutex stack_mutex; //this mutex protects multi-threaded operations on the stack
  QMutex capture_mutex; //this mutex protects multi-threaded operations on the capture control
  VisionStack * stack;
  FrameCounter * counter;
  CaptureInterface * capture = nullptr;
  CaptureInterface * captureDC1394 = nullptr;
  CaptureInterface * captureV4L = nullptr;
  CaptureInterface * captureBlueFox2 = nullptr;
  CaptureInterface * captureBlueFox3 = nullptr;
  CaptureInterface * captureFlycap = nullptr;
  CaptureInterface * captureFiles = nullptr;
  CaptureInterface * captureGenerator = nullptr;
  CaptureInterface * captureBasler = nullptr;
  CaptureInterface * captureSpinnaker = nullptr;
  CaptureInterface * captureSplitter = nullptr;
  AffinityManager * affinity;
  FrameBuffer * rb;
  bool _kill;
  int camId;
  VarList * settings;
  VarList * dc1394 = nullptr;
  VarList * v4l = nullptr;
  VarList * bluefox2 = nullptr;
  VarList * bluefox3 = nullptr;
  VarList * flycap = nullptr;
  VarList * generator = nullptr;
  VarList * fromfile = nullptr;
  VarList * basler = nullptr;
  VarList * spinnaker = nullptr;
  VarList * splitter = nullptr;
  VarList * control;
  VarTrigger * c_start;
  VarTrigger * c_stop;
  VarTrigger * c_reset;
  VarTrigger * c_refresh;
  VarBool * c_auto_refresh;
  VarBool * c_print_timings;
  VarStringEnum * captureModule;

public slots:
  bool init();
  bool stop();
  bool reset();
  void refresh();
  void selectCaptureMethod();

public:
  void setFrameBuffer(FrameBuffer * _rb);
  FrameBuffer * getFrameBuffer() const;
  void setStack(VisionStack * _stack);
  VisionStack * getStack() const;
  void kill();
  VarList * getSettings();
  void setAffinityManager(AffinityManager * _affinity);
  CaptureInterface* getCaptureSplitter() {return captureSplitter;};
  CaptureThread(int cam_id);
  ~CaptureThread();

  virtual void run();

};








#endif

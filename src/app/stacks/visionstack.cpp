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
  \file    visionstack.h
  \brief   C++ Interface: VisionStack
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#include "visionstack.h"

VisionStack::VisionStack(string _name, RenderOptions * _opts) {
  name=_name;
  opts=_opts;
  //counter_proc=0.0;
  //counter_post_proc=0.0;
  settings=new VarList("Global");
}

VisionStack::~VisionStack() {
  delete settings;
}

string VisionStack::getName() {
  return name;
}

VarList * VisionStack::getSettings() {
  return settings;
}

string VisionStack::getSettingsFileName() {
  //this can be done dynamically based on camera-id's etc...
  return name;
}

void VisionStack::process(FrameData * data) {
  double a=0.0;
  double b=0.0;
  unsigned int n=stack.size();
  VisionPlugin * p;
  double total=0.0;
  bool show_timing = false;
  if (show_timing) printf("----------\n");
  for (unsigned int i=0;i<n;i++) {
    p=stack[i];
    p->lock();
    a=GetTimeSec();
    p->process(data,opts);
    b=GetTimeSec();
    p->setTimeProcessing(b-a);
    total+=(p->getTimeProcessing());
    if (show_timing) {
      printf("Plugin %s: %fms\n",p->getName().c_str(),  p->getTimeProcessing() * 1000.0);
    }
    p->unlock();
  }
  if (show_timing) printf("Total time: %fms\n",total * 1000.0);
  //counter_proc+=1.0;
}

void VisionStack::postProcess(FrameData * data) {
  unsigned int n=stack.size();
  double a=0.0;
  double b=0.0;
  VisionPlugin * p;
  for (unsigned int i=0;i<n;i++) {
    p=stack[i];
    p->lock();
    a=GetTimeSec();
    p->postProcess(data,opts);
    b=GetTimeSec();
    p->setTimePostProcessing(b-a);
    p->unlock();
  }
  //counter_post_proc+=1.0;
}

void VisionStack::updateTimingStatistics() {

}

void VisionStack::keyPressEvent ( QKeyEvent * event ) {
  unsigned int n=stack.size();
  VisionPlugin * p;
  for (unsigned int i=0;i<n;i++) {
    p=stack[i];
    p->lock();
    p->keyPressEvent(event);
    p->unlock();
    if (event->isAccepted()) return;
  }
}

void VisionStack::mousePressEvent ( QMouseEvent * event, pixelloc loc ) {
  unsigned int n=stack.size();
  VisionPlugin * p;
  for (unsigned int i=0;i<n;i++) {
    p=stack[i];
    p->lock();
    p->mousePressEvent(event, loc);
    p->unlock();
    if (event->isAccepted()) return;
  }
}

void VisionStack::mouseReleaseEvent ( QMouseEvent * event, pixelloc loc ) {
  unsigned int n=stack.size();
  VisionPlugin * p;
  for (unsigned int i=0;i<n;i++) {
    p=stack[i];
    p->lock();
    p->mouseReleaseEvent(event,loc);
    p->unlock();
    if (event->isAccepted()) return;
  }
}

void VisionStack::mouseMoveEvent ( QMouseEvent * event, pixelloc loc ) {
  unsigned int n=stack.size();
  VisionPlugin * p;
  for (unsigned int i=0;i<n;i++) {
    p=stack[i];
    p->lock();
    p->mouseMoveEvent(event,loc);
    p->unlock();
    if (event->isAccepted()) return;
  }
}

void VisionStack::wheelEvent ( QWheelEvent * event, pixelloc loc ) {
  unsigned int n=stack.size();
  VisionPlugin * p;
  for (unsigned int i=0;i<n;i++) {
    p=stack[i];
    p->lock();
    p->wheelEvent(event,loc);
    p->unlock();
    if (event->isAccepted()) return;
  }
}

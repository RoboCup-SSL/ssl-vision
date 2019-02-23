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

VisionStack::VisionStack(RenderOptions * _opts) {
  opts=_opts;
  settings=new VarList("Global");
}

VisionStack::~VisionStack() {
  delete settings;
}

VarList * VisionStack::getSettings() {
  return settings;
}

void VisionStack::process(FrameData * data) {
  for (auto p : stack) {
    p->lock();
    p->process(data,opts);
    p->unlock();
  }
}

void VisionStack::postProcess(FrameData * data) {
  for (auto p : stack) {
    p->lock();
    p->postProcess(data,opts);
    p->unlock();
  }
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

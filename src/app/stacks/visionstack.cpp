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
#include <iomanip>
#include <iostream>
#include <chrono>

VisionStack::VisionStack(RenderOptions * _opts) {
  opts=_opts;
  settings=new VarList("Global");
  // timings should only be printed on demand for a short period of time by temporally activating this flag
  _v_print_timings = new VarBool("print stack timings", false);
  settings->addChild(_v_print_timings);
}

VisionStack::~VisionStack() {
  delete settings;
}

VarList * VisionStack::getSettings() {
  return settings;
}

void VisionStack::process(FrameData * data) {
  if(_v_print_timings->getBool()) {
    auto totalStart = std::chrono::steady_clock::now();
    for (auto p : stack) {
      p->lock();
      auto start = std::chrono::steady_clock::now();
      p->process(data,opts);
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
      std::cout << std::setw(23) << std::left << p->getName()
                << std::setw(5) << std::right << duration.count() << " μs" << std::endl;
      p->unlock();
    }
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - totalStart);
    std::cout << std::setw(23) << std::left << "All"
              << std::setw(5) << std::right << duration.count() << " μs" << std::endl << std::endl;
  } else {
    for (auto p : stack) {
      p->lock();
      p->process(data,opts);
      p->unlock();
    }
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

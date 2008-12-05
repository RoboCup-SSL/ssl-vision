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
  \file    visionplugin.cpp
  \brief   C++ Implementation: VisionPlugin
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "visionplugin.h"

VisionPlugin::VisionPlugin(FrameBuffer * _buffer)
{
  buffer=_buffer;
  enabled=true;
  shared=false;
  visualize=true;
  setTimeProcessing(0.0);
  setTimePostProcessing(0.0);
}

VisionPlugin::~VisionPlugin()
{
  closeWidgets();
}

string VisionPlugin::getName() {
  return "Unnamed";
}

FrameBuffer * VisionPlugin::getFrameBuffer() const {
  return buffer;
}

void VisionPlugin::lock() {
  mutex.lock();
}

void VisionPlugin::unlock() {
  mutex.unlock();
}

bool VisionPlugin::isEnabled() const {
  return enabled;
}

void VisionPlugin::setEnabled(bool enable) {
  enabled=enable;
}

bool VisionPlugin::isVisualize() const {
  return visualize;
}

void VisionPlugin::setVisualize(bool enable) {
  visualize=enable;
}

bool VisionPlugin::isSharedAmongStacks() const {
  return shared;
}

void VisionPlugin::setSharedAmongStacks(bool enable) {
  shared=enable;
}

void VisionPlugin::displayLoopEvent(bool frame_changed, RenderOptions * opts) {
  (void)frame_changed;
  (void)opts;
}

VarList * VisionPlugin::getSettings() {
  return 0;
}

ProcessResult VisionPlugin::process(FrameData * data, RenderOptions * options) {
  (void)data;
  (void)options;
  return ProcessingOk;
}


void VisionPlugin::postProcess(FrameData * data, RenderOptions * options) {
  (void)data;
  (void)options;
}

QWidget * VisionPlugin::getVisualizationWidget() {
  return 0;
}

QWidget * VisionPlugin::getControlWidget() {
  return 0;
}

void VisionPlugin::closeWidgets() {
}

QList<QAction *> VisionPlugin::actions() {
  QList<QAction *> a;
  return a;
}

void VisionPlugin::keyPressEvent ( QKeyEvent * event) {
  (void)event;
}

void VisionPlugin::mousePressEvent ( QMouseEvent * event, pixelloc loc ) {
  (void)event;
  (void)loc;
}

void VisionPlugin::mouseReleaseEvent ( QMouseEvent * event, pixelloc loc ) {
  (void)event;
  (void)loc;
}

void VisionPlugin::mouseMoveEvent ( QMouseEvent * event, pixelloc loc ) {
  (void)event;
  (void)loc;
}

void VisionPlugin::wheelEvent ( QWheelEvent * event, pixelloc loc ) {
  (void)event;
  (void)loc;
}

void VisionPlugin::setTimeProcessing(double val) {
  time_proc=val;
}

void VisionPlugin::setTimePostProcessing(double val) {
  time_post=val;
}

double VisionPlugin::getTimeProcessing() {
  return time_proc;
}

double VisionPlugin::getTimePostProcessing() {
  return time_post;
}

void VisionPlugin::slotKeyPressEvent ( QKeyEvent * event ) {
  keyPressEvent(event);
}


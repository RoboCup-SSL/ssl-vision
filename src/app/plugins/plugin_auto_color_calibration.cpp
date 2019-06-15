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
  \file    plugin_auto_color_calibration.cpp
  \brief   C++ Implementation: PluginAutoColorCalibration
  \author  Nicolai Ommer <nicolai.ommer@gmail.com>, (C) 2016
           Mark Geiger <markgeiger@posteo.de>, (C) 2017
*/
//========================================================================

#include "plugin_auto_color_calibration.h"
#include <opencv2/opencv.hpp>
#include <gui/automatedcolorcalibwidget.h>

PluginAutoColorCalibration::PluginAutoColorCalibration(
        FrameBuffer *_buffer,
        YUVLUT *lut,
        LUTWidget *_lutw)
        :
        VisionPlugin(_buffer),
        global_lut(lut),
        lutw(_lutw) {

  settings = new VarList("Auto Color Calibration");
  v_calibration_points = new VarList("Calibration Points");
  settings->addChild(v_calibration_points);
  settings->addChild(initialColorCalibrator.maxColorDist);
  settings->addChild(initialColorCalibrator.maxAngle);
  settings->addChild(initialColorCalibrator.weights);

  // get mouse events
  this->installEventFilter(this);
}

PluginAutoColorCalibration::~PluginAutoColorCalibration() {
  delete settings;
}

QWidget *PluginAutoColorCalibration::getControlWidget() {
  if (accw == nullptr) {
    accw = new AutomatedColorCalibWidget(global_lut);
    accw->set_status("Samples Captured: " + QString::number(v_calibration_points->getChildrenCount()));
  }

  return (QWidget *) accw;
}


ProcessResult PluginAutoColorCalibration::process(FrameData *frame, RenderOptions *options) {
  (void) options;
  if (frame == nullptr) {
    return ProcessingFailed;
  }

  // handle GUI commands here
  process_gui_commands();

  // run initial calibration
  if (initial_calibration_running) {
    std::vector<ColorClazz> colors;
    for (auto p : v_calibration_points->getChildren()) {
      yuv color;
      int channel = 0;
      for (auto *c : p->getChildren()) {
        if (c->getName() == "y") color.y = ((VarInt *) c)->getInt();
        if (c->getName() == "u") color.u = ((VarInt *) c)->getInt();
        if (c->getName() == "v") color.v = ((VarInt *) c)->getInt();
        if (c->getName() == "channel") channel = ((VarInt *) c)->getInt();
      }
      colors.emplace_back(color, channel);
    }

    initialColorCalibrator.process(colors, global_lut);
    lutw->getGLLUTWidget()->needs_init = true;
    lutw->getGLLUTWidget()->repaint();

    processed_frames++;
    if (processed_frames > 5) {
      initial_calibration_running = false;
    }
  }

  return ProcessingOk;
}

VarList *PluginAutoColorCalibration::getSettings() {
  return settings;
}

string PluginAutoColorCalibration::getName() {
  return "Auto Color Calibration";
}

void PluginAutoColorCalibration::process_gui_commands() {
  if (accw == nullptr) {
    return;
  }

  if (accw->pending_reset_lut) {
    accw->pending_reset_lut = false;
    global_lut->reset();
    global_lut->updateDerivedLUTs();
    lutw->getGLLUTWidget()->needs_init = true;
    lutw->getGLLUTWidget()->repaint();
  }

  if (accw->pending_reset) {
    accw->pending_reset = false;
    clearCalibrationPoints();
  }

  if (accw->pending_initialize) {
    accw->pending_initialize = false;
    processed_frames = 0;
    initial_calibration_running = true;
    accw->set_status("Triggered initial calibration");
  }
}


void PluginAutoColorCalibration::mouseEvent(QMouseEvent *event, pixelloc loc) {
  auto *tabw = (QTabWidget *) accw->parentWidget()->parentWidget();
  if (tabw->currentWidget() == accw
      && event->buttons() == Qt::LeftButton && accw->currentChannel != -1) {
    FrameBuffer *rb = getFrameBuffer();
    if (rb != nullptr) {
      rb->lockRead();
      int idx = rb->curRead();
      FrameData *frame = rb->getPointer(idx);
      if (loc.x < frame->video.getWidth()
          && loc.y < frame->video.getHeight()
          && loc.x >= 0
          && loc.y >= 0
          && frame->video.getWidth() > 1
          && frame->video.getHeight() > 1) {
        yuv color = frame->video.getYuv(loc.x, loc.y);
        addCalibrationPoint(color, accw->currentChannel);
      }
      rb->unlockRead();
    }
    event->accept();
  } else {
    event->ignore();
  }
}

void PluginAutoColorCalibration::addCalibrationPoint(const yuv &color, int channel) {

  VarList *v_point = new VarList("Sample");
  v_point->addChild(new VarInt("y", color.y, 0, 255));
  v_point->addChild(new VarInt("u", color.u, 0, 255));
  v_point->addChild(new VarInt("v", color.v, 0, 255));
  v_point->addChild(new VarInt("channel", channel, 0));
  v_calibration_points->addChild(v_point);

  accw->set_status("Samples Captured: " + QString::number(v_calibration_points->getChildrenCount()));
}

void PluginAutoColorCalibration::clearCalibrationPoints() {
  for (auto sample : v_calibration_points->getChildren()) {
    v_calibration_points->removeChild(sample);
    delete sample;
  }
}

void PluginAutoColorCalibration::mousePressEvent(QMouseEvent *event, pixelloc loc) {
  mouseEvent(event, loc);
}

void PluginAutoColorCalibration::mouseReleaseEvent(QMouseEvent *event, pixelloc loc) {
  mouseEvent(event, loc);
}

void PluginAutoColorCalibration::mouseMoveEvent(QMouseEvent *event, pixelloc loc) {
  (void) loc;
  event->ignore();
}
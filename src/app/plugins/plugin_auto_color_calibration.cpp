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

  v_maxDistance = new VarDouble("Max distance", 3000, 0);
  v_maxAngle = new VarDouble("Max angle", 0.3, 0, 3.14);

  v_weights = new VarList("Weights");
  for (int i = 0; i < lut->getChannelCount(); i++) {
    v_weights->addChild(createWeight(lut->getChannel(i).label, i));
  }

  auto *v_defaults = new VarList("Defaults");
  v_defaults->addChild(v_weights);
  v_defaults->addChild(v_maxDistance);
  v_defaults->addChild(v_maxAngle);
  settings->addChild(v_defaults);

  // get mouse events
  this->installEventFilter(this);
}

VarDouble *PluginAutoColorCalibration::createWeight(const std::string &colorName, int channel) {
  VarDouble *v_weight = new VarDouble(colorName, 1, 0, 10);
  weightMap[channel] = v_weight;
  return v_weight;
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

  process_gui_commands();

  return ProcessingOk;
}

void PluginAutoColorCalibration::runCalibration() {
  vector<ColorClazz> colors;
  for (auto p : v_calibration_points->getChildren()) {
    bool use = true;
    yuv color;
    int channel = 0;
    float maxAngle = 0;
    float maxDistance = 0;
    float weight = 0;
    for (auto *c : p->getChildren()) {
      if (c->getName() == "use") use = ((VarBool *) c)->getBool();
      if (c->getName() == "y") color.y = ((VarInt *) c)->getInt();
      if (c->getName() == "u") color.u = ((VarInt *) c)->getInt();
      if (c->getName() == "v") color.v = ((VarInt *) c)->getInt();
      if (c->getName() == "channel") channel = ((VarInt *) c)->getInt();
      if (c->getName() == "maxAngle") maxAngle = ((VarDouble *) c)->getDouble();
      if (c->getName() == "maxDistance") maxDistance = ((VarDouble *) c)->getDouble();
      if (c->getName() == "weight") weight = ((VarDouble *) c)->getDouble();
    }
    if (use) colors.emplace_back(color, channel, weight, maxDistance, maxAngle);
  }

  initialColorCalibrator.process(colors, global_lut);
  lutw->getGLLUTWidget()->needs_init = true;
  lutw->getGLLUTWidget()->repaint();
}

VarList *PluginAutoColorCalibration::getSettings() {
  return settings;
}

string PluginAutoColorCalibration::getName() {
  return "Auto Color Calibration";
}

void PluginAutoColorCalibration::process_gui_commands() {

  removeMarkedCalibrationPoints();

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

  if (accw->pending_update) {
    accw->pending_update = false;
    runCalibration();
    accw->set_status("Triggered calibration");
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
  VarList *v_point = new VarList(global_lut->getChannel(channel).label + " sample");

  VarInt *v_channel = new VarInt("channel", channel, 0);
  v_channel->setFlags(VARTYPE_FLAG_READONLY);

  v_point->addChild(new VarBool("use", true));
  v_point->addChild(new VarInt("y", color.y, 0, 255));
  v_point->addChild(new VarInt("u", color.u, 0, 255));
  v_point->addChild(new VarInt("v", color.v, 0, 255));
  v_point->addChild(v_channel);
  v_point->addChild(new VarDouble("maxAngle", v_maxAngle->getDouble(), 0.01, 1.57));
  v_point->addChild(new VarDouble("maxDistance", v_maxDistance->getDouble(), 0.0, 10000));
  v_point->addChild(new VarDouble("weight", getWeight(channel), 0.0, 10));
  v_point->addChild(new VarTrigger("Remove"));
  v_calibration_points->addChild(v_point);

  accw->set_status("Samples Captured: " + QString::number(v_calibration_points->getChildrenCount()));
}

void PluginAutoColorCalibration::removeMarkedCalibrationPoints() {
  for (VarType *v_child : v_calibration_points->getChildren()) {
    auto *v_point = (VarList *) v_child;
    for (auto *c : v_point->getChildren()) {
      if (c->getName() == "Remove") {
        auto *v_remove = (VarTrigger *) c;
        if (v_remove->getAndResetCounter() > 0) {
          for (auto *ch : v_point->getChildren()) {
            v_point->removeChild(ch);
          }
          v_calibration_points->removeChild(v_child);
          return;
        }
        break;
      }
    }
  }
}

float PluginAutoColorCalibration::getWeight(int channel) {
  VarDouble *v_weight = weightMap[channel];
  float weight = 1;
  if (v_weight != nullptr) {
    weight = v_weight->getDouble();
  }
  return weight;
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
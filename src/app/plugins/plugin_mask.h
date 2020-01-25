#pragma once

#include "colors.h"
#include "convex_hull_image_mask.h"
#include "framedata.h"
#include "glLUTwidget.h"
#include "gvector.h"
#include "image.h"
#include "lutwidget.h"
#include "maskwidget.h"
#include "visionplugin.h"
#include <algorithm>
#include <vector>

class PluginMask : public VisionPlugin {
protected:
  MaskWidget *_widget;
  VarList *_settings;
  ConvexHullImageMask &_mask;

  virtual void _mouseEvent(QMouseEvent *event, pixelloc loc);
  virtual void _addPoint(int x, int y);
  virtual void _removePoint(int x, int y);

public:
  PluginMask(FrameBuffer *buffer, ConvexHullImageMask &mask);
  ~PluginMask() override;
  ProcessResult process(FrameData *data, RenderOptions *options) override;
  VarList *getSettings() override;
  std::string getName() override;
  QWidget *getControlWidget() override;
  void mousePressEvent(QMouseEvent *event, pixelloc loc) override;
};

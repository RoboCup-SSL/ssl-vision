#ifndef PLUGIN_MASK_H
#define PLUGIN_MASK_H

#include "colors.h"
#include "convex_hull_image_mask.h"
#include "framedata.h"
#include "gvector.h"
#include "image.h"
#include "maskwidget.h"
#include "visionplugin.h"
#include <algorithm>
#include "glLUTwidget.h"
#include "lutwidget.h"
#include <vector>

class PluginMask : public VisionPlugin
{
 protected:
  LUTWidget *_widget;
  VarList *_settings;
  VarBool *_v_enable;
  ConvexHullImageMask &_mask;

  virtual void _mouseEvent(QMouseEvent *event, const pixelloc loc);
  virtual void _addPoint(const int x, const int y);
  virtual void _removePoint(const int x, const int y);
 public:
  PluginMask(FrameBuffer *buffer, ConvexHullImageMask &mask, LUTWidget *widget);
  ~PluginMask();
  virtual ProcessResult process(FrameData *data, RenderOptions *options);
  virtual VarList *getSettings();
  virtual std::string getName();
  virtual void mousePressEvent(QMouseEvent *event, pixelloc loc);
};

#endif // PLUGIN_MASK_H

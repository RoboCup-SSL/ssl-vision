#ifndef PLUGIN_MASK_H
#define PLUGIN_MASK_H

#include <vector>
#include <algorithm>
#include "framedata.h"
#include "visionplugin.h"
#include "maskwidget.h"
#include "colors.h"
#include "image.h"
#include "gvector.h"
#include "convex_hull_image_mask.h"

class PluginMask : public VisionPlugin
{
 protected:
  MaskWidget *_widget;
  VarList *_settings;
  VarBool *_v_enable;
  ConvexHullImageMask &_mask;

  virtual void _mouseEvent(QMouseEvent *event, const pixelloc loc);
  virtual void _addPoint(const int x, const int y);
  virtual void _removePoint(const int x, const int y);
 public:
  PluginMask(FrameBuffer *buffer, ConvexHullImageMask &mask);
  ~PluginMask();
  virtual ProcessResult process(FrameData *data, RenderOptions *options);
  virtual VarList *getSettings();
  virtual std::string getName();
  virtual QWidget *getControlWidget();
  virtual void mousePressEvent(QMouseEvent *event, pixelloc loc);
};

#endif // PLUGIN_MASK_H

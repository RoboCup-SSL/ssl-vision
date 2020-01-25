#include "plugin_mask.h"

#include <iostream>

PluginMask::PluginMask(FrameBuffer *buffer, ConvexHullImageMask &mask)
    : VisionPlugin(buffer), _mask(mask) {

  _settings = new VarList("Image Mask");
  _widget = new MaskWidget();
}

PluginMask::~PluginMask() { delete _settings; }

QWidget *PluginMask::getControlWidget() { return (QWidget *)_widget; }

VarList *PluginMask::getSettings() { return _settings; }

std::string PluginMask::getName() { return "Mask"; }

ProcessResult PluginMask::process(FrameData *data, RenderOptions *options) {
  (void)options;
  // We can only allocate _mask once we get the first frame.
  // Until then we do not know the size of the image.
  if (_mask.getNumPixels() != data->video.getNumPixels())
    _mask.setSize(data->video.getWidth(), data->video.getHeight());

  if (_widget->clear_mask_button->isChecked()) {
    _mask.reset();
    _widget->clear_mask_button->setChecked(false);
  }

  return ProcessingOk;
}

void PluginMask::_addPoint(const int x, const int y) { _mask.addPoint(x, y); }

void PluginMask::_removePoint(const int x, const int y) {
  _mask.removePoint(x, y, 5);
}

void PluginMask::_mouseEvent(QMouseEvent *event, const pixelloc loc) {
  auto tabw = (QTabWidget*) _widget->parentWidget()->parentWidget();
  if (tabw->currentWidget() != _widget) {
    event->ignore();
    return;
  }

  FrameBuffer *fb = getFrameBuffer();
  if (!fb)
    return;

  if (event->buttons() == Qt::LeftButton) {
    event->accept();
    fb->lockRead();

    int fb_idx = fb->curRead();
    FrameData *frame = fb->getPointer(fb_idx);

    const int video_width = frame->video.getWidth();
    const int video_height = frame->video.getHeight();

    int x = loc.x;
    int y = loc.y;

    // clean the click location
    if (x < 0)
      x = 0;
    else if (x >= video_width)
      x = video_width - 1;

    if (y < 0)
      y = 0;
    else if (y >= video_height)
      y = video_height - 1;

    if (event->modifiers() == Qt::ShiftModifier)
      _removePoint(x, y);
    else
      _addPoint(x, y);

    fb->unlockRead();
  }
}

void PluginMask::mousePressEvent(QMouseEvent *event, pixelloc loc) {
  _mouseEvent(event, loc);
}

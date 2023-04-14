#include "convex_hull_image_mask.h"
#include <queue>
#include <tuple>
#include <iostream>

void computeMask(const ConvexHull &convex_hull, Image<raw8> &mask) {
  const auto WHITE = raw8(255);

  if(convex_hull._points.empty()) {
    mask.fillColor(WHITE);
    return;
  }

  mask.fillBlack();
  for (auto it = convex_hull.begin(); it != convex_hull.end(); ++it) {
    const GVector::vector2d<int> &a = *it;
    const GVector::vector2d<int> &b =
      std::next(it) != convex_hull.end() ? *std::next(it) : *convex_hull.begin();
    mask.drawLine(a.x, a.y, b.x, b.y, WHITE);
  }

  if (convex_hull.getNumPoints() < 3)
    return;

  // linescan, top - bot, left - right
  // first find where the whites should be painted, and then paint them
  for (int y = 0; y < mask.getHeight(); ++y) {
    int minX = mask.getHeight();  // never min
    int maxX = 0;
    for (int x = 0; x < mask.getWidth(); ++x) {
      if (mask.getPixel(x, y) == WHITE) {
        minX = minX <= x ? minX : x;
        maxX = x;
      }
    }
    if (minX == mask.getHeight() ||  // no whites
	minX == maxX)                // single white
      continue;

    assert(minX >= 0 && minX < mask.getWidth() && maxX >= 0 && maxX < mask.getWidth());
    for (int x = minX; x < maxX + 1; ++x)
      mask.setPixel(x, y, WHITE);
  }
}

void ConvexHullImageMask::slotMaskPointsRead() {
  lock();

  VarTypes::VarInt *x;
  VarTypes::VarInt *y;
  std::vector<VarTypes::VarType *> mask_points = _v_list->getChildren();
  for (VarTypes::VarType *p : mask_points) {
    VarTypes::VarList *p_list = static_cast<VarTypes::VarList*>(p);
    x = static_cast<VarTypes::VarInt *>(p_list->findChild("x"));
    y = static_cast<VarTypes::VarInt *>(p_list->findChild("y"));
    _addPoint(x->getInt(), y->getInt(), false);
  }

  unlock();
}

ConvexHullImageMask::ConvexHullImageMask(const std::string &filename)
  : _convex_hull(), _mask() {
  if (filename == "") {
    _v_settings = 0;
    _v_list = 0;
  } else {
    _v_settings = new VarTypes::VarExternal(filename, "Mask");
    _v_list = new VarTypes::VarList("Convex Hull Points");
    _v_settings->addChild(_v_list);

    connect(_v_list,SIGNAL(XMLwasRead(VarType *)),this,SLOT(slotMaskPointsRead()));
  }
}

ConvexHullImageMask::~ConvexHullImageMask() {
  delete _v_settings;
  delete _v_list;
}

void ConvexHullImageMask::reset() {
  lock();

  _convex_hull.clear();
  computeMask(_convex_hull, _mask);
  _v_list->resetToDefault();

  unlock();
}

void ConvexHullImageMask::_addPoint(const int x, const int y, const bool add_to_list) {
  const bool changed = _convex_hull.addPoint(x, y);

  if (changed) {
    computeMask(_convex_hull, _mask);

    if (add_to_list) {
      VarTypes::VarList *point = new VarTypes::VarList();
      point->addChild(new VarTypes::VarInt("x", x));
      point->addChild(new VarTypes::VarInt("y", y));

      _v_list->addChild(point);
    }
  }
}

void ConvexHullImageMask::addPoint(const int x, const int y, const bool add_to_list) {
  lock();
  _addPoint(x, y, add_to_list);
  unlock();
}

void ConvexHullImageMask::removePoint(const int x, const int y, const int margin) {
  lock();

  bool changed = false;
  for (int w = -margin; w < margin + 1 && !changed; ++w)
    for (int h = -margin; h < margin + 1 && !changed; ++h)
      changed = _convex_hull.removePoint(x + w, y + h);

  if (changed) {
    computeMask(_convex_hull, _mask);

    _v_list->resetToDefault();
    for (auto it = _convex_hull.begin(); it != _convex_hull.end(); ++it) {
      const GVector::vector2d<int> &p = *it;

      VarTypes::VarList *point = new VarTypes::VarList();
      point->addChild(new VarTypes::VarInt("x", p.x));
      point->addChild(new VarTypes::VarInt("y", p.y));

      _v_list->addChild(point);
    }
  }

  unlock();
}

void ConvexHullImageMask::setSize(const int w, const int h) {
  lock();
  _mask.allocate(w, h);
  computeMask(_convex_hull, _mask);
  unlock();
}

int ConvexHullImageMask::getNumPixels() const {
  lock();
  const int tmp = _mask.getNumPixels();
  unlock();

  return tmp;
}

int ConvexHullImageMask::getNumBytes() const {
  lock();
  const int tmp = _mask.getNumBytes();
  unlock();

  return tmp;
}

int ConvexHullImageMask::getWidth() const {
  lock();
  const int tmp = _mask.getWidth();
  unlock();

  return tmp;
}

int ConvexHullImageMask::getHeight() const {
  lock();
  const int tmp = _mask.getHeight();
  unlock();

  return tmp;
}

const Image<raw8>& ConvexHullImageMask::getMask() const {
  return _mask;
}

const ConvexHull& ConvexHullImageMask::getConvexHull() const {
  return _convex_hull;
}

VarTypes::VarList * ConvexHullImageMask::getSettings() {
  return _v_settings;
}

void ConvexHullImageMask::lock() const {
  mutex.lock();
}

void ConvexHullImageMask::unlock() const {
  mutex.unlock();
}

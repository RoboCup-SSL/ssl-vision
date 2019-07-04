#ifndef CONVEX_HULL_IMAGE_MASK_H
#define CONVEX_HULL_IMAGE_MASK_H

#include "image.h"
#include "convex_hull.h"
#include "VarTypes.h"
#include <qmutex.h>

class ConvexHullImageMask : public QObject {
  Q_OBJECT
 private:
  ConvexHull _convex_hull;
  Image<raw8> _mask;
  VarTypes::VarExternal * _v_settings;
  VarTypes::VarList * _v_list;
  mutable QMutex mutex;
  void _addPoint(const int x, const int y, const bool add_to_list=true);
  
 public:
  ConvexHullImageMask(const std::string &filename = "");
  virtual ~ConvexHullImageMask();
  void reset();
  void addPoint(const int x, const int y, const bool add_to_list=true);
  void removePoint(const int x, const int y, const int margin = 0);
  void setSize(const int w, const int h);
  int getNumPixels() const;
  int getNumBytes() const;
  int getWidth() const;
  int getHeight() const;
  const Image<raw8>& getMask() const;
  const ConvexHull& getConvexHull() const;

  void lock() const;
  void unlock() const;
  
  VarTypes::VarList* getSettings();

 protected Q_SLOTS:
  void slotMaskPointsRead();
};

#endif

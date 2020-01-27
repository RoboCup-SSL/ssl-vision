#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include <vector>
#include "gvector.h"

class ConvexHull {
 public:
  typedef std::vector<GVector::vector2d<int>>::const_iterator const_iterator;
  
  ConvexHull();
  bool addPoint(const int x, const int y);
  bool removePoint(const int x, const int y);
  void clear();
  ConvexHull::const_iterator begin() const;
  ConvexHull::const_iterator end() const;
  GVector::vector2d<double> centroid() const;
  int getNumPoints() const;
 public:
  std::vector<GVector::vector2d<int>> _points;
};

#endif

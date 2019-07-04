#include "convex_hull.h"
#include <iostream>

using Points = std::vector<GVector::vector2d<int>>;

struct {
  bool operator()(const GVector::vector2d<int> &a, const GVector::vector2d<int> &b) const {
    return (a.x < b.x) || (a.x == b.x && a.y < b.y);
  }
} lexicographicLess;

/*
  Implementation of Andrew's monotone chain convex hull algorithm (O(n log n))
  Returns points in clockwise-order.
*/
Points computeConvexHullFromPoints(Points &P) {
  const int n = P.size();
  if (n == 1) return P;
  
  Points R(2 * n);
  std::sort(P.begin(), P.end(), lexicographicLess);

  int k = 0;
  // Lower hull
  for (int i = 0; i < n; ++i) {
    while (k >= 2 && (R[k - 1] - R[k - 2]).cross(P[i] - R[k - 2]) <= 0) k--;
    R[k++] = P[i];
  }

  // Upper hull
  for (int i = n - 2, t = k + 1; i >= 0; i--) {
    while (k >= t && (R[k - 1] - R[k - 2]).cross(P[i] - R[k - 2]) <= 0) k--;
    R[k++] = P[i];
  }

  R.resize(k - 1);
  return R;
}

ConvexHull::ConvexHull() : _points() {}

bool ConvexHull::addPoint(const int x, const int y) {
  GVector::vector2d<int> p(x, y);
  _points.push_back(p);
  _points = computeConvexHullFromPoints(_points);
  return std::find(_points.begin(), _points.end(), p) != _points.end();
}

bool ConvexHull::removePoint(const int x, const int y) {
  GVector::vector2d<int> p(x, y);
  if (std::find(_points.begin(), _points.end(), p) == _points.end())
    return false;

  _points.erase(std::remove(_points.begin(), _points.end(), p));
  return true;
}

void ConvexHull::clear() {
  _points.clear();
}

GVector::vector2d<double> ConvexHull::centroid() const {
  GVector::vector2d<double> result(0.0, 0.0);
  if (_points.size() == 0)
    return result;
  
  for (auto it = _points.begin(); it != _points.end(); ++it) {
    // necessary because GVector does not cast from int to double
    const GVector::vector2d<int> &p = *it;
    result.x = result.x + p.x;
    result.y = result.y + p.y;
  }

  return result / _points.size();
}

ConvexHull::const_iterator ConvexHull::begin() const {
  return _points.begin();
}

ConvexHull::const_iterator ConvexHull::end() const {
  return _points.end();
}

int ConvexHull::getNumPoints() const {
  return _points.size();
}

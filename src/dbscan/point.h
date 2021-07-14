#pragma once

#include <iostream>
#include <algorithm>
#include <math.h>
#include <iomanip>
#include <limits>
#include "pbbs/parallel.h"

using namespace std;

// *************************************************************
//    any dimensional POINTS
// *************************************************************

template <int _dim> class point {
public:
  using floatT = double;
  using pointT = point;

  floatT x[_dim];
  static const int dim = _dim;
  static constexpr double empty = numeric_limits<double>::max();

  point() { for (int i=0; i<_dim; ++i) x[i]=empty; }
  point(floatT* p) { for (int i=0; i<_dim; ++i) x[i]=p[i]; }
  point(pointT* p) { for (int i=0; i<_dim; ++i) x[i]=p->x[i]; }

  int dimension() {return _dim;}
  void setEmpty() {x[0]=empty;}
  bool isEmpty() {return x[0]==empty;}

  pointT operator-(pointT op2) {
    floatT xx[_dim];
    for (int i=0; i<_dim; ++i) xx[i] = x[i]-op2.x[i];
    return pointT(xx);
  }
  pointT operator/(floatT dv) {
    floatT xx[_dim];
    for (int i=0; i<_dim; ++i) xx[i] = x[i]/dv;
    return pointT(xx);
  }
  pointT operator*(floatT dv) {
    floatT xx[_dim];
    for (int i=0; i<_dim; ++i) xx[i] = x[i]*dv;
    return pointT(xx);
  }
  floatT& operator[](int i) {return x[i];}
  friend bool operator==(pointT a, pointT b) {
    for (intT ii=0; ii<dim; ++ii) {
      if (abs(a[ii]-b[ii]) > 0.0) return false;}
    return true;
  }
  friend bool operator!=(pointT a, pointT b) {return !(a==b);}

  floatT* coordinate() {return x;}
  floatT coordinate(int i) {return x[i];}
  void updateX(int i, floatT val) {x[i]=val;}//Deprecate
  void updateCoordinate(int i, floatT val) {x[i]=val;}
  pointT average(pointT p2) {
    auto pp = pointT();
    for (int i=0; i<_dim; ++i) pp.x[i] = (p2[i] + x[i])/2;
    return pp;
  }
  void minCoords(pointT b) {
    for (int i=0; i<_dim; ++i) x[i] = min(x[i], b.x[i]);}
  void minCoords(floatT* b) {
    for (int i=0; i<_dim; ++i) x[i] = min(x[i], b[i]);}
  void maxCoords(pointT b) {
    for (int i=0; i<_dim; ++i) x[i] = max(x[i], b.x[i]);}
  void maxCoords(floatT* b) {
    for (int i=0; i<_dim; ++i) x[i] = max(x[i], b[i]);}
  intT quadrant(pointT center) {
    intT index = 0;
    intT offset = 1;
    for (int i=0; i<_dim; ++i) {
      if (x[i] > center.x[i]) index += offset;
      offset *= 2;
    }
    return index;
  }
  bool outOfBox(pointT center, floatT hsize) {
    for (int i=0; i<_dim; ++i) {
      if (x[i]-hsize > center.x[i] || x[i]+hsize < center.x[i])
        return true;
    }
    return false;
  }
  inline floatT dist(pointT p) {
    floatT xx=0;
    for (int i=0; i<_dim; ++i) xx += (x[i]-p.x[i])*(x[i]-p.x[i]);
    return sqrt(xx);
  }
  inline floatT distSqr(pointT p) {
    floatT xx=0;
    for (int i=0; i<_dim; ++i) xx += (x[i]-p.x[i])*(x[i]-p.x[i]);
    return xx;
  }
  floatT dot(pointT p2) {
    floatT r = 0;
    for(int i=0; i<dim; ++i) r += x[i]*p2[i];
    return r;}
  pointT mult(floatT c) {
    pointT r;
    for(int i=0; i<dim; ++i) r[i] = x[i]*c;
    return r;}
  pointT normalize() {
    auto r = pointT(x);
    floatT s = 0;
    for (int i=0; i<dim; ++i) s += x[i]*x[i];
    s = sqrt(s);
    for (int i=0; i<dim; ++i) r[i] /= s;
    return r;
  }
};

template <int dim>
static std::ostream& operator<<(std::ostream& os, const point<dim> v) {
  for (int i=0; i<dim; ++i)
    os << v.x[i] << " ";
  return os;
}

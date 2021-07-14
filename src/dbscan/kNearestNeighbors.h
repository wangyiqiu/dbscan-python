// Copyright (c) 2020 Yiqiu Wang
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "kdNode.h"
#include "kBuffer.h"
#include "point.h"
#include "pbbs/parallel.h"

// d-dimensional k-nearest neighbor search using the kdTree

template<int dim, class objT>
void kdNode<dim, objT>::knnRangeHelper(objT* q, pointT qMin, pointT qMax, floatT radius, kbufT *out) {
  int relation = boxCompare(qMin, qMax, getMin(), getMax());

  if(relation == boxExclude) {
    return;
  } else if (relation == boxInclude) {
    for (intT i=0; i<size(); ++i) {
      objT* elem = getItem(i);
      out->insert(kelemT(q->dist(*elem), elem));
    }
  } else { // intersect
    if (isLeaf()) {
      for (intT i=0; i < size(); ++ i) {
        objT* elem = getItem(i);
        float dist = q->dist(*elem);
        if (dist <= radius) {out->insert(kelemT(dist, elem));}
      }
    } else {
      L()->kdNode<dim, objT>::knnRangeHelper(q, qMin, qMax, radius, out);
      R()->kdNode<dim, objT>::knnRangeHelper(q, qMin, qMax, radius, out);
    }
  }
}

template<int dim, class objT>
void kdNode<dim, objT>::knnRange(objT* q, floatT radius, kbufT *out) {
  pointT qMin, qMax;
  for (intT i=0; i<dim; i++) {
    auto tmp = q->coordinate(i)-radius;
    qMin.updateCoordinate(i, tmp);
    qMax.updateCoordinate(i, tmp+radius*2);
  }
  kdNode<dim, objT>::knnRangeHelper(q, qMin, qMax, radius, out);
}

template<int dim, class objT>
void kdNode<dim, objT>::knnHelper(objT* q, intT k, kbufT *out) {

  // find the leaf first
  int relation = boxCompare(getMin(), getMax(), pointT(q->coordinate()), pointT(q->coordinate()));
  if (relation == boxExclude) {
    return;
  } else {
    if (isLeaf()) {
      // basecase
      for (intT i=0; i<size(); ++ i) {
        objT* elem = getItem(i);
        out->insert(kelemT(q->dist(*elem), elem));}
    } else {
      L()->kdNode<dim, objT>::knnHelper(q, k, out);
      R()->kdNode<dim, objT>::knnHelper(q, k, out);
    }
  }

  if (!out->hasK()) {
    if (siblin() == NULL) {
      cout << "error, looks like knnHelper reached root node, still do not have enough neighbors? k = " << k << endl;
      abort();
    }
    for (intT i=0; i<siblin()->size(); ++i) {
      objT* elem = siblin()->getItem(i);
      out->insert(kelemT(q->dist(*elem), elem));}
  } else { // buffer filled to a least k
    if (siblin() != NULL) {
      kelemT tmp = out->keepK();
      siblin()->kdNode<dim, objT>::knnRange(q, tmp.m_cost, out);}
  }
}

template<int dim, class objT>
objT** kdNode<dim, objT>::kNN(objT* q, intT k, objT** R) {
  kbufT* out = new kbufT(k);
  knnHelper(q, k, out);
  out->sortK();
  if(!R) R = newA(objT*, k);
  for(intT i=0; i<k; ++i) R[i] = out->get(i);
  delete out;
  return R;
}

template<int dim, class objT>
void kdNode<dim, objT>::kNN(objT* q, intT k, kbufT* out) {
  out->reset();
  knnHelper(q, k, out);
  out->sortK();
}

// This code is part of the project "Theoretically Efficient and Practical
// Parallel DBSCAN"
// Copyright (c) 2020 Yiqiu Wang, Yan Gu, Julian Shun
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

#ifndef BCCP_CORE_H
#define BCCP_CORE_H

#include "kdTree.h"
#include "kdNode.h"
#include "pbbs/parallel.h"
#include "pbbs/utils.h"

template<class nodeT, class objT>
inline void compBcpCoreHSerial(nodeT* n1, nodeT* n2, floatT* r, intT* coreFlag, objT* P) {
  if (n1->nodeDistance(n2) > *r) return;

  if (n1->isLeaf() && n2->isLeaf()) {//basecase
    for (intT i=0; i<n1->size(); ++i) {
      for (intT j=0; j<n2->size(); ++j) {
        auto pi = n1->getItem(i);
        auto pj = n2->getItem(j);
        if (coreFlag[pi - P] && coreFlag[pj - P]) {
          floatT dist = pi->dist(*pj);
          r[0] = min(r[0], dist);
        }
      }
    }
  } else {//recursive, todo consider call order, might help
    if (n1->isLeaf()) {
      if (n1->nodeDistance(n2->L()) < n1->nodeDistance(n2->R())) {
        compBcpCoreHSerial(n1, n2->L(), r, coreFlag, P);
        compBcpCoreHSerial(n1, n2->R(), r, coreFlag, P);
      } else {
        compBcpCoreHSerial(n1, n2->R(), r, coreFlag, P);
        compBcpCoreHSerial(n1, n2->L(), r, coreFlag, P);
      }
    } else if (n2->isLeaf()) {
      if (n2->nodeDistance(n1->L()) < n2->nodeDistance(n1->R())) {
        compBcpCoreHSerial(n2, n1->L(), r, coreFlag, P);
        compBcpCoreHSerial(n2, n1->R(), r, coreFlag, P);
      } else {
        compBcpCoreHSerial(n2, n1->R(), r, coreFlag, P);
        compBcpCoreHSerial(n2, n1->L(), r, coreFlag, P);
      }
    } else {
      pair<nodeT*, nodeT*> ordering[4];
      ordering[0] = make_pair(n2->L(), n1->L());
      ordering[1] = make_pair(n2->R(), n1->L());
      ordering[2] = make_pair(n2->L(), n1->R());
      ordering[3] = make_pair(n2->R(), n1->R());
      auto bbd = [&](pair<nodeT*,nodeT*> p1, pair<nodeT*,nodeT*> p2) {
                   return p1.first->nodeDistance(p1.second) < p2.first->nodeDistance(p2.second);};
      quickSortSerial(ordering, 4, bbd);
      for (intT o=0; o<4; ++o) {
        compBcpCoreHSerial(ordering[o].first, ordering[o].second, r, coreFlag, P);}
    }
  }
}

template<class nodeT, class objT>
inline void compBcpCoreHBase(nodeT* n1, nodeT* n2, floatT* r, intT* coreFlag, objT* P) {
  if (n1->nodeDistance(n2) > *r) return;

  if (n1->isLeaf() && n2->isLeaf()) {//basecase
    for (intT i=0; i<n1->size(); ++i) {
      for (intT j=0; j<n2->size(); ++j) {
        auto pi = n1->getItem(i);
        auto pj = n2->getItem(j);
        if (coreFlag[pi - P] && coreFlag[pj - P]) {
          floatT dist = pi->dist(*pj);
          utils::writeMin(r, dist);
        }
      }
    }
  } else {//recursive, todo consider call order, might help
    if (n1->isLeaf()) {
      if (n1->nodeDistance(n2->L()) < n1->nodeDistance(n2->R())) {
        compBcpCoreH(n1, n2->L(), r, coreFlag, P);
        compBcpCoreH(n1, n2->R(), r, coreFlag, P);
      } else {
        compBcpCoreH(n1, n2->R(), r, coreFlag, P);
        compBcpCoreH(n1, n2->L(), r, coreFlag, P);
      }
    } else if (n2->isLeaf()) {
      if (n2->nodeDistance(n1->L()) < n2->nodeDistance(n1->R())) {
        compBcpCoreH(n2, n1->L(), r, coreFlag, P);
        compBcpCoreH(n2, n1->R(), r, coreFlag, P);
      } else {
        compBcpCoreH(n2, n1->R(), r, coreFlag, P);
        compBcpCoreH(n2, n1->L(), r, coreFlag, P);
      }
    } else {
      pair<nodeT*, nodeT*> ordering[4];
      ordering[0] = make_pair(n2->L(), n1->L());
      ordering[1] = make_pair(n2->R(), n1->L());
      ordering[2] = make_pair(n2->L(), n1->R());
      ordering[3] = make_pair(n2->R(), n1->R());
      auto bbd = [&](pair<nodeT*,nodeT*> p1, pair<nodeT*,nodeT*> p2) {
                   return p1.first->nodeDistance(p1.second) < p2.first->nodeDistance(p2.second);};
      quickSortSerial(ordering, 4, bbd);
      for (intT o=0; o<4; ++o) {
        compBcpCoreH(ordering[o].first, ordering[o].second, r, coreFlag, P);}
    }
  }
}

template<class nodeT, class objT>
inline void compBcpCoreH(nodeT* n1, nodeT* n2, floatT* r, intT* coreFlag, objT* P) {
  if (n1->nodeDistance(n2) > *r) return;

  if ((n1->isLeaf() && n2->isLeaf()) || (n1->size()+n2->size() < 2000)) {
    return compBcpCoreHBase(n1, n2, r, coreFlag, P);
  } else {//recursive, todo consider call order, might help
    if (n1->isLeaf()) {
      if (n1->nodeDistance(n2->L()) < n1->nodeDistance(n2->R())) {
	par_do([&](){compBcpCoreH(n1, n2->L(), r, coreFlag, P);},
	       [&](){compBcpCoreH(n1, n2->R(), r, coreFlag, P);});
      } else {
	par_do([&](){compBcpCoreH(n1, n2->R(), r, coreFlag, P);},
	       [&](){compBcpCoreH(n1, n2->L(), r, coreFlag, P);});
      }
    } else if (n2->isLeaf()) {
      if (n2->nodeDistance(n1->L()) < n2->nodeDistance(n1->R())) {
	par_do([&](){compBcpCoreH(n2, n1->L(), r, coreFlag, P);},
	       [&](){compBcpCoreH(n2, n1->R(), r, coreFlag, P);});
      } else {
	par_do([&](){compBcpCoreH(n2, n1->R(), r, coreFlag, P);},
	       [&](){compBcpCoreH(n2, n1->L(), r, coreFlag, P);});
      }
    } else {
      pair<nodeT*, nodeT*> ordering[4];
      ordering[0] = make_pair(n2->L(), n1->L());
      ordering[1] = make_pair(n2->R(), n1->L());
      ordering[2] = make_pair(n2->L(), n1->R());
      ordering[3] = make_pair(n2->R(), n1->R());
      auto bbd = [&](pair<nodeT*,nodeT*> p1, pair<nodeT*,nodeT*> p2) {
                   return p1.first->nodeDistance(p1.second) < p2.first->nodeDistance(p2.second);};
      quickSortSerial(ordering, 4, bbd);
      parallel_for (0, 4, [&](intT o) {
	  compBcpCoreH(ordering[o].first, ordering[o].second, r, coreFlag, P);}, 1);
    }
  }
}

template<class cellT, class treeT, class objT>
inline bool hasEdge(intT n1, intT n2, intT* coreFlag, objT* P, floatT epsilon, cellT* cells, treeT** trees) {

  if (cells[n1].size() + cells[n2].size() <= 32) {
    floatT thresh = epsilon*epsilon;
    for (intT i=0; i<cells[n1].size(); ++i) {
      for (intT j=0; j<cells[n2].size(); ++j) {
        auto pi = cells[n1].getItem(i);
        auto pj = cells[n2].getItem(j);
        if (coreFlag[pi - P] && coreFlag[pj - P]) {
          if (pi->distSqr(*pj) <= thresh) return true;}
      }
    }
    return false;
  }

  if (!trees[n1])
    trees[n1] = new treeT(cells[n1].getItem(), cells[n1].size(), false);//todo allocation, parallel
  if (!trees[n2]) 
    trees[n2] = new treeT(cells[n2].getItem(), cells[n2].size(), false);//todo allocation, parallel
  floatT r = floatMax();
  compBcpCoreH(trees[n1]->rootNode(), trees[n2]->rootNode(), &r, coreFlag, P);
  return r <= epsilon;
}

#endif

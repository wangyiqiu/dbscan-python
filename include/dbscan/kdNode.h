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

template<int dim, class objT>
class kdNode {
  typedef double floatT;
  typedef point<dim> pointT;
  typedef kdNode<dim, objT> nodeT;
  static const int spatialMedian = 1;//toggle spatial or object median
  static const int boxInclude = 0;
  static const int boxOverlap = 1;
  static const int boxExclude = 2;
  int k;
  pointT pMin, pMax;
  objT **items;
  intT n;
  nodeT* left;
  nodeT* right;
  nodeT* sib;

  inline void boundingBoxSerial() {
    pMin = pointT(items[0]->coordinate());
    pMax = pointT(items[0]->coordinate());
    for(intT i=0; i<n; ++i) {
      pMin.minCoords(items[i]->coordinate());
      pMax.maxCoords(items[i]->coordinate());
    }}

  inline void boundingBoxParallel() {
    // intT P = getWorkers()*8;
    static const intT P = 36 * 8;
    intT blockSize = (n+P-1)/P;
    pointT localMin[P];
    pointT localMax[P];
    for (intT i=0; i<P; ++i) {
      localMin[i] = pointT(items[0]->coordinate());
      localMax[i] = pointT(items[0]->coordinate());}
    parallel_for(0, P,
		 [&](intT p) {
		   intT s = p*blockSize;
		   intT e = min((intT)(p+1)*blockSize,n);
		   for (intT j=s; j<e; ++j) {
		     localMin[p].minCoords(items[j]->coordinate());
		     localMax[p].maxCoords(items[j]->coordinate());}
		 });
    pMin = pointT(items[0]->coordinate());
    pMax = pointT(items[0]->coordinate());
    for(intT p=0; p<P; ++p) {
      pMin.minCoords(localMin[p].x);
      pMax.maxCoords(localMax[p].x);}
  }

  inline intT splitItemSerial(floatT xM) {
    if (n < 2) {
      cout << "error, kdTree splitting singleton, abort" << endl;abort();}
    intT lPt = 0;
    intT rPt = n-1;
    while (lPt < rPt) {
      if (items[lPt]->coordinate(k)>=xM) {
        while (items[rPt]->coordinate(k)>=xM && lPt < rPt) {
          rPt--;
        }
        if (lPt < rPt) {
          swap(items[lPt], items[rPt]);
          rPt--; }
        else { break;}
      }
      lPt++;
    }
    if (items[lPt]->coordinate(k) < xM) lPt++;
    return lPt;
  }

  inline intT splitItemParallel(floatT xM, objT **scratch, intT* flags) {
    if (n < 2) {
      cout << "error, kdTree splitting singleton, abort" << endl;abort();}
    parallel_for(0, n,
		 [&](intT i) {
		   if (items[i]->coordinate(k)<xM) flags[i]=1;
		   else flags[i] = 0;
		 });
    intT leftSize = sequence::prefixSum(flags,0,n);
    parallel_for(0, n-1,
		 [&](intT i) {
		   if (flags[i] != flags[i+1]) scratch[flags[i]] = items[i];
		   if (i-flags[i]
		       != i+1-flags[i+1]) scratch[leftSize+i-flags[i]] = items[i];
		 });
    if (flags[n-1] != leftSize) scratch[flags[n-1]] = items[n-1];
    if (n-1-flags[n-1] != n-leftSize) scratch[leftSize+n-1-flags[n-1]] = items[n-1];
    parallel_for(0, n,
		 [&](intT i) {
		   items[i] = scratch[i];});
    return leftSize;
  }

  inline int boxCompare(pointT pMin1, pointT pMax1, pointT pMin2, pointT pMax2) {
    bool exclude = false;
    bool include = true;//1 include 2
    for(int i=0; i<dim; ++i) {
      if (pMax1[i]<pMin2[i] || pMin1[i]>pMax2[i]) exclude = true;
      if (pMax1[i]<pMax2[i] || pMin1[i]>pMin2[i]) include = false;
    }
    if (exclude) return boxExclude;
    else if (include) return boxInclude;
    else return boxOverlap;
  }

  inline bool itemInBox(pointT pMin1, pointT pMax1, objT* item) {
    for(int i=0; i<dim; ++i) {
      if (pMax1[i]<item->coordinate(i) || pMin1[i]>item->coordinate(i)) return false;
    }
    return true;}

  intT findWidest() {
    floatT xM = -1;
    for (int kk=0; kk<dim; ++kk) {
      if (pMax[kk]-pMin[kk]>xM) {
        xM = pMax[kk]-pMin[kk];
        k = kk;}}
    return k;
  }

  void constructSerial(nodeT *space, intT leafSize) {
    boundingBoxSerial();
    sib = NULL;
    if (n <= leafSize) {
      left = NULL; right = NULL;
    } else {
      if (!space[0].isEmpty() || !space[1].isEmpty()) {
        cout << "error, kdNode overwrite, abort" << endl;abort();}
      intT k = findWidest();
      floatT xM = (pMax[k]+pMin[k])/2;

      intT median;
      if (spatialMedian) {
        median = splitItemSerial(xM);
      } else {
        auto splitK = [&](objT* a, objT* b) {
                        return a->coordinate(k) < b->coordinate(k);};
        median = ceil(n/2.0);
        nth_element(items, items+median, items + n, splitK);
      }

      if (median == 0 || median == n) {median = ceil(n/2.0);}
      space[0] = nodeT(items, median, space+1, leafSize);
      space[2*median-1] = nodeT(items+median, n-median, space+2*median, leafSize);
      left = space;
      right = space+2*median-1;
      left->sib = right;
      right->sib = left;
    }
  }

  void constructParallel(nodeT *space, objT** scratch, intT* flags, intT leafSize) {
    boundingBoxParallel();
    sib = NULL;
    if (n <= leafSize) {
      left = NULL; right = NULL;
    } else {
      if (!space[0].isEmpty() || !space[1].isEmpty()) {
        cout << "error, kdNode overwrite, abort" << endl;abort();}
      intT k = findWidest();
      floatT xM = (pMax[k]+pMin[k])/2;

      intT median;
      if (spatialMedian) {
        median = splitItemParallel(xM, scratch, flags);
      } else {
        auto splitK = [&](objT* a, objT* b) {
                        return a->coordinate(k) < b->coordinate(k);};
        median = ceil(n/2.0);
        nth_element(items, items+median, items + n, splitK);//todo
      }

      if (median == 0 || median == n) {median = (n/2.0);}
      par_do([&](){space[0] = nodeT(items, median, space+1, scratch, flags, leafSize);},
	     [&](){space[2*median-1] = nodeT(items+median, n-median, space+2*median, scratch+median, flags+median, leafSize);});
      left = space;
      right = space+2*median-1;
      left->sib = right;
      right->sib = left;
    }
  }

  public:
  inline nodeT* L() {return left;}
  inline nodeT* R() {return right;}
  inline nodeT* siblin() {return sib;}//todo
  inline pair<pointT, pointT> getBox() {return make_pair(pMin, pMax);}
  inline objT** getItems() {return items;}
  inline objT* getItem(intT i) {return items[i];}
  inline intT size() {return n;}
  inline objT* operator[](intT i) {return items[i];}
  inline pointT getMax() {return pMax;}
  inline pointT getMin() {return pMin;}
  inline floatT getMax(intT i) {return pMax[i];}
  inline floatT getMin(intT i) {return pMin[i];}

  kdNode(objT** itemss, intT nn, nodeT *space, objT** scratch, intT* flags, intT leafSize=16): items(itemss), n(nn) {
    if (n>2000) constructParallel(space, scratch, flags, leafSize);
    else constructSerial(space, leafSize);}
  kdNode(objT** itemss, intT nn, nodeT *space, intT leafSize=16): items(itemss), n(nn) {
    constructSerial(space, leafSize);}

  void setEmpty() {n=-1;}
  bool isEmpty() {return n<0;}
  bool isLeaf() {return !left;}//check

  floatT nodeDiag() {//todo change name
    floatT result = 0;
    for (int d = 0; d < dim; ++ d) {
      floatT tmp = pMax[d] - pMin[d];
      result += tmp * tmp;
    }
    return sqrt(result);
  }

  // return maximum span of node bounding box among all dimensions
  inline floatT lMax() {
    floatT myMax = 0;
    for (int d=0; d<dim; ++d) {
      floatT thisMax = pMax[d] - pMin[d];
      if (thisMax > myMax) {
	myMax = thisMax;}
    }
    return myMax;
  }

  //return the bb distance with and n2
  inline floatT nodeDistance(nodeT* n2) {
    for (int d = 0; d < dim; ++ d) {
      if (pMin[d] > n2->pMax[d] || n2->pMin[d] > pMax[d]) {
        // disjoint at dim d, and intersect on dim < d
        floatT rsqr = 0;
        for (int dd = d; dd < dim; ++ dd) {
          floatT tmp = max(pMin[dd]-n2->pMax[dd], n2->pMin[dd]-pMax[dd]);
          tmp = max(tmp, (floatT)0);
          rsqr += tmp*tmp;
        }
        return sqrt(rsqr);
      }
    }
    return 0; // intersect
  }

  //return the far bb distance between n1 and n2
  inline floatT nodeFarDistance(nodeT* n2) {
    floatT result = 0;
    for (int d = 0; d < dim; ++ d) {
      floatT tmp = max(pMax[d],n2->pMax[d]) - min(pMin[d],n2->pMin[d]);
      result += tmp *tmp;
    }
    return sqrt(result);
  }

  //whether well separated with v
  inline bool wellSeparated(nodeT *v, floatT s=2) {
    floatT circleDiam_u = 0;
    floatT circleDiam_v = 0;
    floatT circleDistance = 0;
    for (int d = 0; d < dim; ++ d) {
      floatT uTmpDiff = pMax[d] - pMin[d];
      floatT vTmpDiff = v->pMax[d] - v->pMin[d];
      floatT uTmpAvg = (pMax[d] + pMin[d])/2;
      floatT vTmpAvg = (v->pMax[d] + v->pMin[d])/2;
      circleDistance += (uTmpAvg - vTmpAvg) * (uTmpAvg - vTmpAvg);
      circleDiam_u += uTmpDiff * uTmpDiff;
      circleDiam_v += vTmpDiff * vTmpDiff;
    }
    circleDiam_u = sqrt(circleDiam_u);
    circleDiam_v = sqrt(circleDiam_v);
    floatT myRadius = max(circleDiam_u, circleDiam_v)/2;
    circleDistance = sqrt(circleDistance) - circleDiam_u/2 - circleDiam_v/2;
    return circleDistance >= (s * myRadius);
  }

  //vecT need to be vector<objT*>
  template<class vecT>
  void rangeNeighbor(pointT queryPt, floatT r, pointT pMin1, pointT pMax1, vecT* accum) {
    int relation = boxCompare(pMin1, pMax1, pMin, pMax);
    if (relation == boxInclude) {
      for(intT i=0; i<n; ++i) {
	if (items[i]->getCoordObj()->dist(queryPt) <= r)
	  accum->push_back(items[i]);
      }
    } else if (relation == boxOverlap) {
      if (isLeaf()) {
        for(intT i=0; i<n; ++i) {
	  if (items[i]->getCoordObj()->dist(queryPt) <= r &&
	      itemInBox(pMin1, pMax1, items[i])) accum->push_back(items[i]);
	}
      } else {
        left->rangeNeighbor(queryPt, r, pMin1, pMax1, accum);
        right->rangeNeighbor(queryPt, r, pMin1, pMax1, accum);}
    }
  }

  template<class func, class func2>
  void rangeNeighbor(pointT queryPt, floatT r, pointT pMin1, pointT pMax1, func term, func2 doTerm) {
    if (term()) return;
    int relation = boxCompare(pMin1, pMax1, pMin, pMax);
    if (relation == boxInclude) {
      for(intT i=0; i<n; ++i) {
	if (items[i]->getCoordObj()->dist(queryPt) <= r &&
	    doTerm(items[i])) break;
      }
    } else if (relation == boxOverlap) {
      if (isLeaf()) {
        for(intT i=0; i<n; ++i) {
	  if (items[i]->getCoordObj()->dist(queryPt) <= r &&
		doTerm(items[i])) break;
        }
      } else {
        left->rangeNeighbor(queryPt, r, pMin1, pMax1, term, doTerm);
        right->rangeNeighbor(queryPt, r, pMin1, pMax1, term, doTerm);}
    }
  }

  struct bcp {
    objT* u;
    objT* v;
    floatT dist;
    bcp(objT* uu, objT* vv, floatT distt): u(uu), v(vv), dist(distt) {}
    bcp(): u(NULL), v(NULL), dist(floatMax()) {}
    void update(objT* uu, objT* vv) {
      auto distt = uu->dist(*vv);
      if (distt < dist) {
        u = uu; v = vv; dist = distt;}
    }
    void update(objT* uu, objT* vv, floatT distt) {
      if (distt < dist) {
        u = uu; v = vv; dist = distt;}
    }
  };

  inline void compBcpH(nodeT* n2, bcp* r) {
    if (nodeDistance(n2) > r->dist) return;

    if (isLeaf() && n2->isLeaf()) {//basecase
      for (intT i=0; i<size(); ++i) {
        for (intT j=0; j<n2->size(); ++j) {
          r->update(items[i], n2->items[j]);}
      }
    } else {//recursive, todo consider call order, might help
      if (isLeaf()) {
        if (nodeDistance(n2->left) < nodeDistance(n2->right)) {
          compBcpH(n2->left, r);
          compBcpH(n2->right, r);
        } else {
          compBcpH(n2->right, r);
          compBcpH(n2->left, r);
        }
      } else if (n2->isLeaf()) {
        if (n2->nodeDistance(left) < n2->nodeDistance(right)) {
          n2->compBcpH(left, r);
          n2->compBcpH(right, r);
        } else {
          n2->compBcpH(right, r);
          n2->compBcpH(left, r);
        }
      } else {
        pair<nodeT*, nodeT*> ordering[4];
        ordering[0] = make_pair(n2->left, left);
        ordering[1] = make_pair(n2->right, left);
        ordering[2] = make_pair(n2->left, right);
        ordering[3] = make_pair(n2->right, right);
        auto bbd = [&](pair<nodeT*,nodeT*> p1, pair<nodeT*,nodeT*> p2) {
                     return p1.first->nodeDistance(p1.second) < p2.first->nodeDistance(p2.second);};
        quickSortSerial(ordering, 4, bbd);
        for (intT o=0; o<4; ++o) {
          ordering[o].first->compBcpH(ordering[o].second, r);}
      }
    }
  }

  inline bcp compBcp(nodeT* n2) {
    auto r = bcp();
    compBcpH(n2, &r);
    // for (intT i=0; i<size(); ++i) {
    //   for (intT j=0; j<n2->size(); ++j) {
    //     r.update(items[i], n2->items[j]);}
    // }
    return r;
  }

};

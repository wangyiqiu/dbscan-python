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

#pragma once

template<int dim, class pointT>
intT* coreBF(pointT* P, intT n, floatT epsilon, intT minPts) {
  intT* coreFlag = newA(intT, n);

  parallel_for (0, n, [&](intT i) {
      coreFlag[i] = 0;
      for (intT j=0; j<n; ++j) {
	if (P[i].dist(P[j]) <= epsilon) {
	  coreFlag[i] ++;
	}}
    });

  intT numCore = 0;
  for (intT i=0; i<n; ++i) {
    if (coreFlag[i] >= minPts) {
      coreFlag[i] = 1;
      numCore ++;
    } else coreFlag[i] = 0;
    //cout << coreFlag[i] << " ";
  }
  //cout << endl << endl;
  cout << "bf-num-core = " << numCore << endl;
  return coreFlag;
}


template<int dim, class pointT>
intT* clusterCoreBF(pointT* P, intT n, floatT epsilon, intT minPts, intT* coreFlag) {
  auto uf = unionFind(n);
  parallel_for (0, n, [&](intT i) {
      for (intT j=i+1; j<n; ++j) {
	if (coreFlag[i] && coreFlag[j] && P[i].dist(P[j]) <= epsilon) {
	  uf.link(i,j);
	}}
    });
  intT* cluster = newA(intT, n);
  parallel_for (0, n, [&](intT i) {
      auto pi = P[i];
      cluster[i] = -1;
      if(coreFlag[i]) cluster[i] = uf.find(i);
    });
  // for (intT i=0; i<n; ++i) cout << cluster[i] << " ";
  // cout << endl;
  uf.del();
  return cluster;
}

template<int dim, class pointT>
void clusterBorderBF(pointT* P, intT n, floatT epsilon, intT minPts, intT* coreFlag, intT* clusterb) {
  floatT thresh = epsilon*epsilon;
  parallel_for(0, n, [&](intT i) {
      if (!coreFlag[i]) {
	intT cid = -1;
	floatT cDistSqr = floatMax();
	for(intT j=0; j<n; ++j) {
	  if (coreFlag[j]) {
	    auto dist = P[i].distSqr(P[j]);
	    if (dist <= thresh && dist < cDistSqr) {
	      cid = clusterb[j];
	      cDistSqr = dist;
	    }
	  }
	}
	clusterb[i] = cid;
      }
    });
}

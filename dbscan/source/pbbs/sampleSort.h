// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2010 Guy Blelloch and Harsha Vardhan Simhadri and the PBBS team
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

// This file is basically the cache-oblivious sorting algorithm from:
//
// Low depth cache-oblivious algorithms.
// Guy E. Blelloch, Phillip B. Gibbons and  Harsha Vardhan Simhadri.
// Proc. ACM symposium on Parallelism in algorithms and architectures (SPAA), 2010

// intT is either "int" or "long" (needs to be "long" if n >= 2^31)

#ifndef SAMPLE_SORT_H
#define SAMPLE_SORT_H

#include "parallel.h"
#include "sequence.h"
#include "math.h"
#include "quickSort.h"
#include "transpose.h"
#include "utils.h"

template<class E, class BinPred, class intT>
void mergeSeq (E* sA, E* sB, intT* sC, long lA, long lB, BinPred f) {
  if (lA==0 || lB==0) return;
  E *eA = sA+lA;
  E *eB = sB+lB;
  for (long i=0; i <= lB; i++) sC[i] = 0;
  while(1) {
    while (f(*sA, *sB)) {(*sC)++; if (++sA == eA) return;}
    sB++; sC++;
    if (sB == eB) break;
    if (!(f(*(sB-1),*sB))) {
      while (!f(*sB, *sA)) {(*sC)++; if (++sA == eA) return;}
      sB++; sC++;
      if (sB == eB) break;
    }
  } 
  *sC = eA-sA;
}

inline unsigned long hashVal(unsigned long a) {
  // 982.. is a largish prime
  return (((unsigned long) 982451653 * a) + (unsigned long) 12345);
}

// the following parameters can be tuned
#define PBBS_QUICKSORT_THRESHOLD 1000
#define PBBS_BLOCK_QUOTIENT 2
#define PBBS_BUCKET_QUOTIENT 2
#define PBBS_OVER_SAMPLE 10

template<class E, class BinPred, class intT>
void sampleSort (E* A, intT n, BinPred f) {
  if (n < PBBS_QUICKSORT_THRESHOLD) quickSort(A, n, f);  
  else {
    long sqrt = (long) ceil(pow(n,0.5));
    long numBlocks = (long) (sqrt/PBBS_BLOCK_QUOTIENT) + 1;
    long blockSize = ((n-1)/numBlocks) + 1;
    int numBuckets = (int) ((sqrt/PBBS_BUCKET_QUOTIENT) + 1);
    long sampleSetSize = numBuckets * PBBS_OVER_SAMPLE;

    E* sampleSet = newA(E,sampleSetSize);

    // generate "random" samples with oversampling
    parallel_for(0, sampleSetSize,
		 [&](intT j) {sampleSet[j] = A[hashVal(j)%n];});

    // sort the samples
    quickSort(sampleSet, sampleSetSize, f);

    // subselect samples at even stride
    E* pivots = newA(E,numBuckets-1);
    parallel_for(0, numBuckets-1,
		 [&](intT k){pivots[k] = sampleSet[PBBS_OVER_SAMPLE*k];});
    free(sampleSet);

    // sort each block and merge with samples to get counts for each bucket
    intT *counts = newA(intT, numBlocks*numBuckets);
    parallel_for(0, numBlocks,
		 [&](intT i) {
		   long offset = i * blockSize;
		   long size =  (i < numBlocks - 1) ? blockSize : n - offset;
		   quickSort(A+offset, size, f);
		   mergeSeq(A + offset, pivots, counts + i*numBuckets, size, numBuckets-1, f);
		 });

    E *B = newA(E, numBlocks*blockSize);
    intT *sourceOffsets = newA(intT, numBlocks*numBuckets);
    intT *destOffsets = newA(intT, numBlocks*numBuckets);

    // transpose from blocks-major to bucket-major
    sequence::scan(counts, sourceOffsets, numBlocks*numBuckets, plus<intT>(),(intT)0);
    transpose<intT,intT>(counts, destOffsets).trans(numBlocks, numBuckets);
    sequence::scan(destOffsets, destOffsets,
		   numBlocks*numBuckets, plus<intT>(),(intT)0);
    blockTrans<E,intT>(A, B, sourceOffsets,
		       destOffsets, counts).trans(numBlocks, numBuckets);
    free(sourceOffsets);
    free(counts);

    // sort within each bucket
    parallel_for(0, numBuckets,
		 [&](intT i) {
		   long start = destOffsets[i*numBlocks];
		   long end = (i < numBuckets -1) ? destOffsets[(i+1)*numBlocks] : n;

		   // middle buckets need not be sorted if two consecutive pivots are equal
		   if (i == 0 || i == numBuckets - 1 || f(pivots[i-1],pivots[i]))
		     quickSort(B+start, end - start, f);

		   // copy back to A
		   for (long j = start; j < end; j++)
		     A[j] = B[j];
		 });
    free(pivots);
    free(destOffsets);
    free(B);
  }
}

#undef compSort
#define compSort(__A, __n, __f) (sampleSort(__A, __n, __f))

#endif

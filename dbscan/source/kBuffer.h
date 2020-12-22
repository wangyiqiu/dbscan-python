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

#ifndef K_BUFFER_H
#define K_BUFFER_H

#include "pbbs/parallel.h"
#include "pbbs/sampleSort.h"

// *************************************************************
//    KBuffer is a data structure to keep the smallest k elements
//    and support insertion.
//
//    - struct KBuffer will keep the smallest k KElem<T> based on
//    its cost, i.e. KElem<T>.m_cost;
//    - it supports KInsert and KeepK for insertion and
//    keeping the topK. KeepK will also resturn the kth smallest
//    - KBuffer will not sort the top k
//    - k = [1, xx]
// *************************************************************

namespace KBuffer {

  template <typename T>
  struct KElem {
    
    floatT m_cost;
    T m_entry;
    KElem(floatT t_cost, T t_entry) : m_cost(t_cost), m_entry(t_entry) {}
    KElem(floatT t_cost) : m_cost(t_cost), m_entry(T()) {}
    KElem() : m_cost(std::numeric_limits<floatT>::max()), m_entry(T()) {}
    bool operator<(const KElem& other) const {
      if (m_cost < other.m_cost) return true;
      return false;
    }
    bool operator<=(const KElem& other) const {
      if (m_cost <= other.m_cost) return true;
      return false;
    }
    bool operator>(const KElem& other) const {
      if (m_cost > other.m_cost) return true;
      return false;
    }
    bool operator>=(const KElem& other) const {
      if (m_cost >= other.m_cost) return true;
      return false;
    }
    bool operator==(const KElem& other) const {
      if (m_cost == other.m_cost) return true;
      return false;
    }
    bool operator!=(const KElem& other) const {
      if (m_cost != other.m_cost) return true;
      return false;
    }
  };

  // comparator for sampleSort.h of pbbs, todo use lambda
  template <class T>
  struct KElemLtCmp {
    bool operator()(const KElem<T>& u, const KElem<T>& v) {
      return u.m_cost < v.m_cost;
    }
  };


  // *************************************************************
  //    QUICK SELECT
  //    REFERENCE: https://www.geeksforgeeks.org/quickselect-algorithm/
  //    with modifications, subroutine for KeepK
  // *************************************************************

  // Standard partition process of QuickSort().
  // It considers the last element as pivot
  // and moves all smaller element to left of
  // it and greater elements to right
  template <class T>
  inline intT partition(KElem<T> *arr, intT l, intT r)
  {
    KElem<T> x = arr[r - 1];
    intT i = l;
    for (intT j = l; j < r - 1; j++) {
      if (arr[j] <= x) {
        swap(arr[i], arr[j]);
        i++;
      }
    }
    swap(arr[i], arr[r - 1]);
    return i;
  }

  // This function returns k'th smallest
  // element in arr[l..r] using QuickSort
  // based method.  ASSUMPTION: ALL ELEMENTS
  // IN ARR[] ARE DISTINCT
  template <class T>
  inline KElem<T> kthSmallest(KElem<T> *arr, intT l, intT r, intT k)
  {

    // If k is smaller than number of
    // elements in array
    if (k >= 0 && k < r - l) {

      // Partition the array around last
      // element and get position of pivot
      // element in sorted array
      intT index = partition(arr, l, r);

      // If position is same as k
      if (index - l == k)
        return arr[index];

      // If position is more, recur
      // for left subarray
      if (index - l > k)
        return kthSmallest(arr, l, index, k);

      // Else recur for right subarray
      return kthSmallest(arr, index, r,
                         k - index + l);
    }

    // If k is more than number of
    // elements in array
    return KElem<T>();
  }

  // *************************************************************
  //    KBuffer
  // *************************************************************

  template <typename T>
  struct KBuffer {
    typedef double floatT;
    intT m_k; // keeping k elements
    intT m_ptr; // next write location m_buf
    intT m_used; // next read location m_buf
    intT m_maxLen; // m_buf length
    KElem<T> *m_buf; // buffer

    KBuffer(intT t_k): m_k(t_k), m_ptr(0), m_used(0), m_maxLen(t_k*2) {
      m_buf = newA(KElem<T>, m_maxLen);
      //for (intT i=0; i<m_maxLen; ++i) m_buf[i] = KElem<T>();
    }

    ~KBuffer() {
      free(m_buf);
    }

    inline void reset() {
      m_ptr = 0;
      m_used = 0;
    }

    bool hasK() {
      return m_ptr >= m_k;
    }

    // keep the top k of a KBuffer, compact to index [0,k), but unsorted
    // also return the kth smallest (last of kept)
    // only valid when buffer contains at least k elements
    KElem<T> keepK() {
      if (m_ptr < m_k) {
        cout << "illegal use of keep k, fewer than k elems" << endl;
        exit(1);
      }
      KElem<T> result = kthSmallest(m_buf, 0, m_ptr, m_k-1);
      m_ptr = m_k;
      return result;
    }

    intT sortK() {
      if (hasK()) {
        keepK();
        sampleSort(m_buf, m_k, KElemLtCmp<T>());
        return m_k;
      } else {
        sampleSort(m_buf, m_ptr, KElemLtCmp<T>());
        return m_ptr;
      }
    }

    void insert(KElem<T> t_elem) {
      m_buf[m_ptr++] = t_elem;
      if (m_ptr >= m_maxLen) {
        keepK();
      }
    }

    T get(intT i) {
      if (i < m_ptr) { //check access range
        return m_buf[i].m_entry;
      } else {
        return T();
      }
    }

  };

  template <class T>
  inline struct KBuffer<T> *allocKBuffer(intT k, intT n) {
    typedef struct KBuffer<T> bufT;
    typedef struct KElem<T> elemT;
    intT bufLen = max((intT)(k * 2), (intT)2);
    //struct KBuffer<T> *kb = (struct KBuffer<T> *) malloc( ( sizeof(struct KBuffer<T>)*n ));
    auto kb = newA(bufT, n);
    //KElem<T> *kb_buffer = (KElem<T> *) malloc( sizeof(KElem<T>) * bufLen * n);
    auto kb_buffer = newA(elemT, bufLen*n);
    parallel_for(0, n, [&](intT i) {
			 kb[i].m_k = k;
			 kb[i].m_ptr = 0;
			 kb[i].m_used = 0;
			 kb[i].m_maxLen = bufLen;
			 kb[i].m_buf = kb_buffer + bufLen*i;
		       });
    return kb;
  }

  template <class T>
  inline void deleteKBuffer(struct KBuffer<T> *in) {
    free(in[0].m_buf);
    free(in);
  }
}

#endif

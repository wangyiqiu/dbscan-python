// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2010 Guy Blelloch and the PBBS team
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

#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <algorithm>
#include "parallel.h"

/*
#if defined(__APPLE__)
#define PTCMPXCH "  cmpxchgl %2,%1\n"
#else
#define PTCMPXCH "  cmpxchgq %2,%1\n"

// Needed to make frequent large allocations efficient with standard
// malloc implementation.  Otherwise they are allocated directly from
// vm.
#include <malloc.h>
static int __ii =  mallopt(M_MMAP_MAX,0);
static int __jj =  mallopt(M_TRIM_THRESHOLD,-1);
#endif
*/

#define newA(__E,__n) (__E*) malloc((__n)*sizeof(__E))

namespace utils {

// returns the log base 2 rounded up (works on ints or longs or unsigned versions)
template <class T>
static int log2Up(T i) {
  int a=0;
  T b=i-1;
  while (b > 0) {b = b >> 1; a++;}
  return a;
}

inline unsigned int hash(unsigned int a)
{
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   return a;
}

inline int hashInt(unsigned int a) {
  return hash(a) & (((unsigned) 1 << 31) - 1);
}

inline unsigned int hash2(unsigned int a)
{
  return (((unsigned int) 1103515245 * a) + (unsigned int) 12345) %
    (unsigned int) 0xFFFFFFFF;
}

/*

// compare and swap on 8 byte quantities
inline bool LCAS(long *ptr, long oldv, long newv) {
  unsigned char ret;
  // Note that sete sets a 'byte' not the word
  __asm__ __volatile__ (
                "  lock\n"
                "  cmpxchgq %2,%1\n"
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (newv), "m" (*ptr), "a" (oldv)
                : "memory");
  return ret;
}

// compare and swap on 4 byte quantity
inline bool SCAS(int *ptr, int oldv, int newv) {
  unsigned char ret;
  // Note that sete sets a 'byte' not the word
  __asm__ __volatile__ (
                "  lock\n"
                "  cmpxchgl %2,%1\n"
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (newv), "m" (*ptr), "a" (oldv)
                : "memory");
  return ret;
}

//#if defined(MCX16)
//ET should be 128 bits and 128-bit aligned
template <class ET>
  inline bool CAS128(ET* a, ET b, ET c) {
  return __sync_bool_compare_and_swap_16((__int128*)a,*((__int128*)&b),*((__int128*)&c));
}
//#endif

// The conditional should be removed by the compiler
// this should work with pointer types, or pairs of integers
template <class ET>
inline bool CAS(ET *ptr, ET oldv, ET newv) {
  if (sizeof(ET) == 1) {
    return __sync_bool_compare_and_swap_1((bool*) ptr, *((bool*) &oldv), *((bool*) &newv));
  } else if (sizeof(ET) == 8) {
    return __sync_bool_compare_and_swap_8((long*) ptr, *((long*) &oldv), *((long*) &newv));
    //return utils::LCAS((long*) ptr, *((long*) &oldv), *((long*) &newv));
  } else if (sizeof(ET) == 4) {
    return __sync_bool_compare_and_swap_4((int *) ptr, *((int *) &oldv), *((int *) &newv));
    //return utils::SCAS((int *) ptr, *((int *) &oldv), *((int *) &newv));
  }
//#if defined(MCX16)
  else if (sizeof(ET) == 16) {
    return utils::CAS128(ptr, oldv, newv);
  }
//#endif
  else {
    std::cout << "common/utils.h CAS bad length " << sizeof(ET) << std::endl;
    abort();
  }
}

*/

/*

template <class ET>
// inline bool CAS_GCC(ET *ptr, ET oldv, ET newv) {
inline bool CAS(ET *ptr, ET oldv, ET newv) {
  if (sizeof(ET) == 4) {
    return __sync_bool_compare_and_swap((int*)ptr, *((int*)&oldv), *((int*)&newv));
  } else if (sizeof(ET) == 8) {
    return __sync_bool_compare_and_swap((long*)ptr, *((long*)&oldv), *((long*)&newv));
  }
#ifdef MCX16
  else if(sizeof(ET) == 16)
    return __sync_bool_compare_and_swap_16((__int128*)ptr,*((__int128*)&oldv),*((__int128*)&newv));
#endif
  else {
    std::cout << "common/utils.h CAS_GCC bad length" << sizeof(ET) << std::endl;
    abort();
  }
}

*/

/*

inline long xaddl(long *variable, long value) {
   asm volatile(
		"lock; xaddl %%eax, %2;"
		:"=a" (value)                   //Output
		: "a" (value), "m" (*variable)  //Input
		:"memory" );
   return value;
}

inline int xaddi(int *variable, int value) {
   asm volatile(
		"lock; xadd %%eax, %2;"
		:"=a" (value)                   //Output
		: "a" (value), "m" (*variable)  //Input
		:"memory" );
   return value;
}

// The conditional should be removed by the compiler
// this should work with pointer types, or pairs of integers
template <class ET>
inline ET xadd(ET *variable, ET value) {
  if (sizeof(ET) == 8) {
    return xaddl((long*)variable,(long)value);
  } else if (sizeof(ET) == 4) {
    return xaddi((int*)variable,(int)value);
  } else {
    std::cout << "xadd bad length" << std::endl;
    abort();
  }
}


template <class ET>
inline ET fetchAndAdd(ET *a, ET b) {
  volatile ET newV, oldV;
  //abort();
  do {oldV = *a; newV = oldV + b;}
  while (!CAS_GCC(a, oldV, newV));
  return oldV;
}

template <class ET>
inline void writeAdd(ET *a, ET b) {
  volatile ET newV, oldV;
  do { oldV = *a; newV = oldV + b;}
  while (!CAS_GCC(a, oldV, newV));
}

template <class ET>
inline bool writeAddOnce(ET *a, ET b) {
  volatile ET newV, oldV;
  oldV = *a; newV = oldV + b;
  return CAS_GCC(a, oldV, newV);
}

template <class ET>
inline bool writeAddOnce(ET *a, ET b, intT k) {
  volatile ET newV, oldV;
  for (intT i=0; i<k; i++) {
    oldV = *a; newV = oldV + b;
    if (CAS_GCC(a, oldV, newV)) {
      return true;
    }
  }
  return false;
}

template <class ET>
inline bool writeMax(ET *a, ET b) {
  ET c; bool r=0;
  do c = *a;
  while (c < b && !(r=CAS_GCC(a,c,b)));
  return r;
}

template <class ET>
inline bool writeMin(ET *a, ET b) {
  ET c; bool r=0;
  do c = *a;
  while (c > b && !(r=CAS_GCC(a,c,b)));
  return r;
}

template <class ET>
inline bool writeMin(ET **a, ET *b) {
  ET* c; bool r = 0;
  do c = *a;
  while (c > b && !(r=CAS_GCC(a,c,b)));
  return r;
}

 template <class ET, class F>
  inline bool writeMin(ET *a, ET b, F f) {
  ET c; bool r=0;
  do c = *a;
  while (f(b,c) && !(r=CAS_GCC(a,c,b)));
  return r;
}

template <class E>
struct identityF { E operator() (const E& x) {return x;}};

template <class E>
struct addF { E operator() (const E& a, const E& b) const {return a+b;}};

template <class E>
struct absF { E operator() (const E& a) const {return std::abs(a);}};

template <class E>
struct zeroF { E operator() (const E& a) const {return 0;}};

template <class E>
struct maxF { E operator() (const E& a, const E& b) const {return (a>b) ? a : b;}};

template <class E>
struct minF { E operator() (const E& a, const E& b) const {return (a<b) ? a : b;}};

template <class E1, class E2>
  struct firstF {E1 operator() (std::pair<E1,E2> a) {return a.first;} };

template <class E1, class E2>
  struct secondF {E2 operator() (std::pair<E1,E2> a) {return a.second;} };

*/

template<typename eType>
bool myCAS(eType* p, eType o, eType n) {
  return std::atomic_compare_exchange_strong_explicit(
    reinterpret_cast<std::atomic<eType>*>(p), &o, n, std::memory_order_acq_rel, std::memory_order_acquire);
}

template <class ET>
inline bool writeMin(ET *a, ET b) {
  ET c; bool r=0;
  do c = *a; 
  // while (c > b && !(r=CAS_GCC(a,c,b)));
  while (c > b &&!(r=myCAS(a,c,b)));
  return r;
}

template <class E>
struct identityF { E operator() (const E& x) {return x;}};

template <class E>
struct addF { E operator() (const E& a, const E& b) const {return a+b;}};

}

#endif
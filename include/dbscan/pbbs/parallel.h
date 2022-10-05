#ifndef PARALLEL_H
#define PARALLEL_H

#include <limits>
#include <iostream>
using namespace std;

typedef int intT;
typedef unsigned int uintT;
typedef double floatT;
static intT intMax() {return numeric_limits<intT>::max();}
static floatT floatMax() {return numeric_limits<floatT>::max();}

#define HOMEGROWN

#if defined(OPENCILK)

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#define parallel_main main
#define par_for cilk_for
#define par_for_1 _Pragma("cilk_grainsize = 1") cilk_for
#define par_for_256 _Pragma("cilk_grainsize = 256") cilk_for

extern "C" int __cilkrts_internal_worker_id(void);

static int getWorkers() {
  return __cilkrts_get_nworkers();}
static int getWorkerId() {return __cilkrts_internal_worker_id();}
static void setWorkers(int n) { }
static void printScheduler() {
  cout << "scheduler = OpenCilk" << endl;
  cout << "num-threads = " << getWorkers() << endl;
}

//new syntax:

inline size_t num_workers() {
  return __cilkrts_get_nworkers();}

inline size_t worker_id() {
  return __cilkrts_internal_worker_id();}

template <class F>
inline void parallel_for(size_t start, size_t end, F f,
			 size_t granularity=0,
			 bool conservative=false) {
  if (end > start) {
    if (granularity == 1) {
      _Pragma("cilk_grainsize = 1") cilk_for(size_t i=start; i<end; ++i) f(i);
    } else if (granularity == 256) {
      _Pragma("cilk_grainsize = 256") cilk_for(size_t i=start; i<end; ++i) f(i);
    } else {
      cilk_for(size_t i=start; i<end; ++i) f(i);
    }
  }
}

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool conservative=false) {
  cilk_spawn left();
  right();
  cilk_sync;
}

#elif defined(CILK)

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#define parallel_main main
#define par_for cilk_for
#define par_for_1 _Pragma("cilk_grainsize = 1") cilk_for
#define par_for_256 _Pragma("cilk_grainsize = 256") cilk_for

static int getWorkers() {
  return __cilkrts_get_nworkers();}
static int getWorkerId() {return __cilkrts_get_worker_number();}
static void setWorkers(int n) { }
static void printScheduler() {
  cout << "scheduler = CilkPlus" << endl;
  cout << "num-threads = " << getWorkers() << endl;
}

//new syntax:

inline size_t num_workers() {
  return __cilkrts_get_nworkers();}

inline size_t worker_id() {
  return __cilkrts_get_worker_number();}

template <class F>
inline void parallel_for(size_t start, size_t end, F f,
			 size_t granularity=0,
			 bool conservative=false) {
  if (end > start) {
    if (granularity == 1) {
      _Pragma("cilk_grainsize = 1") cilk_for(size_t i=start; i<end; ++i) f(i);
    } else if (granularity == 256) {
      _Pragma("cilk_grainsize = 256") cilk_for(size_t i=start; i<end; ++i) f(i);
    } else {
      cilk_for(size_t i=start; i<end; ++i) f(i);
    }
  }
}

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool conservative=false) {
  cilk_spawn left();
  right();
  cilk_sync;
}

#elif defined(HOMEGROWN)

#include "scheduler.h"

namespace parlay {
  namespace internal {
    // Use a "Meyer singleton" to provide thread-safe
    // initialisation and destruction of the scheduler
    //
    // The declaration of get_default_scheduler must be
    // extern inline to ensure that there is only ever one
    // copy of the scheduler. This is guaranteed by the C++
    // standard: 7.1.2/4 A static local variable in an
    // extern inline function always refers to the same
    // object.
    extern inline fork_join_scheduler& get_default_scheduler() {
      static fork_join_scheduler fj;
      return fj;
    }
  }

  inline size_t num_workers() {
    return internal::get_default_scheduler().num_workers();
  }

  inline size_t worker_id() {
    return internal::get_default_scheduler().worker_id();
  }

  template <class F>
  inline void parallel_for(size_t start, size_t end, F f,
			   size_t granularity=0,
			   bool conservative=false) {
    if (end > start)
      internal::get_default_scheduler().parfor(start, end, f, granularity, conservative);
  }

  template <typename Lf, typename Rf>
  inline void par_do(Lf left, Rf right, bool conservative=false) {
    return internal::get_default_scheduler().pardo(left, right, conservative);
  }
}

using namespace parlay;

#define cilk_spawn
#define cilk_sync
#define parallel_main main
#define par_for for
#define par_for_1 for
#define par_for_256 for

static int getWorkers() {return (int)num_workers();}
static int getWorkerId() {return (int)worker_id();}
static void setWorkers(int n) { }
static void printScheduler() {
  cout << "scheduler = Parlay-HomeGrown" << endl;
  cout << "num-threads = " << getWorkers() << endl;}

#else

#define cilk_spawn
#define cilk_sync
#define parallel_main main
#define par_for for
#define par_for_1 for
#define par_for_256 for

static void printScheduler() {
  cout << "scheduler = sequential" << endl;}
static int getWorkers() {return 1;}
static int getWorkerId() {return 0;}
static void setWorkers(int n) { }

//new syntax:

inline size_t num_workers() {return 1;}
inline size_t worker_id() {return 0;}

template <class F>
inline void parallel_for(size_t start, size_t end, F f,
			 size_t granularity=0,
			 bool conservative=false) {
  if (end > start) {
    for(size_t i=start; i<end; ++i) f(i);}
}

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool conservative=false) {
  left();
  right();
}

#endif

#define nblocks(_n,_bsize) (1 + ((_n)-1)/(_bsize))

template <class F>
inline void blocked_for(intT _s, intT _e, intT _bsize, F f) {
  if (_e > _s) {
    intT _ss = _s;
    intT _ee = _e;
    intT _n = _ee-_ss;
    intT _l = nblocks(_n,_bsize);
    auto body = [&](intT _i) {
      intT _s = _ss + _i * (_bsize);
      intT _e = min(_s + (_bsize), _ee);
      f(_s, _e, _i);
    };
    parallel_for(0, _l, body);
  }
}

template <class F>
inline void granular_for(intT _s, intT _e, intT _thresh, F f, intT granularity=0) {
  if (_e - _s > _thresh)
    parallel_for(_s, _e, f, granularity);
  else
    for(intT i=_s; i<_e; ++i) f(i);
}

#endif

// Copyright (c) 2020-present Guy Blelloch and other contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef PARLAY_INTERNAL_WORK_STEALING_JOB_H_
#define PARLAY_INTERNAL_WORK_STEALING_JOB_H_

#include <cassert>

#include <atomic>

namespace parlay {

// Jobs are thunks -- i.e., functions that take no arguments
// and return nothing. Could be a lambda, e.g. [] () {}.

struct WorkStealingJob {
  WorkStealingJob() {
    done.store(false, std::memory_order_relaxed);
  }
  ~WorkStealingJob() = default;
  void operator()() {
    assert(done.load(std::memory_order_relaxed) == false);
    execute();
    done.store(true, std::memory_order_relaxed);
  }
  bool finished() {
    return done.load(std::memory_order_relaxed);
  }
  virtual void execute() = 0;
  std::atomic<bool> done;
};

// Holds a type-specific reference to a callable object
template<typename F>
struct JobImpl : WorkStealingJob {
  explicit JobImpl(F& _f) : WorkStealingJob(), f(_f) { }
  void execute() override {
    f();
  }
  F& f;
};

template<typename F>
JobImpl<F> make_job(F& f) { return JobImpl<F>(f); }

}

#endif  // PARLAY_INTERNAL_WORK_STEALING_JOB_H_

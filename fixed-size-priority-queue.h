// Copyright 2016  Junbo Zhang

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#ifndef FIXED_SIZE_PRIORITY_QUEUE_H_
#define FIXED_SIZE_PRIORITY_QUEUE_H_

#include <iostream>
#include <algorithm>
#include <vector>

/// A priority queue with fixed size. When the maximum size was reached,
/// the element with the lowest priority would be removed automatically.
/// Note that this class only supports point elements.
template<typename T>
class fixed_size_priority_queue
{
  public:
    fixed_size_priority_queue(size_t max_size) : max_size_(max_size) {}

    typedef typename std::vector<T>::iterator iterator;
    iterator begin() { return c_.begin(); }
    iterator end() { return c_.end(); }

    inline void push(const T &x) {
      if(c_.size() == max_size_) {
        if(x > *std::min_element(c_.begin(), c_.end())) {
            *std::min_element(c_.begin(), c_.end()) = x;
            std::make_heap(c_.begin(), c_.end());
        }
      }
      else {
        c_.push_back(x);
        std::make_heap(c_.begin(), c_.end());
      }
    }

    inline void pop() {
      if (c_.empty())
        return;
      std::pop_heap(c_.begin(), c_.end());
      c_.pop_back();
    }

    inline T& top() {
      return c_.front();
    }

    inline size_t size() {
      return c_.size();
    }

  protected:
    std::vector<T> c_;
    size_t max_size_;

  private:
    // heap allocation is not allowed
    void * operator new   (size_t);
    void * operator new[] (size_t);
    void   operator delete   (void *);
    void   operator delete[] (void*);
};

#endif  // FIXED_SIZE_PRIORITY_QUEUE_H_


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

#include <algorithm>
#include <vector>

/// A priority queue with fixed size. When the maximum size was reached,
/// the element with the lowest priority would be removed automatically.
/// Note that this class only supports point elements.
template<typename T>
class fixed_size_priority_queue 
{
  public:
    void set_size(unsigned int size) {
      fixed_size = size; 
    }
    void push(const T& x) 
    { 
      // If we've reached capacity, find the FIRST smallest object and replace
      // it if 'x' is larger
      if(c_.size() == fixed_size)
      {
        // 'c' is the container used by priority_queue and is a protected member.
        if(x > *std::min_element(c_.begin(), c_.end()))
        {
            *std::min_element(c_.begin(), c_.end()) = x;
            // Re-make the heap, since we may have just invalidated it.
            std::make_heap(c_.begin(), c_.end());
        }
      }
      // Otherwise just push the new item.
      else          
      {
        c_.push_back(x);
      }
    }

  private:
    std::vector<T> c_;
    unsigned int fixed_size;

    // Prevent heap allocation
    void * operator new   (size_t);
    void * operator new[] (size_t);
    void   operator delete   (void *);
    void   operator delete[] (void*);
};

#endif  // FIXED_SIZE_PRIORITY_QUEUE_H_


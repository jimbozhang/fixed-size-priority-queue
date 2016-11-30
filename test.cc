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

#include "fixed-size-priority-queue.h"
using namespace std;

class Foo {
  public:
    Foo (int a, float b) : a_(a), b_(b) {}
  
    friend inline std::ostream &operator<<(std::ostream &os, Foo &foo) {
      os << "(" << foo.a_ << ", " << foo.b_ << ")";
      return os;    
    }

    inline bool operator < (const Foo &other) const {
      return b_ < other.b_;
    }

    inline bool operator > (const Foo &other) const {
      return b_ > other.b_;
    }

  private:
    int a_;
    float b_;
};


template<typename T>
void print_queue(fixed_size_priority_queue<T> &q) {
    cout << "[size = " << q.size() << ", top = " << q.top() << "]";
    for (typename fixed_size_priority_queue<T>::iterator it = q.begin(); it != q.end(); it++) {
        cout << "\t" << *it;
    }
    cout << endl;
}

template<typename T>
void test(fixed_size_priority_queue<T> &q) {
  cout << "Init" << endl;
  print_queue(q);
  for (size_t i = 0; i < 8; i++) {
    q.pop();
    cout << "[" << i << "] pop" << endl;
    print_queue(q);
  }
}

int main(int argc, char const *argv[]) {
  fixed_size_priority_queue<int> q_simple(5);
  q_simple.push(2);
  q_simple.push(3);
  q_simple.push(1);
  q_simple.push(5);
  q_simple.push(5);
  q_simple.push(6);
  q_simple.push(2);
  q_simple.push(3);
  q_simple.push(1);
  q_simple.push(9);
  test(q_simple);

  fixed_size_priority_queue<Foo> q_complex(5);
  q_complex.push(Foo(2, 3));
  q_complex.push(Foo(3, 2));
  q_complex.push(Foo(1, 5));
  q_complex.push(Foo(5, 7));
  q_complex.push(Foo(5, 23));
  q_complex.push(Foo(6, 3));
  q_complex.push(Foo(2, 6));
  q_complex.push(Foo(3, 7));
  q_complex.push(Foo(1, 1));
  q_complex.push(Foo(9, 0));
  test(q_complex);
  
  fixed_size_priority_queue<Foo*> q_pointer(5);
  q_pointer.push(new Foo(2, 3));
  q_pointer.push(new Foo(3, 2));
  q_pointer.push(new Foo(1, 5));
  q_pointer.push(new Foo(5, 7));
  q_pointer.push(new Foo(5, 23));
  q_pointer.push(new Foo(6, 3));
  q_pointer.push(new Foo(2, 6));
  q_pointer.push(new Foo(3, 7));
  q_pointer.push(new Foo(1, 1));
  q_pointer.push(new Foo(9, 0));
  test(q_pointer);

  return 0;
}
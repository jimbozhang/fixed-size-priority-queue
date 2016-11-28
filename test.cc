// Copyright 2016       Lele Innovation and Intelligence Technology (Author: Junbo Zhang)

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

struct Foo {
    int a;
    float b;    
};

int main(int argc, char const *argv[])
{
    fixed_size_priority_queue<Foo> q1;

    fixed_size_priority_queue<Foo*> q2;

    return 0;
}
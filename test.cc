// Copyright 2016  Junbo Zhang
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

#include "MinMaxHeap.h"
#include <iostream>
#include <random>

using namespace std;

// A sample comparable object...
struct SomeThing
{
    // Some sample data...
    long int    m_A;
    bool        m_B;

    // Default constructor...
    SomeThing()
      : m_A(0),
        m_B(false)
    {
    }

    // Construct from an integer...
    SomeThing(const long int A)
      : m_A(A),
        m_B(true)
    {
    }

    // Compare with another object...
    bool operator<(const SomeThing &RightHandSide) const noexcept
    {
        return m_A < RightHandSide.m_A;
    }
};

int main()
{
    // Alert user...
    cout << "Checking min-max heap construction via Floyds' method..." << endl;

    // Number of sample elements...
    constexpr auto TotalElements = 10'000'000;

    // Populate a sample buffer of incrementing SomeThing values...
    cout << "Constructing sample buffer of size: " << TotalElements << endl;
    vector<SomeThing> SampleValues(TotalElements);
    iota(begin(SampleValues), end(SampleValues), 1l);

    // Seed a random generator and use generator to randomly arrange
    //  sample values so we don't inject in sorted order...
    cout << "Randomizing order..." << endl;
    random_device RandomDevice;
    mt19937 RandomGenerator(RandomDevice());
    shuffle(begin(SampleValues), end(SampleValues), RandomGenerator);

    // Build min-max heap using Floyd's method...
    cout << "Constructing heap via Floyd's method..." << endl;
    MinMaxHeap<SomeThing> SomeMinMaxHeap(
        begin(SampleValues), end(SampleValues));

    // As we unload the min-max heap, these are the ordinals our values
    //  are expected to carry...
    long int MinimumExpectedValue = 1l;
    long int MaximumExpectedValue = TotalElements;

    // Keep unloading, while not empty...
    cout << "Verifying heap";
    while(!SomeMinMaxHeap.IsEmpty())
    {
        // Show periodic status update...
        if((SomeMinMaxHeap.GetSize() % 1'000'000) == 0)
            cout << "." << flush;

        // Find, validate, delete and increment next expected minimum...
        const auto &CurrentMinimum = SomeMinMaxHeap.FindMinimum();
        assert(CurrentMinimum.m_A == MinimumExpectedValue++);
        SomeMinMaxHeap.DeleteMinimum();

        // Find, validate, delete and increment next expected maximum...
        const auto &CurrentMaximum = SomeMinMaxHeap.FindMaximum();
        assert(CurrentMaximum.m_A == MaximumExpectedValue--);
        SomeMinMaxHeap.DeleteMaximum();
    }

    // Done...
    cout << endl << endl;
}


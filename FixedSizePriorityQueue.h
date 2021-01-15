/*
    Kip Warner's min-max priority queue. 
*/

// Multiple include protection...
#ifndef _FIXED_SIZE_PRIORITY_QUEUE_H_
#define _FIXED_SIZE_PRIORITY_QUEUE_H_

// Includes...

    // Standard C++ / POSIX system headers...
    #include <algorithm>
    #include <functional>
    #include <iterator>
    #include <stdexcept>
    #include <vector>

    // Our headers...
    #include "MinMaxHeap.h"

// Min (or max) priority queue with a fixed capacity. Adding a new element when
//  the limit has been reached automatically drops the highest (or lowest)
//  priority element. This is implemented internally using a high performance
//  min-max heap...
template
<
    typename Type,
    bool MaxPriority = false,   /* Set to true to create a max priority instead of min. */
    typename LessThanComparisonType = decltype(std::less<Type>())
>
class FixedSizePriorityQueue
{
    // Public types...
    public:

        // Type aliase to internal container's iterator...
        using iterator = typename MinMaxHeap<Type, LessThanComparisonType>::iterator;

        // Type aliase to internal container's const iterator...
        using const_iterator = typename MinMaxHeap<Type, LessThanComparisonType>::const_iterator;

    // Public methods...
    public:

        // Construct a priority queue with a maximum capacity...
        explicit FixedSizePriorityQueue(
            const size_t MaximumCapacity,
            const LessThanComparisonType &LessThanCompare = LessThanComparisonType())
          : m_MaximumCapacity(MaximumCapacity),
            m_MinMaxHeap(LessThanCompare),
            m_LessThanComparison(LessThanCompare)
        {

        }

        // Construct a priority queue from another container and with a maximum
        //  size. O(N) space, O(N) and O(N lg N) best and worst case time
        //  complexity...
        template <typename InputForwardIteratorType>
        FixedSizePriorityQueue(
            InputForwardIteratorType First,
            InputForwardIteratorType Last,
            const size_t MaximumCapacity,
            const LessThanComparisonType &LessThanCompare = LessThanComparisonType())
          : m_MaximumCapacity(MaximumCapacity),
            m_MinMaxHeap(LessThanCompare),
            m_LessThanComparison(LessThanCompare)
        {
            // Verify the input iterator has forward capability. The
            //  std::distance() call requires it...
            static_assert(std::is_base_of_v<
                std::forward_iterator_tag,
                typename std::iterator_traits<InputForwardIteratorType>::iterator_category>,
                "Requires a forward capable iterator.");

            // How many elements does the source container have? This is a
            //  constant time operation if source supports random access...
            const size_t SourceElementCount = std::distance(First, Last);

            // Number of initial elements to copy directly into heap to benefit
            //  from Floyd's optimization...
            const size_t InitialCopyCount =
                std::min(SourceElementCount, m_MaximumCapacity);

            // Construct in linear time up until capacity...
            m_MinMaxHeap = MinMaxHeap<Type, LessThanComparisonType>(
                First, std::next(First, InitialCopyCount), m_LessThanComparison);

            // Iterator to track next element, if any, from source container...
            InputForwardIteratorType CurrentInput = std::next(First, InitialCopyCount);

            // Add each element into the min-max heap until we reach the end of
            //  the source...
            while(CurrentInput != Last)
            {
                // Insert a new element if not at capacity. If at capacity, only
                //  if priority requires an existing element to be dropped.
                //  O(lg N) time complexity...
                Insert(*CurrentInput);

                // Seek source to next one...
              ++CurrentInput;
            }
        }

        // Construct a new element directly in-place in logarithmic time if not
        //  already at capacity. If at capacity, add only if priority requires
        //  an existing element to be dropped. O(lg N) time complexity
        template <typename ... ArgumentTypes>
        void Emplace(ArgumentTypes &&... Arguments)
        {
            // We need to construct the new element before potential insertion
            //  because we need to perform a comparison against an existing
            //  element in order to determine whether to add this new one...
            Type Element(std::forward<ArgumentTypes>(Arguments)...);

            // If we haven't reached capacity yet, then always insert it...
            if(m_MinMaxHeap.GetSize() < m_MaximumCapacity)
                m_MinMaxHeap.Insert(std::move(Element));

            // Otherwise, if capacity reached, then it depends on what's in the
            //  priority queue already...
            else
            {
                // If user wanted a max priority queue...
                if constexpr(MaxPriority)
                {
                    // Create greater than comparison from the user's less than
                    //  comparison...
                    const auto GreaterThanComparison =
                        ComposeGreaterThanFromLessThan();

                    // ...then inject new element only if it is greater than the
                    //  lowest priority. Doing that discards the latter...
                    if(GreaterThanComparison(Element, m_MinMaxHeap.FindMinimum()))
                    {
                        // Add new element...
                        m_MinMaxHeap.Insert(std::move(Element));

                        // Remove minimum...
                        m_MinMaxHeap.DeleteMinimum();
                    }
                }

                // Or a min priority queue...
                else
                {
                    // ...then inject new element only if it is less than the
                    //  highest priority. Doing that discards the latter...
                    if(Element < m_MinMaxHeap.FindMaximum())
                    {
                        // Add new element...
                        m_MinMaxHeap.Insert(std::move(Element));

                        // Remove maximum...
                        m_MinMaxHeap.DeleteMaximum();
                    }
                }
            }
        }

        // Check if we're empty...
        [[nodiscard]] bool IsEmpty() const
        {
            return m_MinMaxHeap.IsEmpty();
        }

        // Get the current number of elements. O(1) time complexity...
        [[nodiscard]] auto GetSize() const noexcept
        {
            return m_MinMaxHeap.GetSize();
        }

        // Insert a new const lvalue element if not at capacity. If at capacity,
        //  only if priority requires an existing element to be dropped. O(lg N)
        //  time complexity...
        void Insert(const Type &Element)
        {
            Emplace(Element);
        }

        // Same as above, but optimized for an rvalue reference...
        void Insert(Type &&Element)
        {
            Emplace(std::move(Element));
        }

        // Get the highest priority element. O(1) time complexity...
        template <bool IsMaxPriority = MaxPriority>
        std::enable_if_t<IsMaxPriority, const Type &> PeekMaximum() const
        {
            return m_MinMaxHeap.FindMaximum();
        }

        // Get the highest priority element. O(1) time complexity...
        template <bool IsMaxPriority = MaxPriority>
        std::enable_if_t<!IsMaxPriority, const Type &> PeekMinimum() const
        {
            return m_MinMaxHeap.FindMinimum();
        }

        // Remove highest priority element. O(lg n) time complexity...
        template <bool IsMaxPriority = MaxPriority>
        std::enable_if_t<IsMaxPriority> PopMaximum()
        {
            // Remove it...
            m_MinMaxHeap.DeleteMaximum();
        }

        // Remove lowest priority element. O(lg n) time complexity...
        template <bool IsMaxPriority = MaxPriority>
        std::enable_if_t<!IsMaxPriority> PopMinimum()
        {
            // Remove it...
            m_MinMaxHeap.DeleteMinimum();
        }

        // Set the maximum capacity. O(N) time complexity...
        void SetMaximumCapacity(const size_t NewCapacity)
        {
            // Store new size...
            m_MaximumCapacity = NewCapacity;

            // If the new size would is less than current capacity, trim it...
            while(m_MinMaxHeap.GetSize() > m_MaximumCapacity)
            {
                // If user wanted a max priority queue, remove the lowest
                //  priority element...
                if constexpr(MaxPriority)
                    m_MinMaxHeap.DeleteMinimum();

                // Otherwise remove the maximum priority element...
                else
                    m_MinMaxHeap.DeleteMaximum();
            }
        }

    // Public iterators...
    public:

        // Retrieve an iterator...
        iterator begin() noexcept
        {
            return std::begin(m_MinMaxHeap);
        }

        // Retrieve a const iterator...
        const_iterator begin() const noexcept
        {
            return std::cbegin(m_MinMaxHeap);
        }

        // Retrieve a const iterator...
        const_iterator cbegin() const noexcept
        {
            return std::cbegin(m_MinMaxHeap);
        }

        // Retrieve an iterator end which is the next value after the last valid
        //  one...
        iterator end() noexcept
        {
            return std::end(m_MinMaxHeap);
        }

        // Retrieve a const iterator end which is the next value after the last
        //  valid one...
        const_iterator end() const noexcept
        {
            return std::cend(m_MinMaxHeap);
        }

        // Retrieve a const iterator end which is the next value after the last
        //  valid one...
        const_iterator cend() const noexcept
        {
            return std::cend(m_MinMaxHeap);
        }

    // Protected methods...
    protected:

        // Compose a greater than lambda from user's less than...
        constexpr auto ComposeGreaterThanFromLessThan() const noexcept
        {
            // We compose a greater than lambda from the user's less than
            //  without having to know anything about the type or how less than
            //  is implemented...
            return [this](const Type &LeftHandSide, const Type &RightHandSide)
                -> bool
            {
                // Swapping the arguments transforms lambda into a greater than
                //  comparison. This is possible because A < B iff B > A. Note
                //  that this axiom is only guaranteed to hold if A and B are of
                //  the same type...
                return m_LessThanComparison(RightHandSide, LeftHandSide);
            };
        }

    // Protected attributes...
    protected:

        // Maximum capacity of container...
        size_t                                      m_MaximumCapacity;

        // Min-max heap as the underlying container...
        MinMaxHeap<Type, LessThanComparisonType>    m_MinMaxHeap;

        // Less than comparison operator...
        LessThanComparisonType                      m_LessThanComparison;
};

// Alias templates specializations for minimum and maximum priority queues...

    // Maximum...
    template
    <
        typename Type,
        typename LessThanComparisonType = decltype(std::less<Type>())
    >
    using FixedSizeMaximumPriorityQueue =
        FixedSizePriorityQueue<Type, true, LessThanComparisonType>;

    // Minimum...
    template
    <
        typename Type,
        typename LessThanComparisonType = decltype(std::less<Type>())
    >
    using FixedSizeMinimumPriorityQueue =
        FixedSizePriorityQueue<Type, false, LessThanComparisonType>;

// Multiple include protection...
#endif


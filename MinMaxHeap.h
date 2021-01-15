/*
    Kip Warner's min-max priority queue. 
*/

// Multiple include protection...
#ifndef _MIN_MAX_HEAP_H_
#define _MIN_MAX_HEAP_H_

// Includes...

    // Standard C++ / POSIX system headers...
    #include <algorithm>
    #include <cassert>
    #include <climits>
    #include <cmath>
    #include <functional>
    #include <stdexcept>
    #include <vector>

    // Our headers...
    #include "NumericIterator.h"

// Min-max heap. This is just like a normal heap, except you can find both the
//  minimum and maximum values in constant time and deletions of either extremum
//  in logarithmic time. Insertions, like with a min heap, are logarithmic
//  complexity but faster by a constant factor...
template
<
    typename Type,
    typename LessThanComparisonType = decltype(std::less<Type>())
>
class MinMaxHeap
{
    // Public types...
    public:

        // Type aliase to internal container's iterator...
        using iterator = typename std::vector<Type>::iterator;

        // Type aliase to internal container's const iterator...
        using const_iterator = typename std::vector<Type>::const_iterator;

        // Type alias for difference type...
        using difference_type = typename std::vector<Type>::difference_type;

        // Type alias for direct addressing within the heap array...
        using NodeOffsetType = difference_type;

    // Public methods...
    public:

        // Construct an empty min-max heap...
        explicit MinMaxHeap(const LessThanComparisonType &LessThanCompare = LessThanComparisonType())
          : m_LessThanComparison(LessThanCompare)
        {
        }

        // Construct a min-max heap from the values in another container. This
        //  is O(N) complexity (assuming random access iterator) by using an
        //  adaption of Floyd's linear-time construction algorithm. His
        //  original algorithm worked from the bottom-up. This operation is
        //  sometimes called heapify...
        template <typename InputForwardIteratorType>
        MinMaxHeap(
            InputForwardIteratorType First,
            InputForwardIteratorType Last,
            const LessThanComparisonType &LessThanComparison = LessThanComparisonType());

        // Clear all elements...
        void Clear() noexcept
        {
            m_Heap.clear();
        }

        // Remove the highest priority element in logarithmic time...
        void DeleteMaximum();

        // Remove the lowest priority element in logarithmic time...
        void DeleteMinimum();

        // Construct a new element directly in-place in logarithmic time...
        template <typename ... ArgumentTypes>
        void Emplace(ArgumentTypes &&... Arguments);

        // Get the highest priority element in constant time...
        [[nodiscard]] const Type &FindMaximum() const
        {
            return GetValue(FindMaximumOffset());
        }

        // Get the lowest priority element in constant time...
        [[nodiscard]] const Type &FindMinimum() const
        {
            return GetValue(FindMinimumOffset());
        }

        // Get the underlying container...
        const std::vector<Type> &GetContainer() const noexcept
        {
            return m_Heap;
        }

        // Get the current number of elements in the underlying container...
        [[nodiscard]] typename std::vector<Type>::size_type GetSize() const noexcept
        {
            return m_Heap.size();
        }

        // Inserts a new const lvalue element in logarithmic time...
        void Insert(const Type &Element)
        {
            Emplace(Element);
        }

        // Same as above, but optimized for an rvalue reference...
        void Insert(Type &&Element)
        {
            Emplace(std::move(Element));
        }

        // Check if we're empty...
        [[nodiscard]] bool IsEmpty() const
        {
            return m_Heap.empty();
        }

    // Standard public iterators the user or STL expects...
    public:

        // Retrieve an iterator...
        iterator begin() noexcept
        {
            return std::begin(m_Heap);
        }

        // Retrieve a const iterator...
        const_iterator begin() const noexcept
        {
            return std::cbegin(m_Heap);
        }

        // Retrieve a const iterator...
        const_iterator cbegin() const noexcept
        {
            return std::cbegin(m_Heap);
        }

        // Retrieve an iterator end which is the next value after the last valid
        //  one...
        iterator end() noexcept
        {
            return std::end(m_Heap);
        }

        // Retrieve a const iterator end which is the next value after the last
        //  valid one...
        const_iterator end() const noexcept
        {
            return std::cend(m_Heap);
        }

        // Retrieve a const iterator end which is the next value after the last
        //  valid one...
        const_iterator cend() const noexcept
        {
            return std::cend(m_Heap);
        }

    // Protected types...
    protected:

        // Selector used to distinguish how algorithms should operate based on
        //  whether they are beginning on a minimum or maximum level...
        enum class LevelOrderingType
        {
            Minimum,
            Maximum
        };

    // Protected methods...
    protected:

        // Compute the floor of the base two integral logarithm. By floor, we
        //  simply mean the fractional portion is truncated.
        //
        // We can't use floorf(log2f(n)) instead because we need an exact
        //  answer. Not all integers can be accurately represented as a floating
        //  point when they get very large. A heap with a node at offset
        //  16,777,205 would have resulted in it being incorrectly located on
        //  level 23 because (floor2f(log2f(16'777'205+1) = 23.99999914)). But
        //  the correct answer is actually 24. If a level is calculated
        //  incorrectly, the algorithms may apply the incorrect ordering
        //  property on that level which breaks the heap...
        template <typename UnsignedIntegralType>
        static int BinaryLogarithmIntegerFloor(UnsignedIntegralType Value) noexcept
        {
            // The value's type must be an unsigned integral...
            static_assert(
                std::is_integral<UnsignedIntegralType>::value &&
                std::is_unsigned<UnsignedIntegralType>::value,
                "Operand must be unsigned integral value");

            // Bounds check input...
            assert(Value > 0);

            // Use optimized version if compiler has a built-in function for
            //  counting leading zeros. Benchmarking with GCC 10 and full
            //  optimizations on an i7 shows 3 % performance increase over
            //  generic implementation that follows...
            #ifdef HAVE___BUILTIN_CLZ

                // Width of input type in bits...
                constexpr size_t BitWidth =
                    sizeof(UnsignedIntegralType) * CHAR_BIT;

                // Number of leading bits before first toggled bit...
                int LeadingZeros = 0;

                // Use appropriate built-in for an unsigned integer to count
                //  leading zeros...
                if constexpr(std::is_same_v<unsigned int, UnsignedIntegralType>)
                    LeadingZeros = __builtin_clz(Value);

                // Use appropriate built-in for an unsigned long to count
                //  leading zeros...
                else if constexpr(std::is_same_v<unsigned long, UnsignedIntegralType>)
                    LeadingZeros = __builtin_clzl(Value);

                // Use appropriate built-in for an unsigned long long to count
                //  leading zeros...
                else if constexpr(std::is_same_v<unsigned long long, UnsignedIntegralType>)
                    LeadingZeros = __builtin_clzll(Value);

                // And if no built-in available for given width, raise error...
                else
                    static_assert(
                        std::is_same_v<false, UnsignedIntegralType>,
                        "No appropriate __builtin_clz for given width.");

                // Calculate the number of powers of two...
                const int PowersOfTwo = BitWidth - LeadingZeros - 1;

            // Otherwise use generic implementation...
            #else

                // Accumulator for number of times base had to be raised to
                //  become value...
                UnsignedIntegralType PowersOfTwo = 0;

                // Each time we bit shift the value to the right, it is the same
                //  as dividing by two...
                while(Value >>= 1)
                  ++PowersOfTwo;

            #endif

            // Return number of times two had to be raised to become value
            //  without fractional component...
            return PowersOfTwo;
        }

        // Compute the ceiling of the base two integral logarithm...
        template <typename UnsignedIntegralType>
        static int BinaryLogarithmIntegerCeil(UnsignedIntegralType Value) noexcept
        {
            // Bounds check input...
            assert(Value > 0);

            // Two raised to zeroth power is always one. Prevent undefined behaviour...
            if(Value == 1)
                return 0;

            // Compute result...
            return BinaryLogarithmIntegerFloor<UnsignedIntegralType>(Value - 1) + 1;
        }

        // Heapify given leaf node upwards until ordering properties
        //  satisfied...
        void BubbleUp(const NodeOffsetType NodeOffset);

        // Heapify given leaf node upwards until ordering properties
        //  satisfied. Given leaf is situated on the given level type. This is
        //  a recursive call...
        template <LevelOrderingType LevelOrdering>
        void BubbleUpImplementation(const NodeOffsetType NodeOffset);

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

        // Get the highest priority offset in constant time...
        NodeOffsetType FindMaximumOffset() const;

        // Get the lowest priority offset in constant time...
        NodeOffsetType FindMinimumOffset() const;

        // Get the grandparent of the given grandchild...
        const Type &GetGrandParent(const NodeOffsetType GrandChildOffset) const
        {
            return GetValue(GetParentOffset(GetParentOffset(GrandChildOffset)));
        }

        // Get the grandparent of the given grandchild...
        Type &GetGrandParent(const NodeOffsetType GrandChildOffset)
        {
            return GetValue(GetParentOffset(GetParentOffset(GrandChildOffset)));
        }

        // Get the grandparent offset of the given grandchild...
        NodeOffsetType GetGrandParentOffset(
            const NodeOffsetType GrandChildOffset) const noexcept
        {
            return GetParentOffset(GetParentOffset(GrandChildOffset));
        }

        // Get the height of the binary tree. A single node has a height of
        //  zero...
        typename std::vector<Type>::size_type GetHeight() const noexcept
        {
            return static_cast<typename std::vector<Type>::size_type>(
                BinaryLogarithmIntegerCeil(GetSize() + 1) - 1);
            /*return static_cast<typename std::vector<Type>::size_type>(
                ceil(log2(GetSize() + 1)) - 1);*/
        }

        // Get the offset of the last node, or throw an exception if there is
        //  none...
        NodeOffsetType GetLastNodeOffset() const
        {
            // If the heap is empty, it can't contain a last node...
            if(m_Heap.empty())
                throw std::out_of_range("No last node in an already empty heap.");

            // Otherwise calculate and return the offset of the last node...
            else
                return (m_Heap.size() - 1);
        }

        // Get the last sub-tree, or throw an exception if there is none...
        NodeOffsetType GetLastParentOffset() const
        {
            return GetParentOffset(GetLastNodeOffset());
        }

        // Get left child node of given parent's offset. Throws an exception if
        //  left child doesn't exist...
        const Type &GetLeftChild(const NodeOffsetType ParentOffset) const
        {
            return GetValue(GetLeftChildOffset(ParentOffset));
        }

        // Get offset of left child node of given parent's offset. Child may or
        //  may not exist...
        NodeOffsetType GetLeftChildOffset(
            const NodeOffsetType ParentOffset) const noexcept
        {
            return (2 * ParentOffset + 1);
        }

        // Get the level of the binary tree a node at the given offset would be
        //  located at, or negative if not valid...
        NodeOffsetType GetLevel(const NodeOffsetType NodeOffset) const noexcept
        {
            // Not valid, signal...
            if(!IsValidNode(NodeOffset))
                return -1;

            // Otherwise compute correct level...
            else
            {
                // To calculate fast integer binary logarithm, we need unsigned
                //  encoded integral value...
                const auto UnsignedNodeOffset = static_cast<unsigned>(NodeOffset);
                return BinaryLogarithmIntegerFloor(UnsignedNodeOffset + 1);
            }
            //return static_cast<NodeOffsetType>(floor(log2(NodeOffset + 1)));
        }

        // Get parent of given child's offset...
        const Type &GetParent(const NodeOffsetType ChildOffset) const
        {
            return GetValue(GetParentOffset(ChildOffset));
        }

        // Get parent of given child's offset...
        Type &GetParent(const NodeOffsetType ChildOffset)
        {
            return GetValue(GetParentOffset(ChildOffset));
        }

        // Get offset of parent node of given child's offset, or less than zero
        //  if none...
        NodeOffsetType GetParentOffset(
            const NodeOffsetType ChildOffset) const noexcept
        {
            // The root can't have a parent...
            if(ChildOffset == GetRootOffset())
                return -1;

            // Otherwise calculate correct offset of parent...
            else
                return ((ChildOffset - 1) / 2);
        }

        // Get right child node of given parent's offset. Throws an exception if
        //  no right child...
        const Type &GetRightChild(const NodeOffsetType ParentOffset) const
        {
            return GetValue(GetRightChildOffset(ParentOffset));
        }

        // Get offset of right child node of given parent's offset. Child may or
        //  may not exist...
        NodeOffsetType GetRightChildOffset(
            const NodeOffsetType ParentOffset) const noexcept
        {
            return (GetLeftChildOffset(ParentOffset) + 1);
        }

        // Get the offset of the root node...
        NodeOffsetType GetRootOffset() const noexcept
        {
            return 0;
        }

        // Get the value of a node, or throw an exception if index is invalid...
        const Type &GetValue(const NodeOffsetType NodeOffset) const
        {
            return m_Heap.at(NodeOffset);
        }

        // Get the value of a node, or throw an exception if index is invalid...
        Type &GetValue(const NodeOffsetType NodeOffset)
        {
            return m_Heap.at(NodeOffset);
        }

        // Perform a greater than comparison, or invert operands to perform less
        //  than...
        template <bool InvertRelationalOperator>
        bool GreaterThan(
            const Type &LeftHandSide, const Type &RightHandSide) const;

        // True if the second node is a child of the first...
        bool IsChild(
            const NodeOffsetType ParentOffset,
            const NodeOffsetType ChildOffset) const noexcept
        {
            // If at least one of the nodes doesn't exist, then the answer will always
            //  be false...
            if(!IsValidNode(ParentOffset) || !IsValidNode(ChildOffset))
                return false;

            // Check that the supposed child's parent is the one the user supplied...
            return (GetParentOffset(ChildOffset) == ParentOffset);
        }

        // True if the second node is a grandchild of the first...
        bool IsGrandChild(
            const NodeOffsetType GrandParentOffset,
            const NodeOffsetType GrandChildOffset) const noexcept
        {
            // If at least one of the nodes doesn't exist, then the answer will always
            //  be false...
            if(!IsValidNode(GrandParentOffset) || !IsValidNode(GrandChildOffset))
                return false;

            // Check that the supposed grandchild's grandparent is the one the user
            //  supplied...
            return (GetGrandParentOffset(GrandChildOffset) == GrandParentOffset);
        }

        // True if node has a grandparent...
        bool IsGrandParentExist(
            const NodeOffsetType GrandChildOffset) const noexcept
        {
            // A grandparent exists if its node offset is valid...
            return IsValidNode(GetGrandParentOffset(GrandChildOffset));
        }

        // True if given node's offset has a left child on the next level...
        bool IsLeftChildExist(const NodeOffsetType ParentOffset) const noexcept
        {
            // If the index into the heap is valid, the child exists...
            return (
                GetLeftChildOffset(ParentOffset) <
                static_cast<NodeOffsetType>(m_Heap.size()));
        }

        // True if the node at the given offset is located on a minimum level in
        //  the binary tree. These are levels 1, 3, 5...
        bool IsMaximumLevel(const NodeOffsetType NodeOffset) const noexcept
        {
            return !IsMinimumLevel(NodeOffset);
        }

        // True if the node at the given offset is located on a minimum level in
        //  the binary tree. These are levels 0, 2, 4...
        bool IsMinimumLevel(const NodeOffsetType NodeOffset) const noexcept
        {
            return (GetLevel(NodeOffset) % 2 == 0);
        }

        // True if the given node has a child at a preceding level...
        bool IsParentExist(const NodeOffsetType NodeOffset) const noexcept
        {
            // Calculate the offset of the parent...
            const NodeOffsetType ParentOffset = GetParentOffset(NodeOffset);

            // If the index into the heap is valid, the parent exists...
            return IsValidNode(ParentOffset);
        }

        // True if given node's offset has a right child on the next level...
        bool IsRightChildExist(const NodeOffsetType ParentOffset) const noexcept
        {
            // If the index into the heap is valid, the child exists...
            return (
                GetRightChildOffset(ParentOffset) <
                static_cast<NodeOffsetType>(m_Heap.size()));
        }

        // Check if a node's offset is valid...
        bool IsValidNode(const NodeOffsetType NodeOffset) const noexcept
        {
            return (
                (0 <= NodeOffset) &&
                (NodeOffset < static_cast<NodeOffsetType>(m_Heap.size())));
        }

        // Perform a less than comparison, or invert operands to perform greater
        //  than...
        template <bool InvertRelationalOperator>
        bool LessThan(const Type &LeftHandSide, const Type &RightHandSide) const;

        // Heapify given sub-tree...
        void TrickleDown(const NodeOffsetType SubTreeOffset);

        // Heapify given sub-tree that begins on a given level type. This is a
        //  recursive call...
        template <LevelOrderingType LevelOrdering>
        void TrickleDownImplementation(const NodeOffsetType SubTreeOffset);

    // Protected attributes...
    protected:

        // Array representation of internal heap. It is a binary tree
        //  implemented as an array with two properties. It is complete and it
        //  has the heap-order property. The latter means a parent node's value
        //  is always greater than both of its childrens'. It is complete when
        //  every level is full, except for possibly the last one in which the
        //  only vacancies are all on the right...
        std::vector<Type>       m_Heap;

        // Less than comparison operator...
        LessThanComparisonType  m_LessThanComparison;
};

// Inline definitions...
#include "MinMaxHeap.inl"

// Multiple include protection...
#endif


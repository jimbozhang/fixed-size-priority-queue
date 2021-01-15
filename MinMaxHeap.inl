/*
    Kip Warner's min-max priority queue. 
*/

// Construct a min-max heap from the values in another container. This is O(N)
//  complexity (assuming random access iterator) by using an adaption of Floyd's
//  linear-time construction algorithm. His original algorithm worked from the
//  bottom-up. This operation is sometimes called heapify...
template <typename Type, typename LessThanComparisonType>
template <typename InputForwardIteratorType>
MinMaxHeap<Type, LessThanComparisonType>::MinMaxHeap(
    InputForwardIteratorType First,
    InputForwardIteratorType Last,
    const LessThanComparisonType &LessThanComparison)
  : m_LessThanComparison(LessThanComparison)
{
    // Verify the input iterator has forward capability. The std::distance()
    //  call requires it...
    static_assert(std::is_base_of_v<
        std::forward_iterator_tag,
        typename std::iterator_traits<InputForwardIteratorType>::iterator_category>,
        "Requires a forward capable iterator.");

    // How many elements does the source container have? This is a constant time
    //  operation if source supports random access...
    const auto SourceElementCount = std::distance(First, Last);

    // Allocate enough reserve memory for heap, plus one for a new insertion...
    m_Heap.reserve(SourceElementCount + 1);
    m_Heap.resize(SourceElementCount);

    // Start by copying the source container into our heap. Our min-max heap is
    //  complete, but is assumed to not be correctly ordered at this point...
    std::copy(First, Last, std::begin(m_Heap));

    // If tree is empty or has only one node, then it already has the required
    //  min-max properties...
    if(GetSize() < 2)
        return;

    // Create an iterable range from the root to the very last sub-tree. This
    //  range can be traversed across every possible sub-tree within the tree...
    NumericRange<NodeOffsetType> SubTreeRange(
        GetRootOffset(),
        GetLastParentOffset() + 1);

    // Starting from the last sub-tree, keep heapifying until we have heapified
    //  every sub-tree all the way back up to the root...
    for(auto CurrentSubTreeIterator = std::rbegin(SubTreeRange);
        CurrentSubTreeIterator != std::rend(SubTreeRange);
      ++CurrentSubTreeIterator)
    {
        // Get the node offset for the current sub-tree...
        const NodeOffsetType SubTreeOffset = *CurrentSubTreeIterator;

        // Heapify given sub-tree...
        TrickleDown(SubTreeOffset);
    }
}

// Heapify given leaf node upwards until ordering properties satisfied...
template <typename Type, typename LessThanComparisonType>
void MinMaxHeap<Type, LessThanComparisonType>::BubbleUp(
    const NodeOffsetType NodeOffset)
{
    // If new leaf is currently on a minimum level, bubble up as far as
    //  necessary based on correct ordering properties...
    if(IsMinimumLevel(NodeOffset))
    {
        // If we haven't reached the top yet, and the node and its parent are
        //  out of order...
        if(IsParentExist(NodeOffset) &&
           GreaterThan<false>(GetValue(NodeOffset), GetParent(NodeOffset)))
        {
            // ...then correct their order. This is because the parent's level
            //  is a maximum level requiring the larger of the two nodes to be
            //  situated above...
            std::swap(GetValue(NodeOffset), GetParent(NodeOffset));

            // Continue bubbling the new node up from its new location on the
            //  maximum level above...
            BubbleUpImplementation<LevelOrderingType::Maximum>(GetParentOffset(NodeOffset));
        }

        // Otherwise, if we have already reached the top or the node and its
        //  parent are not out of order...
        else
        {
            // Continue checking the ordering property from the current level
            //  upwards...
            BubbleUpImplementation<LevelOrderingType::Minimum>(NodeOffset);
        }
    }

    // Otherwise if it is currently on a maximum level, bubble as far as
    //  necessary based on correct ordering properties...
    else
    {
        // If we haven't reached the top yet, and the node and its parent are
        //  out of order...
        if(IsParentExist(NodeOffset) &&
           LessThan<false>(GetValue(NodeOffset), GetParent(NodeOffset)))
        {
            // ...then correct their order. This is because the parent's level
            //  is a minimum level requiring the smaller of the two nodes to
            //  be situated above...
            std::swap(GetValue(NodeOffset), GetParent(NodeOffset));

            // Continue bubbling the new node up from its new location on the
            //  minimum level above...
            BubbleUpImplementation<LevelOrderingType::Minimum>(GetParentOffset(NodeOffset));
        }

        // Otherwise, if we have already reached the top or the node and its
        //  parent are not out of order...
        else
        {
            // Continue checking the ordering property from the current level
            //  upwards...
            BubbleUpImplementation<LevelOrderingType::Maximum>(NodeOffset);
        }
    }
}

// Heapify given leaf node upwards until ordering properties satisfied. Given
//  leaf is situated on the given level type. This is a recursive call...
template <typename Type, typename LessThanComparisonType>
template <typename MinMaxHeap<Type, LessThanComparisonType>::LevelOrderingType LevelOrdering>
void MinMaxHeap<Type, LessThanComparisonType>::BubbleUpImplementation(
    const NodeOffsetType NodeOffset)
{
    // Adapt our algorithm based on whether the new leaf node is located on a
    //  minimum or maximum level accordingly. By knowing this at compile time we
    //  can optimize...

        // Determine whether we need to invert relational operators...
        constexpr bool InvertRelationalOperator =
            (LevelOrdering == LevelOrderingType::Maximum);

        // If new leaf is currently on a minimum level, perform a sanity check
        //  that the node is actually at a minimum level...
        if constexpr(LevelOrdering == LevelOrderingType::Minimum)
            assert(IsMinimumLevel(NodeOffset));

        // On the other hand, sanity check that node is actually at a maximum
        //  level...
        else
            assert(IsMaximumLevel(NodeOffset));

    // If new node does not have a grandparent, then there is no next minimum
    //  (maximum) level two levels up and we have reached the terminating
    //  condition...
    if(!IsGrandParentExist(NodeOffset))
        return;

    // Reduction step that moves computation closer to the termination
    //  condition. Node and its grandparent are out of order, according to the
    //  rules of the level...
    if(LessThan<InvertRelationalOperator>(GetValue(NodeOffset), GetGrandParent(NodeOffset)))
    {
        // Correct the order by exchanging the nodes...
        std::swap(GetValue(NodeOffset), GetGrandParent(NodeOffset));

        // Recursive procedure calls itself. Hopefully this tail call is
        //  optimized by the compiler to produce a loop, but even if it
        //  isn't, the stack would scale only logarithmically. What we are
        //  trying to do here is continue bubbling the new node up from its new
        //  level using minimum ordering rules if applicable...
        if constexpr(LevelOrdering == LevelOrderingType::Minimum)
            BubbleUpImplementation<LevelOrderingType::Minimum>(
                GetGrandParentOffset(NodeOffset));

        // Otherwise continue bubbling the new node up from its new level using
        //  maximum ordering rules if applicable...
        else
            BubbleUpImplementation<LevelOrderingType::Maximum>(
                GetGrandParentOffset(NodeOffset));
    }
}

// Remove the highest priority element in logarithmic time...
template <typename Type, typename LessThanComparisonType>
void MinMaxHeap<Type, LessThanComparisonType>::DeleteMaximum()
{
    // Find the maximum element...
    const MinMaxHeap<Type, LessThanComparisonType>::NodeOffsetType
        MaximumOffset = FindMaximumOffset();

    // Move the last leaf node over the maximum and drop the old maximum...
    GetValue(MaximumOffset) = std::move(GetValue(GetLastNodeOffset()));
    m_Heap.pop_back();

    // Restore the min-max heapify properties since the order may be broken
    //  now. We start at the location of the replaced maximum and proceed
    //  down...
    TrickleDown(MaximumOffset);
}

// Remove the lowest priority element in logarithmic time...
template <typename Type, typename LessThanComparisonType>
void MinMaxHeap<Type, LessThanComparisonType>::DeleteMinimum()
{
    // No elements left to remove. This is an error...
    if(IsEmpty())
        throw std::out_of_range("Cannot delete minimum from an empty min-max heap.");

    // To remove the lowest priority element, move the last leaf node over the
    //  root element and drop the old root...
    m_Heap.front() = std::move(m_Heap.back());
    m_Heap.pop_back();

    // Restore the heapify property since ordering may be broken now...
    TrickleDown(GetRootOffset());
}

// Construct a new element directly in-place in logarithmic time...
template <typename Type, typename LessThanComparisonType>
template <typename ... ArgumentTypes>
void MinMaxHeap<Type, LessThanComparisonType>::Emplace(
    ArgumentTypes &&... Arguments)
{
    // Add element at the very back, potentially breaking heap property...
    m_Heap.emplace_back(std::forward<ArgumentTypes>(Arguments)...);

    // Percolate the element just added as a the very last leaf node up as far
    //  as necessary to restore ordering property...
    BubbleUp(GetLastNodeOffset());
}

// Get the highest priority offset in constant time...
template <typename Type, typename LessThanComparisonType>
typename MinMaxHeap<Type, LessThanComparisonType>::NodeOffsetType
MinMaxHeap<Type, LessThanComparisonType>::FindMaximumOffset() const
{
    // If there are no nodes, there can be no maximum...
    if(IsEmpty())
        throw std::out_of_range("Cannot find maximum in an empty min-max heap.");

    // If there is only one element, that must be the maximum...
    else if(GetSize() == 1)
        return GetRootOffset();

    // If the root has any children, the maximum is always the greater of its
    //  two. The child or children are situated on the first maximum level. This
    //  maximum level has the greatest values of all lower maximum levels...
    else
    {
        // If there is a right child and it is greater than the left child, then
        //  it is the maximum...
        if(IsRightChildExist(GetRootOffset()) &&
           GreaterThan<false>(GetRightChild(GetRootOffset()), GetLeftChild(GetRootOffset())))
            return GetRightChildOffset(GetRootOffset());

        // Otherwise the left child must be the maximum...
        else
            return GetLeftChildOffset(GetRootOffset());
    }
}

// Get the lowest priority in constant time...
template <typename Type, typename LessThanComparisonType>
typename MinMaxHeap<Type, LessThanComparisonType>::NodeOffsetType
MinMaxHeap<Type, LessThanComparisonType>::FindMinimumOffset() const
{
    // If there are no nodes, there can be no minimum...
    if(IsEmpty())
        throw std::out_of_range("Cannot find minimum in an empty min-max heap.");

    // The minimum is always the root node...
    return GetRootOffset();
}

// Perform a greater than comparison, or invert operands to perform less
//  than...
template <typename Type, typename LessThanComparisonType>
template <bool InvertRelationalOperator>
bool MinMaxHeap<Type, LessThanComparisonType>::GreaterThan(
    const Type &LeftHandSide, const Type &RightHandSide) const
{
    // If we're not being asked to invert the relational operator, then perform
    //  normal greater than comparison...
    if constexpr(!InvertRelationalOperator)
    {
        // Create greater than comparison...
        const auto GreaterThanComparison = ComposeGreaterThanFromLessThan();

        // Perform comparison...
        return GreaterThanComparison(LeftHandSide, RightHandSide);
    }

    // Otherwise invert the operands and use user's less than lambda...
    else
        return m_LessThanComparison(LeftHandSide, RightHandSide);
}

// Perform a less than comparison, or invert operands to perform greater
//  than...
template <typename Type, typename LessThanComparisonType>
template <bool InvertRelationalOperator>
bool MinMaxHeap<Type, LessThanComparisonType>::LessThan(
    const Type &LeftHandSide, const Type &RightHandSide) const
{
    // If we're not being asked to invert the relational operator, then perform
    //  normal less than comparison...
    if constexpr(!InvertRelationalOperator)
        return m_LessThanComparison(LeftHandSide, RightHandSide);

    // Otherwise invert the operands and use greater than lambda...
    else
    {
        // Create greater than comparison...
        const auto GreaterThanComparison = ComposeGreaterThanFromLessThan();

        // Perform comparison...
        return GreaterThanComparison(LeftHandSide, RightHandSide);
    }
}

// Heapify given sub-tree...
template <typename Type, typename LessThanComparisonType>
void MinMaxHeap<Type, LessThanComparisonType>::TrickleDown(
    const MinMaxHeap<Type, LessThanComparisonType>::NodeOffsetType SubTreeOffset)
{
    // If the sub-tree begins at a minimum level, restore ordering property...
    if(IsMinimumLevel(SubTreeOffset))
        TrickleDownImplementation<LevelOrderingType::Minimum>(SubTreeOffset);

    // Otherwise for maximum levels restore appropriate ordering property...
    else
        TrickleDownImplementation<LevelOrderingType::Maximum>(SubTreeOffset);
}

// Heapify given sub-tree that begins on a given level type. This is a recursive
//  call...
template <typename Type, typename LessThanComparisonType>
template <typename MinMaxHeap<Type, LessThanComparisonType>::LevelOrderingType LevelOrdering>
void MinMaxHeap<Type, LessThanComparisonType>::TrickleDownImplementation(
    const MinMaxHeap<Type, LessThanComparisonType>::NodeOffsetType SubTreeOffset)
{
    // Adapt our algorithm based on whether the root node is located on a
    //  minimum or maximum level accordingly. By knowing this at compile time we
    //  can optimize...

        // Determine whether we need to invert relational operators...
        constexpr bool InvertRelationalOperator =
            (LevelOrdering == LevelOrderingType::Maximum);

        // If we are applying minimum ordering property, sanity check root is at
        //  the expected level ordering......
        if constexpr(LevelOrdering == LevelOrderingType::Minimum)
            assert(IsMinimumLevel(SubTreeOffset));

        // On the other hand, if we are applying maximum ordering property,
        //  perform sanity check...
        else
            assert(IsMaximumLevel(SubTreeOffset));

    // Termination condition is when we hit a leaf node. Since the binary tree
    //  is always complete, if there is no left child, there can't be a right
    //   child - or any other nodes after...
    if(!IsLeftChildExist(SubTreeOffset))
        return;

    // Find the smallest of the children and grandchildren, if any, of node...

        // Retrieve the offsets for left and right children...
        const NodeOffsetType LeftChildOffset = GetLeftChildOffset(SubTreeOffset);
        const NodeOffsetType RightChildOffset = GetRightChildOffset(SubTreeOffset);

        // Start with the left child as the minimum (maximum), since we know
        //  this node has to exist...
        NodeOffsetType ExtremumOffset = LeftChildOffset;

        // Create a list of all six, at most, descendents to examine...
        /*              (P)
                      /     \
                     /       \
                 (LC)         (RC)
                /  \          /  \
               /    \        /    \
          (LCLC)   (LCRC) (RCLC)  (RCRC) */
        std::array<NodeOffsetType, 6> DescendentOffsets =
        {
            // Left child - must exist...
            LeftChildOffset,

            // Left child's left child...
            GetLeftChildOffset(LeftChildOffset),

            // Left child's right child...
            GetRightChildOffset(LeftChildOffset),

            // Right child...
            RightChildOffset,

            // Right child's left child...
            GetLeftChildOffset(RightChildOffset),

            // Right child's right child...
            GetRightChildOffset(RightChildOffset)
        };

        // Examine each extant descendent, looking for new minimum... (maximum)
        for(auto CurrentDescendentOffset : DescendentOffsets)
        {
            // If node doesn't exist, try next one...
            if(!IsValidNode(CurrentDescendentOffset))
                continue;

            // Otherwise examine it's value to see if it is less (greater) than
            //  current minimum... (maximum)
            if(LessThan<InvertRelationalOperator>(
                GetValue(CurrentDescendentOffset),
                GetValue(ExtremumOffset)))
                    ExtremumOffset = CurrentDescendentOffset;
        }

    // Reduction step that moves computation closer to the termination
    //  condition...

        // Check ordering property if the minimum (maximum) node was a
        //  grandchild...
        if(GetGrandParentOffset(ExtremumOffset) == SubTreeOffset)
        {
            // ...and the ordering property is currently not correct...
            if(LessThan<InvertRelationalOperator>(
                GetValue(ExtremumOffset),
                GetValue(SubTreeOffset)))
            {
                // ...then correct ordering property between parent and
                //  grandchild...
                std::swap(GetValue(ExtremumOffset), GetValue(SubTreeOffset));

                // If parent and child are ordered incorrectly, correct...
                if(GreaterThan<InvertRelationalOperator>(
                    GetValue(ExtremumOffset),
                    GetParent(ExtremumOffset)))
                        std::swap(GetValue(ExtremumOffset), GetParent(ExtremumOffset));
            }

            // Recursive procedure calls itself. Hopefully this tail call is
            //  optimized by the compiler to produce a loop, but even if it
            //  isn't, the stack would scale only logarithmically...
            TrickleDownImplementation<LevelOrdering>(ExtremumOffset);
        }

        // Check ordering property if the minimum (maximum) node was a child...
        else if(GetParentOffset(ExtremumOffset) == SubTreeOffset)
        {
            // Child and parent are out of order. Correct...
            if(LessThan<InvertRelationalOperator>(
                GetValue(ExtremumOffset),
                GetValue(SubTreeOffset)))
                    std::swap(GetValue(ExtremumOffset), GetValue(SubTreeOffset));
        }
}


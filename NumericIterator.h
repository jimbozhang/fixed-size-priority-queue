/*
    Kip Warner's min-max priority queue. 
*/

// Multiple include protection...
#ifndef _NUMERIC_ITERATOR_H_
#define _NUMERIC_ITERATOR_H_

// Includes...

    // Standard C++ / POSIX system headers...
    #include <cstddef>
    #include <iterator>
    #include <type_traits>
    #include <utility>

// Custom bidirectional iterator that iterates across integer values. Prior to
//  C++17, it was encouraged to inherit from std::iterator which would
//  automatically populate our class with all type definitions. This is
//  discouraged since C++17...
template <typename Type>
class NumericIterator
{
    // Verify underlying type is of some integral type...
    static_assert(std::is_integral<Type>::value);

    // Public traits...
    public:

        // Some compilers or iterators will complain if we don't provide
        //  appropriate iterator traits. These tell the STL that our iterator is
        //  of the bidirectional iterator category and can iterates over Type
        //  values...

            // Signed integer that can be used to identify distance between
            //  iterators...
            using difference_type   = std::ptrdiff_t;

            // Category iterator belongs to...
            using iterator_category = std::bidirectional_iterator_tag;

            // Type of object when iterator is dereferenced...
            using value_type        = Type;

            // Type of reference to the type iterated over...
            using reference         = Type &;

            // Pointer to type iterated over...
            using pointer           = Type *;

    // Public methods...
    public:

        // Direction increment operator could move positions...
        enum class DirectionType
        {
            Forwards,
            Backwards
        };

        // Explicit constructor initializes to given position...
        NumericIterator(
            const Type Position = 0,
            const DirectionType Direction = DirectionType::Forwards) noexcept
          : m_Position(std::move(Position)),
            m_Direction(Direction)
        {
        }

        // Dereference operator returns value of current position...
        const Type &operator*() const noexcept { return m_Position; }
        Type &operator*() noexcept { return m_Position; }

        // Class member access operator returns pointer to internal position value.
        //  These can't ever be used since we only allow integral types. It's
        //  strictly for conformance reasons...
        const Type *operator->() const noexcept { return &**this; }
        Type *operator->() noexcept { return &**this; }

        // Prefix increment operator...
        NumericIterator &operator++() noexcept
        {
            // Increment position if a forward iterator...
            if(m_Direction == DirectionType::Forwards)
              ++m_Position;

            // Backwards otherwise...
            else
              --m_Position;

            // Return reference to object...
            return *this;
        }

        // Postfix increment operator...
        NumericIterator operator++(int) noexcept
        {
            // Return previous state, incrementing our self...
            return std::exchange(*this, ++*this);
        }

        // Prefix decrement operator...
        NumericIterator &operator--() noexcept
        {
            // Decrement position if a forward iterator...
            if(m_Direction == DirectionType::Forwards)
              --m_Position;

            // Increment otherwise...
            else
              ++m_Position;

            // Return reference to object...
            return *this;
        }

        // Postfix decrement operator...
        NumericIterator operator--(int) noexcept
        {
            // Return previous state, decrementing our self...
            return std::exchange(*this, --*this);
        }

        // Inequality operator. This is used in for loops where the iterator is
        //  compared against the end iterator. If they are unequal, it will
        //  continue iterating...
        bool operator!=(const NumericIterator<Type> &RightHandSide) const noexcept
        {
            // It's true that the iterators are unequal if their positions
            //  differ...
            return m_Position != RightHandSide.m_Position;
        }

        // Equality operator...
        bool operator==(const NumericIterator<Type> &RightHandSide) const noexcept
        {
            // It's true that the iterators are equal if their positions are...
            return m_Position == RightHandSide.m_Position;
        }

    // Protected attributes...
    protected:

        // Current numeric position of iterator...
        Type m_Position;

        // Direction increment operator moves position...
        DirectionType m_Direction;
};

// Numeric range class for iterating through a range of integral values...
template <typename Type>
class NumericRange
{
    // Verify underlying type is of some integral type...
    static_assert(std::is_integral<Type>::value);

    // Public types...
    public:

        // Alias to numeric iterator type...
        using iterator = NumericIterator<Type>;

        // Alias to const numeric iterator type...
        using const_iterator = NumericIterator<Type>;

    // Public methods...
    public:

        // Constructor takes from and to range iterator will traverse...
        NumericRange(const Type From, const Type To)
          : m_From(std::move(From)),
            m_To(std::move(To))
        {
            // Bounds check...
            if(m_From > m_To)
                throw std::out_of_range("Start range was greater than end range with NumericRange.");
        }

        // Retrieve an iterator start...
        iterator begin() noexcept
        {
            return iterator(m_From);
        }

        // Retrieve a const iterator start...
        const_iterator begin() const noexcept
        {
            return const_iterator(m_From);
        }

        // Retrieve a const iterator start...
        const_iterator cbegin() const noexcept
        {
            return const_iterator(m_From);
        }

        // Retrieve the reverse iterator start...
        iterator rbegin() const noexcept
        {
            // What was the end now becomes the beginning...
            Type ReverseStart = m_To;

            // One less is the first valid value...
          --ReverseStart;

            // Construct the backwards iterator...
            return iterator(ReverseStart, iterator::DirectionType::Backwards);
        }

        // Retrieve an iterator end which is the next value after the last valid
        //  one...
        iterator end() noexcept
        {
            return iterator(m_To);
        }

        // Retrieve a const iterator end which is the next value after the last
        //  valid one...
        const_iterator end() const noexcept
        {
            return const_iterator(m_To);
        }

        // Retrieve a const iterator end which is the next value after the last
        //  valid one...
        const_iterator cend() const noexcept
        {
            return const_iterator(m_To);
        }

        // Retrieve reverse iterator end which is the next value after the last
        //  valid one...
        iterator rend() const noexcept
        {
            return iterator(m_From - 1, iterator::DirectionType::Backwards);

            // What was the start now becomes the end...
            Type ReverseEnd = m_From;

            // One less is the last valid value...
          --ReverseEnd;

            // Construct the backwards iterator...
            return iterator(ReverseEnd, iterator::DirectionType::Backwards);
        }

    // Protected attributes...
    protected:

        // Iterator starts at this value...
        Type m_From;

        // First number past the last number we can iterate to...
        Type m_To;
};

// Multiple include protection...
#endif


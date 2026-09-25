/**
 * @brief span class
 */

#pragma once

#include <cstddef>      // size_t
#include <stdexcept>    // out_of_range
#include <type_traits>  // remove_cv
#include <iterator>     // reverse_iterator



namespace qinmo
{

template<typename T>
class Span
{
public:
    // constants and types
    using element_type = T;
    using value_type = typename std::remove_cv<T>::type;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = element_type*;
    using const_pointer = const element_type*;
    using reference = element_type&;
    using const_reference = const element_type&;
    using iterator = element_type*;
    using const_iterator = const element_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
/*
                constructors, copy, move, and assignment
*/
    constexpr Span() noexcept : data_(nullptr), size_(0) { }
    constexpr Span(pointer data, size_type size) : data_(data), size_(size) { }
    constexpr Span(const Span&) noexcept = default;
    Span& operator=(const Span&) noexcept = default;
    constexpr Span(Span&&) noexcept = default;
    Span& operator=(Span&&) noexcept = default;


/*
                subview
*/
    /**
     * @note does not perform bounds-checking, ensure  count <= size
     */
    constexpr Span first(size_type count) const noexcept { return Span(data_, count); }
    /**
     * @note does not perform bounds-checking, ensure  count <= size
     */
    constexpr Span last(size_type count) const noexcept { return Span(data_ + size() - count, count); }
    /**
     * @note does not perform bounds-checking, ensure  count + offset <= size
     */
    constexpr Span subspan(size_type offset, size_type count) const noexcept { return Span(data_ + offset, count); }


/*
                observers
*/
    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type size_bytes() const noexcept { return size() * sizeof(T); }
    constexpr bool empty() const noexcept { return 0 == size_; }


/*
                element access
*/
    /**
     * @note does not perform bounds-checking, ensure  index < size
     */
    constexpr reference operator[](size_type index) const{ return data()[index]; }
    /**
     *  @note performs bounds-checking
     *  @note maybe have no  constexpr  in C++11
     */
    constexpr reference at(size_type index) const
    {
        if (index >= size())
            throw std::out_of_range("Span: index out of range.");

        return (*this)[index];
    }
    /**
     * @note does not perform bounds-checking, ensure  index < size
     * @note calling this on an empty Span is undefined behavior
     */
    constexpr reference front() const { return (*this)[0]; }
    /**
     * @note does not perform bounds-checking, ensure  index < size
     * @note calling this on an empty Span is undefined behavior
     */
    constexpr reference back() const { return (*this)[size() - 1]; }
    constexpr pointer data() const noexcept { return data_; }


/*
                iterator support
*/
    constexpr iterator begin() const noexcept { return data_; }
    constexpr iterator end() const noexcept { return data_ + size(); }
    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }
    constexpr reverse_iterator rbegin() const noexcept { return std::reverse_iterator(end()); }
    constexpr reverse_iterator rend() const noexcept { return std::reverse_iterator(begin()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

private:
    pointer data_;
    size_type size_;
};
} // namespace qinmo
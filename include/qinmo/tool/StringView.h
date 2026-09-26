/**
 * @brief string view class
 */

#pragma once

#include "StringConcat.h"
#include <string.h>
#include <stdint.h>



namespace qinmo
{

namespace detail
{

/// @note evaluated at compile time for literals, but may not be in other situations
constexpr std::size_t constexpr_strlen(const char* str)
{
    return ('\0' == *str ? 0 : 1 + constexpr_strlen(str + 1));
}
} // namespace detail



/// @brief string view class for fast string access
/// @note the origin string must not be freed during the lifetime of the view
class StringView
{
public:
    // constants and types
    using traits_type = std::char_traits<char>;
    using value_type = char;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using const_iterator = const value_type*;
    using iterator = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = const_reverse_iterator;
    static constexpr size_type npos = size_type(-1);

public:
    // constructors, copy, move, and assignment
    constexpr StringView() : data_(nullptr), size_(0) { }
    /// @note Ensure len <= string length
    constexpr StringView(const char* ptr, size_type len) : data_(ptr), size_(len) { }
    /// @note Ensure len <= string length
    constexpr StringView(const unsigned char* ptr, size_type len)
        : data_(static_cast<const char*>(static_cast<const void*>(ptr)))
        , size_(len)
    { }
    constexpr StringView(const char* first, const char* last) : data_(first), size_(static_cast<size_type>(last - first)) { }
    constexpr StringView(const unsigned char* first, const unsigned char* last)
        : data_(static_cast<const char*>(static_cast<const void*>(first)))
        , size_(static_cast<size_type>(last - first))
    { }
    /// @note evaluated at compile time for literals, but may not be in other situations
    constexpr StringView(const char* ptr)
        : data_(ptr)
        , size_(static_cast<size_type>((nullptr == ptr ? 0 : detail::constexpr_strlen(ptr))))
    { }
    /// @note evaluated at compile time for literals, but may not be in other situations
    constexpr StringView(const unsigned char* ptr)
        : data_(static_cast<const char*>(static_cast<const void*>(ptr)))
        , size_(static_cast<size_type>((nullptr == ptr ? 0 : detail::constexpr_strlen(static_cast<const char*>(static_cast<const void*>(ptr))))))
    { }
    StringView(const std::string& string)
        : data_(string.c_str())
        , size_(string.size())
    { }

public:
    // capacity
    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type length() const noexcept { return size(); }
    constexpr bool empty() const noexcept { return 0 == size(); }

    // element access
    constexpr const_reference operator[](size_type index) const noexcept { return *(data_ + index); }
    constexpr const_reference at(size_type index) const
    {
        if (index >= size())
            throw std::out_of_range(qinmo::concat("StringView::at: index(", index, ") out of range(0 ~ ", size() - 1, ')'));

        return *(data_ + index);
    }
    constexpr const_reference front() const noexcept { return (*this)[0]; }
    constexpr const_reference back() const noexcept { return (*this)[size() - 1]; }
    constexpr const char* data() const noexcept { return data_; }


    // string operations
    /// @throw std::out_of_range  when  pos > size
    size_type copy(char* s, size_type n, size_type pos = 0) const
    {
        if (pos > size())
            throw std::out_of_range(qinmo::concat("StringView::copy: pos(", pos, ") > size(", size(), ')'));

        const size_type count = std::min(n, size() - pos);
        traits_type::copy(s, data() + pos, count);
        return count;
    }
    /// @throw std::out_of_range  when  pos > size
    StringView substr(size_type pos = 0, size_type n = npos) const
    {
        if (pos > size())
            throw std::out_of_range(qinmo::concat("StringView::substr: pos(", pos, ") > size(", size(), ')'));

        return StringView(data() + pos, data() + std::min(pos + n, size()));
    }
    /// @throw std::out_of_range  when  pos > size
    StringView subview(size_type pos = 0, size_type n = npos) const
    {
        if (pos > size())
            throw std::out_of_range(qinmo::concat("StringView::subview: pos(", pos, ") > size(", size(), ')'));

        return StringView(data() + pos, data() + std::min(pos + n, size()));
    }


    // // deleted
    // void set(const char* ptr) { data_ = ptr; size_ = (nullptr == ptr ? 0 : ::strlen(ptr)); }
    // void set(const char* ptr, int len) { data_ = ptr; size_ = len; }
    // void clear() { data_ = nullptr; size_ = 0; }


    // iterator support
    constexpr const_iterator begin() const noexcept { return data_; }
    constexpr const_iterator end() const noexcept { return data_ + size(); }
    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }
    constexpr const_reverse_iterator rbegin() const noexcept { return std::reverse_iterator(end()); }
    constexpr const_reverse_iterator rend() const noexcept { return std::reverse_iterator(begin()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }


    /// @brief construct a string
    /// @note nonstandard
    std::string to_string() const { return std::string(data_, size_); }

private:
    const char* data_;
    size_type size_;

};

inline bool operator==(const StringView& a, const StringView& b) { return 0 == strncmp(a.data(), b.data(), a.size()); }
inline bool operator!=(const StringView& a, const StringView& b) { return !(a == b); }
inline std::ostream& operator<<(std::ostream& os, const StringView& view) { os << view.to_string(); return os; }

} // namespace qinmo
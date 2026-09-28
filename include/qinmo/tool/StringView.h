/**
 * @brief string view class
 */

#pragma once

#include "StringConcat.h"
#include <string.h>
#include <stdint.h>



namespace qinmo
{

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
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    static constexpr size_type npos = size_type(-1);

public:
    // constructors, copy, move, and assignment
    constexpr StringView() noexcept : data_(nullptr), size_(0) { }
    /// @note Ensure len <= string length
    constexpr StringView(const char* ptr, size_type len) noexcept : data_(ptr), size_(len) { }
    constexpr StringView(const char* first, const char* last) noexcept : data_(first), size_(static_cast<size_type>(last - first)) { }
    /// @note evaluated at compile time for literals, but may not be in other situations
    constexpr StringView(const char* ptr)
        : data_(ptr)
        , size_(static_cast<size_type>((nullptr == ptr ? 0 : traits_type::length(ptr))))
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

    // modifiers
    /// @note does not perform bounds-checking, ensure  n <= size
    void remove_prefix(size_type n) noexcept { data_ += n; size_ -= n; }
    /// @note does not perform bounds-checking, ensure  n <= size
    void remove_suffix(size_type n) noexcept { size_ -= n; }
    void swap(StringView& s) noexcept { std::swap(data_, s.data_); std::swap(size_, s.size_); }

    // element access
    constexpr const_reference operator[](size_type index) const noexcept { return *(data_ + index); }
    const_reference at(size_type index) const
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
    // constexpr int compare(StringView s) const noexcept { return traits_type::compare(data(), s.data(), std::min(size(), s.size())); }
    // constexpr int compare(size_type pos1, size_type n1, StringView s) const;
    // constexpr int compare(size_type pos1, size_type n1, StringView s, size_type pos2, size_type n2) const;  
    // constexpr int compare(const char* s) const;
    // constexpr int compare(size_type pos1, size_type n1, const char* s) const;
    // constexpr int compare(size_type pos1, size_type n1, const char* s, size_type n2) const;
    /// @return false  if  x.size() > size
    constexpr bool starts_with(StringView x) const noexcept { return size() >= x.size() && traits_type::compare(data(), x.data(), x.size()) == 0; }
    /// @return false  if  empty
    constexpr bool starts_with(char x) const noexcept { return !empty() && x == (*this)[0]; }
    /// @return false  if  nullptr or x.size() > size
    constexpr bool starts_with(const char* x) const noexcept { return nullptr != x && size() >= traits_type::length(x) && traits_type::compare(data(), x, size()) == 0; }
    /// @return false  if  x.size() > size
    constexpr bool ends_with(StringView x) const noexcept { return size() >= x.size() && traits_type::compare(data() + size() - x.size(), x.data(), x.size()) == 0; }
    /// @return false  if  empty
    constexpr bool ends_with(char x) const noexcept { return !empty() && x == (*this)[size() - 1]; }
    /// @return false  if  nullptr or x.size() > size
    constexpr bool ends_with(const char* x) const noexcept { return nullptr != x && size() >= traits_type::length(x); }


    // search
    /// @return npos  when  pos >= size()
    size_type find(StringView s, size_type pos = 0) const noexcept
    {
        if (pos > size())
            return npos;

        return find_default(data(), size(), s.data(), s.size());
    }
    /// @return npos  when  pos >= size()
    size_type find(char c, size_type pos = 0) const noexcept
    {
        for (size_type i = pos; i < size(); ++i)
        {
            if (c == (*this)[i])
                return i;
        }

        return npos;
    }
    /// @return npos  when  pos >= size()
    size_type find(const char* s, size_type pos, size_type n) const noexcept
    {
        return find(StringView(s, n), pos);
    }
    /// @return npos  when  pos >= size()
    size_type find(const char* s, size_type pos = 0) const
    {
        return find(StringView(s), pos);
    }
    size_type rfind(StringView s, size_type pos = npos) const noexcept;
    size_type rfind(char c, size_type pos = npos) const noexcept;
    size_type rfind(const char* s, size_type pos, size_type n) const noexcept;
    size_type rfind(const char* s, size_type pos = npos) const;


    // iterator support
    constexpr const_iterator begin() const noexcept { return data_; }
    constexpr const_iterator end() const noexcept { return data_ + size(); }
    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }
    /// @note not  constexpr  in c++11
    const_reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
    /// @note not  constexpr  in c++11
    const_reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }
    /// @note not  constexpr  in c++11
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    /// @note not  constexpr  in c++11
    const_reverse_iterator crend() const noexcept { return rend(); }


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
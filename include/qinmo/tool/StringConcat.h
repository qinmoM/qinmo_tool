/**
 * @brief string concatenation of parameter pack
 */

#pragma once

#include <string>
#include <sstream>
#include <iostream>



namespace qinmo
{

/// @brief concatenate parameters into a string
template <typename... Args>
std::string concat(Args&&... args)
{
    std::ostringstream oss;

    using Expander = int[];
    (void)Expander{ 0, (oss << std::forward<Args>(args), 0)... };

    return oss.str();
}


/// @note print the concatenated string
/// @note example : print(13, 't', "h")
template <typename... Args>
void print(Args&&... args)
{
    std::cout << concat(args...);
}

/// @note auto append newline char.
template <typename... Args>
void println(Args&&... args)
{
    print(args..., '\n');
}



/// @note evaluated at compile time for literals when possible
/// @note avoid using it for very long strings, as it uses recursive evaluation
constexpr std::size_t constexpr_strlen(const char* str) noexcept
{
    return ('\0' == *str ? 0 : 1 + constexpr_strlen(str + 1));
}

/// @return <0 if a is smaller, ==0 if equal, >0 if a is larger
/// @note evaluated at compile time for literals when possible
/// @note avoid using it for very long strings, as it uses recursive evaluation
constexpr int constexpr_compare(const char* a, const char* b, std::size_t count) noexcept
{
    return 0 == count
        ? 0
        : (*a == *b)
            ? constexpr_compare(a + 1, b + 1, count - 1)
            : (*a) - (*b);
}



/// @brief invalid index
constexpr std::size_t kFindNpos = std::size_t(-1);

/// @brief match strings using Brute-Force algorithm
/// @param haystack main string
/// @param needle pattern string
/// @return first matching index, qinmo::kFindNpos if not found
std::size_t find_BF(const char* haystack, std::size_t haystackSize, const char* needle, std::size_t needleSize) noexcept
{
    if (nullptr == haystack || nullptr == needle)
        return kFindNpos;

    if (0 == needleSize)
        return 0;

    std::size_t maxIndex = haystackSize - needleSize;
    for (std::size_t i = 0; i <= maxIndex; ++i)
    {
        std::size_t j = 0;
        for (; j < needleSize; ++j)
        {
            if (haystack[i + j] != needle[j])
                break;
        }

        if (j == needleSize)
            return i;
    }
    return kFindNpos;
}

/// @brief match strings using Boyer-Moore-Horstpool algorithm
/// @param haystack main string
/// @param needle pattern string
/// @return first matching index, qinmo::kFindNpos if not found
std::size_t find_BMH(const char* haystack, std::size_t haystackSize, const char* needle, std::size_t needleSize) noexcept
{
    if (nullptr == haystack || nullptr == needle || haystackSize < needleSize)
        return kFindNpos;

    if (0 == needleSize)
        return 0;

    std::size_t shift[256];
    for (int i = 0; i < 256; ++i)
        shift[i] = needleSize;

    for (int i = 0; i < needleSize - 1; ++i)
        shift[static_cast<unsigned char>(needle[i])] = needleSize - i - 1;

    std::size_t i = needleSize - 1;
    while (i < haystackSize)
    {
        std::size_t j = needleSize - 1, k = i;

        while (haystack[k] == needle[j])
        {
            if (0 == j)
                return k;

            k -= 1, j -= 1;
        }

        i += shift[static_cast<unsigned char>(haystack[i])];
    }

    return kFindNpos;
}

/// @brief match strings using KMP algorithm
/// @param haystack main string
/// @param needle pattern string
/// @return first matching index, qinmo::kFindNpos if not found
std::size_t find_KMP(const char* haystack, std::size_t haystackSize, const char* needle, std::size_t needleSize)
{
    if (nullptr == haystack || nullptr == needle)
        return kFindNpos;

    if (0 == needleSize)
        return 0;

    // next array
    std::size_t i = 0, j = kFindNpos;
    std::vector<std::size_t> next(needleSize, 0);
    next[0] = j;
    while (i < needleSize - 1)
    {
        if (kFindNpos != j && needle[i] != needle[j])
            j = next[j];

        i++, (j == kFindNpos) ? (j = 0) : (j += 1);
        if (needle[i] == needle[j])
            next[i] = next[j];
        else
            next[i] = j;
    }

    // search
    i = 0, j = 0;
    while (kFindNpos == j || i < haystackSize && j < needleSize)
    {
        if (kFindNpos == j || haystack[i] == needle[j])
            i++, (j == kFindNpos) ? (j = 0) : (j += 1);
        else
            j = next[j];
    }
    return (j == needleSize) ? i - j : kFindNpos;
}

} // namespace qinmo
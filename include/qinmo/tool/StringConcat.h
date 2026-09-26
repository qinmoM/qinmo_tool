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
constexpr std::size_t constexpr_strlen(const char* str)
{
    return ('\0' == *str ? 0 : 1 + constexpr_strlen(str + 1));
}

/// @return <0 if a is smaller, ==0 if equal, >0 if a is larger
/// @note evaluated at compile time for literals when possible
/// @note avoid using it for very long strings, as it uses recursive evaluation
constexpr int constexpr_compare(const char* a, const char* b, std::size_t count)
{
    return 0 == count
        ? 0
        : (*a == *b)
            ? constexpr_compare(a + 1, b + 1, count - 1)
            : (*a) - (*b);
}

} // namespace qinmo
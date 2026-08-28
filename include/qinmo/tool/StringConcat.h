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


/// @note auto append newline char.
/// @note example : println(13, 't', "h")
template <typename... Args>
void println(Args&&... args)
{
    std::cout << concat(args...) << '\n';
}

} // namespace qinmo
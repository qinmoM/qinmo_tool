#pragma once

#include <memory>
#include <utility>

namespace qinmo
{

/// @brief construct a std::unique_ptr of type T
template <
    class T,
    class... Args,
    typename std::enable_if<
        !std::is_array<T>::value,
        int
    >::type = 0
>
inline std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/// @brief construct a std::unique_ptr for dynamic array
/// @param num size of array
/// @note do not use non-standard compiler extension `T arr[0]`
template <
    class T,
    typename std::enable_if<
        std::is_array<T>::value &&
        std::extent<T>::value == 0,
        int
    >::type = 0
>
inline std::unique_ptr<T> make_unique(std::size_t num)
{
    using U = typename std::remove_extent<T>::type;
    return std::unique_ptr<T>(new U[num]());
}

/// @note do not use fixed-size array
/// @note do not use non-standard compiler extension `T arr[0]`
template <
    class T,
    class... Args,
    typename std::enable_if<
        std::is_array<T>::value &&
        std::extent<T>::value != 0,
        int
    >::type = 0
>
std::unique_ptr<T> make_unique(Args&&...) = delete;
} // namespace qinmo

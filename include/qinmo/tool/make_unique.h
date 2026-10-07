#pragma once

#include <memory>
#include <utility>

namespace qinmo
{

template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
} // namespace qinmo

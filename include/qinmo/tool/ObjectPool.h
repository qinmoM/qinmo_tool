#pragma once

#include <vector>

namespace qinmo
{

struct PoolOptions
{
    std::size_t maxSize_ = 1024;    // total limit
    std::size_t maxIdle_ = 64;      // max idle count
    std::size_t initSize_ = 0;      // initial capacity
    bool resetOnReturn_ = false;    // reset on release
    bool validateOnBorrow_ = false; // validate on acquire
    bool validateOnReturn_ = false; // validate on release
};

template <typename T>
class ObjectPool
{};
} // namespace qinmo

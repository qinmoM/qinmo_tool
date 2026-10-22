#pragma once

#include <functional>
#include <memory>

namespace qinmo
{

struct PoolOptions
{
    std::size_t maxSize_ = 1024U;       // total limit
    std::size_t maxIdle_ = 64U;         // max idle count
    std::size_t initSize_ = 0U;         // initial capacity
    bool resetOnReturn_ = false;        // reset on release
    bool validateOnBorrow_ = false;     // validate on acquire
    bool validateOnReturn_ = false;     // validate on release
};

template <typename T>
class ObjectPool
{
public:
    using CreateFunc = std::function<T()>;
    using CloseFunc = std::function<void(T&)>;
    using ResetFunc = std::function<void(T&)>;
    using ValidateFunc = std::function<bool(T&)>;

    static std::shared_ptr<ObjectPool<T>> getInstance(
        const PoolOptions& ops,
        CreateFunc createFunc = nullptr,
        CloseFunc closeFunc = nullptr,
        ResetFunc resetFunc = nullptr,
        ValidateFunc validateFunc = nullptr
    );

private:
    ObjectPool();
};
} // namespace qinmo

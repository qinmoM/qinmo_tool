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
struct PoolRules
{
    using CreateFunc = std::function<T()>;
    using CloseFunc = std::function<void(T&)>;
    using ResetFunc = std::function<void(T&)>;
    using ValidateFunc = std::function<bool(T&)>;

    CreateFunc create_ = nullptr;
    CloseFunc close_ = nullptr;
    ResetFunc reset_ = nullptr;
    ValidateFunc validate_ = nullptr;
};



template <typename T>
class ObjectPool
{
public:
    struct Builder
    {
        PoolOptions ops_;
        PoolRules<T> rs_;

        Builder& setMaxSize(std::size_t v);
        Builder& setMaxIdle(std::size_t v);
        Builder& setInitSize(std::size_t v);
        Builder& setResetOnReturn(bool v);
        Builder& setValidateOnBorrow(bool v);
        Builder& setValidateOnReturn(bool v);
        Builder& setFuncCreate(PoolRules::CreateFunc v);
        Builder& setFuncClose(PoolRules::CloseFunc v);
        Builder& setFuncReset(PoolRules::ResetFunc v);
        Builder& setFuncValidate(PoolRules::ValidateFunc v);

        std::shared_ptr<ObjectPool<T>> build();
    };

public:
    static std::shared_ptr<ObjectPool<T>> create(const PoolOptions& ops, const PoolRules& rs);

    static Builder builder();

private:
    ObjectPool();

    Builder builder_;
};
} // namespace qinmo

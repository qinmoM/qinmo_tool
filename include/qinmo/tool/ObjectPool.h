#pragma once

#include <functional>
#include <memory>
#include <stdexcept>



namespace qinmo
{

struct PoolOptions
{
    std::size_t maxSize = 1024U;       // total limit
    std::size_t maxIdle = 64U;         // max idle count
    std::size_t initSize = 0U;         // initial capacity
    bool resetOnReturn = false;        // reset on release
    bool validateOnBorrow = false;     // validate on acquire
    bool validateOnReturn = false;     // validate on release
};


template <typename T>
struct PoolRules
{
    using CreateFunc = std::function<T()>;
    using CloseFunc = std::function<void(T&)>;
    using ResetFunc = std::function<void(T&)>;
    using ValidateFunc = std::function<bool(T&)>;

    CreateFunc create = nullptr;
    CloseFunc close = nullptr;
    ResetFunc reset = nullptr;
    ValidateFunc validate = nullptr;
};



template <typename T>
class ObjectPool //  : public std::enable_shared_from_this<ObjectPool<T>>
{
public:
    struct Builder
    {
        PoolOptions ops_;
        PoolRules<T> rs_;

        Builder& setMaxSize(std::size_t v) { ops_.maxSize = v; return *this; }
        Builder& setMaxIdle(std::size_t v) { ops_.maxIdle = v; return *this; }
        Builder& setInitSize(std::size_t v) { ops_.initSize = v; return *this; }
        Builder& setResetOnReturn(bool v) { ops_.resetOnReturn = v; return *this; }
        Builder& setValidateOnBorrow(bool v) { ops_.validateOnBorrow = v; return *this; }
        Builder& setValidateOnReturn(bool v) { ops_.validateOnReturn = v; return *this; }
        Builder& setFuncCreate(typename PoolRules<T>::CreateFunc v) { rs_.create = std::move(v); return *this; }
        Builder& setFuncClose(typename PoolRules<T>::CloseFunc v) { rs_.close = std::move(v); return *this; }
        Builder& setFuncReset(typename PoolRules<T>::ResetFunc v) { rs_.reset = std::move(v); return *this; }
        Builder& setFuncValidate(typename PoolRules<T>::ValidateFunc v) { rs_.validate = std::move(v); return *this; }

        std::shared_ptr<ObjectPool<T>> build() { return std::shared_ptr<ObjectPool<T>>(new ObjectPool<T>(*this)); }
    };
    friend struct Builder;

public:
    static std::shared_ptr<ObjectPool<T>> create(const PoolOptions& ops, const PoolRules<T>& rs)
    {
        return std::shared_ptr<ObjectPool<T>>(new ObjectPool<T>(Builder{ ops, rs }));
    }

    static Builder builder()
    {
        return Builder{};
    }

private:
    ObjectPool(const Builder& builder)
        : builder_(builder)
    {
        if (builder.ops_.maxSize < builder.ops_.maxIdle || builder.ops_.maxIdle < builder.ops_.initSize)
            throw std::invalid_argument("maxSize must be >= maxIdle, maxIdle must be >= initSize");

        if (builder.ops_.resetOnReturn && nullptr == builder.rs_.reset)
            throw std::invalid_argument("reset function must be exist when reset on return");

        if ((builder.ops_.validateOnBorrow || builder.ops_.validateOnReturn) && nullptr == builder.rs_.validate)
            throw std::invalid_argument("validate function must be exist when validate on borrow or return");
    }

    const Builder builder_;
};
} // namespace qinmo

#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>



namespace qinmo
{

struct PoolOptions
{
    std::size_t maxSize = 1024U;       // total limit
    std::size_t maxIdle = 64U;         // max idle count
    std::size_t initSize = 0U;         // initial capacity
    bool resetOnReturn = false;        // reset on release
    bool validateOnCreate = false;     // validate on create
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
class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>>
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
        Builder& setValidateOnCreate(bool v) { ops_.validateOnCreate = v; return *this; }
        Builder& setValidateOnReturn(bool v) { ops_.validateOnReturn = v; return *this; }
        Builder& setFuncCreate(typename PoolRules<T>::CreateFunc v) { rs_.create = std::move(v); return *this; }
        Builder& setFuncClose(typename PoolRules<T>::CloseFunc v) { rs_.close = std::move(v); return *this; }
        Builder& setFuncReset(typename PoolRules<T>::ResetFunc v) { rs_.reset = std::move(v); return *this; }
        Builder& setFuncValidate(typename PoolRules<T>::ValidateFunc v) { rs_.validate = std::move(v); return *this; }

        std::shared_ptr<ObjectPool<T>> build() { return std::shared_ptr<ObjectPool<T>>(new ObjectPool<T>(*this)); }
    };
    friend struct Builder;

private:
    struct Deleter
    {
        void operator()(T* ptr) const
        {
            if (nullptr == ptr)
                return;

            std::shared_ptr<ObjectPool<T>> poolPtr = pool.lock();

            if (poolPtr)
                poolPtr->reclaim(ptr);
            else
                delete ptr;
        }

        std::weak_ptr<ObjectPool<T>> pool;
    };
    friend class Deleter;

public:
    static std::shared_ptr<ObjectPool<T>> create(const PoolOptions& ops, const PoolRules<T>& rs)
    {
        return std::shared_ptr<ObjectPool<T>>(new ObjectPool<T>(Builder{ ops, rs }));
    }

    static Builder builder()
    {
        return Builder{ };
    }

    ~ObjectPool() { close(); }



    std::unique_ptr<T, Deleter> acquire()
    {
        return acquire_impl(true, std::chrono::time_point{ });
    }

    template <typename Req, typename Period>
    std::unique_ptr<T, Deleter> acquire(std::chrono::duration<Req, Period> timeout)
    {
        return acquire_impl(true, std::chrono::steady_clock::now() + timeout);
    }

    std::unique_ptr<T, Deleter> try_acquire()
    {
        return acquire_impl(false, std::chrono::time_point{ });
    }



    bool isClosed()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return close_;
    }

    void close()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        close_ = true;
        for (T* ptr : queue_)
            delete ptr;
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

        queue_.reserve(builder.ops_.maxSize);

        for (std::size_t i = 0; i < builder.ops_.initSize; ++i)
            queue_.push_back(new T(std::move(builder.rs_.create())));
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;


    void reclaim(T* ptr)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (isClosed() || queue_.size() == builder_.ops_.maxIdle || (builder_.ops_.validateOnReturn && !builder_.rs_.validate()))
            { 
                count_.fetch_sub(1);
                delete ptr;
                return;
            }

            if (builder_.ops_.resetOnReturn)
                builder_.rs_.reset(*ptr);

            queue_.push_back(ptr);
        }

        cv_.notify_one();
    }


    std::unique_ptr<T, Deleter> acquire_impl(bool needWait, std::chrono::time_point deadline)
    {
        // std::unique_lock<std::mutex> lock(mutex_);
        // std::size_t count = count_.load(std::memory_order_acquire);
        // Deleter deleter;
        // deleter.pool = std::weak_ptr<ObjectPool>(this->shared_from_this());

        // if (queue_.empty())
        // {
        //     if (count != builder_.ops_.maxSize)
        //     {
        //         count_.fetch_add(1);
        //         return std::unique_ptr<T, Deleter>(new T(std::move(builder.rs_.create())), deleter);
        //     }

        //     cv_.wait(lock, [this, count]() -> bool { return !queue_.empty(); });
        // }

        // T* temp = queue_.back();
        // queue_.pop_back();
        // return std::unique_ptr<T, Deleter>((builder_.rs_.validate(*temp) ? temp : nullptr), deleter);
    }

private:
    const Builder builder_;
    std::vector<T*> queue_;
    std::size_t count_{ 0 };
    bool close_{ false };

    std::mutex mutex_;
    std::condition_variable cv_;

};
} // namespace qinmo

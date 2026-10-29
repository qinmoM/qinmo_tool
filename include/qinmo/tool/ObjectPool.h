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
    bool resetOnRelease = false;       // reset on release
    bool validateOnCreate = false;     // validate on create
    bool validateOnAcquire = false;    // validate on acquire
    bool validateOnRelease = false;    // validate on release
};


template <typename T>
struct PoolRules
{
    using CreateFunc = std::function<T()>;
    using CloseFunc = std::function<void(T&)>;
    using ResetFunc = std::function<void(T&)>;
    using ValidateFunc = std::function<bool(T&)>;

    CreateFunc create = nullptr;        // object creation function
    CloseFunc close = nullptr;          // object closure function
    ResetFunc reset = nullptr;          // reset state when returned
    ValidateFunc validate = nullptr;    // check whether the object is valid
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
        Builder& setResetOnRelease(bool v) { ops_.resetOnRelease = v; return *this; }
        Builder& setValidateOnAcquire(bool v) { ops_.validateOnAcquire = v; return *this; }
        Builder& setValidateOnCreate(bool v) { ops_.validateOnCreate = v; return *this; }
        Builder& setValidateOnRelease(bool v) { ops_.validateOnRelease = v; return *this; }
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
            {
                poolPtr->release(ptr);
            }
            else
            {
                state->rs_.close(*ptr);
                delete ptr;
            }
        }

        std::shared_ptr<Builder> state;
        std::weak_ptr<ObjectPool<T>> pool;
    };
    friend class Deleter;

public:
    ~ObjectPool() { close(); }


    /// @brief create a pool with `options` and `rules`
    static std::shared_ptr<ObjectPool<T>> create(const PoolOptions& ops, const PoolRules<T>& rs)
    {
        return std::shared_ptr<ObjectPool<T>>(new ObjectPool<T>(Builder{ ops, rs }));
    }

    /// @brief return a builder
    static Builder builder()
    {
        return Builder{ };
    }

public:
    using UniquePtr = std::unique_ptr<T, Deleter>;
    using Clock = std::chrono::steady_clock;

    /// @brief acquire object with blocking
    /// @return object's unique pointer
    UniquePtr acquire()
    {
        return acquire_impl(true, Clock::time_point{ });
    }

    /// @brief acquire object with blocking, return upon timeout
    /// @param timeout relative time, such as `std::chrono::milliseconds(1)`
    /// @return object's unique pointer
    template <typename Req, typename Period>
    UniquePtr acquire(std::chrono::duration<Req, Period> timeout)
    {
        return acquire_impl(true, Clock::now() + timeout);
    }

    /// @brief non-blocking acquire of object
    /// @return object's unique pointer
    UniquePtr try_acquire()
    {
        return acquire_impl(false, Clock::time_point{ });
    }



    /// @return true if the pool is closed
    bool isClosed()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return close_;
    }

    /// @brief close this pool
    /// @note object cannot be acquired after `close`
    void close()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (close_)
                return;

            close_ = true;
            for (T* ptr : queue_)
            {
                builder_->rs_.close(*ptr);
                delete ptr;
            }

            queue_.clear();
        }
        cv_.notify_all();
    }

private:
    ObjectPool(const Builder& builder)
        : builder_(new Builder(builder))
    {
        if (builder.ops_.maxSize < builder.ops_.maxIdle || builder.ops_.maxIdle < builder.ops_.initSize)
            throw std::invalid_argument("maxSize must be >= maxIdle, maxIdle must be >= initSize");

        if (builder.ops_.resetOnRelease && nullptr == builder.rs_.reset)
            throw std::invalid_argument("reset function must be exist when reset on return");

        if ((builder.ops_.validateOnAcquire || builder.ops_.validateOnRelease || builder.ops_.validateOnCreate) && nullptr == builder.rs_.validate)
            throw std::invalid_argument("validate function must be exist when validate on borrow or return");

        queue_.reserve(builder.ops_.maxSize);

        for (std::size_t i = 0; i < builder.ops_.initSize; ++i)
        {
            queue_.push_back(createPtr());
            count_ += 1;
        }
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

private:
    void release(T* ptr)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (close_ || queue_.size() == builder_->ops_.maxIdle
                || (builder_->ops_.validateOnRelease && !builder_->rs_.validate(*ptr)))
            { 
                count_ -= 1;
                builder_->rs_.close(*ptr);
                delete ptr;
                return;
            }

            if (builder_->ops_.resetOnRelease)
                builder_->rs_.reset(*ptr);

            queue_.push_back(ptr);
        }

        cv_.notify_all();
    }


    UniquePtr acquire_impl(bool needWait, Clock::time_point deadline)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        while (true)
        {
            if (close_)
                return nullptr;

            if (UniquePtr obj = tryGetIdle())
                return obj;

            if (UniquePtr obj = tryGetCreate())
                return obj;

            if (!needWait)
                return nullptr;

            if (deadline != Clock::time_point{ } && Clock::now() >= deadline)
                return nullptr;

            if (cv_.wait_until(lock, deadline) == std::cv_status::timeout)
                return nullptr;
        }
    }

    /// @brief return a heap object pointer
    /// @note default construct without creator
    T* createPtr()
    {
        if (builder_->rs_.create)
            return new T(std::move(builder_.rs_.create()));
        else
            return new T();
    }

    UniquePtr convertTo(T* ptr)
    {
        Deleter deleter;
        deleter.pool = std::weak_ptr<ObjectPool>(this->shared_from_this());
        deleter.state = builder_;
        return UniquePtr(ptr, deleter);
    }

    /// @brief try to take a object from queue
    /// @note must be called under lock
    UniquePtr tryGetIdle()
    {
        while (!queue_.empty())
        {
            T* temp = queue_.back();
            queue_.pop_back();

            if (!builder_->ops_.validateOnAcquire || builder_->rs_.validate(*temp))
                return convertTo(temp);

            count_ -= 1;
            builder_->rs_.close(*temp);
            delete temp;
        }

        return nullptr;
    }

    /// @brief try to take a object from queue
    /// @note must be called under lock
    UniquePtr tryGetCreate()
    {
        if (count_ != builder_->ops_.maxSize)
        {
            T* temp = createPtr();
            if (!builder_->ops_.validateOnCreate || builder_->rs_.validate(*temp))
            {
                count_ += 1;
                return convertTo(temp);
            }

            builder_->rs_.close(temp);
            delete temp;
        }

        return nullptr;
    }

private:
    std::shared_ptr<Builder> builder_;
    std::vector<T*> queue_;
    std::size_t count_{ 0 };
    bool close_{ false };

    std::mutex mutex_;
    std::condition_variable cv_;

};
} // namespace qinmo

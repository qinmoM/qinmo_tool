/**
 * @brief span class
 */

#pragma once

#include <initializer_list>
#include <utility>



namespace qinmo
{

struct nullopt_t
{
    enum class Construct { Token };

    explicit constexpr nullopt_t(Construct) noexcept { }
};

extern const nullopt_t nullopt;



template<typename T>
class Optional
{
public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

public:
/*
                constructors
*/
    constexpr Optional() noexcept { }
    constexpr Optional(nullopt_t) noexcept { }
    Optional(const Optional& other)
    {
        if (other.has_value())
        {
            new (value_) T(other.value());
            valid_ = true;
        }
    }
    /// @note remains valid after move, reset() must be called manually after move
    Optional(Optional&& other) noexcept(std::is_nothrow_constructible<T>::value)
    {
        if (other.has_value())
        {
            new (value_) T(std::move(other.value()));
            valid_ = true;
        }
    }

/*
                destructor
*/
    ~Optional() { reset(); }


/*
                assignment
*/
    constexpr Optional& operator=(nullopt_t) noexcept { reset(); }
    Optional& operator=(const Optional& other)
    {
        if (other.has_value())
        {
            new (value_) T(other.value());
            valid_ = true;
        }
        return *this;
    }
    Optional& operator=(Optional&& other) noexcept(std::is_nothrow_constructible<T>::value)
    {
        if (other.has_value())
        {
            new (value_) T(std::move(other.value()));
            valid_ = true;
        }
        return *this;
    }
    template<class... Args>
    T& emplace(Args&&... args)
    {
        reset();

        new (value_) T(std::forward<Args>(args)...);
        valid_ = true;
        return *ptr();
    }
    template<class U, class... Args>
    T& emplace(std::initializer_list<U> list, Args&&... args)
    {
        reset();

        new (value_) T(list, std::forward<Args>(args)...);
        valid_ = true;
        return *ptr();
    }


/*
                iterator support
*/
    iterator begin() noexcept { return ptr(); }
    constexpr const_iterator begin() const noexcept { return begin(); }
    iterator end() noexcept { return has_value() ? ptr() + 1 : ptr(); }
    constexpr const_iterator end() const noexcept { return end(); }


/*
                observers
*/
    constexpr bool has_value() const noexcept { return valid_; }
    T& value() & { return *ptr(); }
    constexpr const T& value() const & { return *ptr(); }
    T&& value() && { return *ptr(); }
    constexpr const T&& value() const && { return *ptr(); }


/*
                modifiers
*/
    void reset() noexcept
    {
        if (has_value())
        {
            ptr()->~T();
            valid_ = false;
        }
    }

private:
    T* ptr() noexcept { return reinterpret_cast<T*>(value_); }
    constexpr const T* ptr() const noexcept { return reinterpret_cast<const T*>(value_); }

    alignas(T) unsigned char value_[sizeof(T)];
    bool valid_ = false;

};
} // namespace qinmo

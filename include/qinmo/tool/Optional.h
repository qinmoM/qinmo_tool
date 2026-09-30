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

public:
/*
                constructors
*/
    constexpr Optional() noexcept { }
    constexpr Optional(nullopt_t) noexcept { }

/*
                destructor
*/
    ~Optional() { reset(); }


/*
                assignment
*/
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
        if (valid_)
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

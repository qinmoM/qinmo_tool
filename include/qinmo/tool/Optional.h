/**
 * @brief span class
 */

#pragma once

#include <initializer_list>
#include <utility>
#include <stdexcept>



namespace qinmo
{

struct nullopt_t
{
    enum class Construct { Token };

    explicit constexpr nullopt_t(Construct) noexcept { }
};

extern const nullopt_t nullopt;


/// @brief custom Optional access exception
class bad_optional_access : public std::exception
{
public:
    ~bad_optional_access() noexcept override = default;

    const char* what() const noexcept override
    {
        return "bad optional access";
    }
};



template<typename T>
class Optional
{
public:
    // types
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
        reset();

        if (other.has_value())
        {
            new (value_) T(*other);
            valid_ = true;
        }
    }
    /// @note remains valid after move, reset() must be called manually after move
    Optional(Optional&& other) noexcept(std::is_nothrow_constructible<T>::value)
    {
        reset();

        if (other.has_value())
        {
            new (value_) T(std::move(*other));
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
        reset();

        if (other.has_value())
        {
            new (value_) T(*other);
            valid_ = true;
        }

        return *this;
    }
    Optional& operator=(Optional&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
        reset();

        if (other.has_value())
        {
            new (value_) T(std::move(*other));
            valid_ = true;
        }

        return *this;
    }
    template<class U = typename std::remove_cv<T>::type>
        Optional& operator=(U&& value)
        {
            if (has_value())
                **this = std::forward<U>(value);
            else
                emplace(std::forward<U>(value));
            return *this;
        }
    template<class U>
        typename std::enable_if<
            !std::is_same<T, U>::value &&
            std::is_constructible<T, const U&>::value &&
            std::is_assignable<T&, const U&>::value,
            Optional&
        >::type operator=(const Optional<U>& other)
        {
            if (has_value() && other.has_value())
                **this = *other;
            else if (has_value())
                reset();
            else if (other.has_value())
                emplace(*other);

            return *this;
        }
    template<class U>
        typename std::enable_if<
            !std::is_same<T, U>::value &&
            std::is_constructible<T, U&&>::value &&
            std::is_assignable<T&, U&&>::value,
            Optional&
        >::type operator=(Optional<U>&& other)
        {
            if (has_value() && other.has_value())
                **this = std::move(*other);
            else if (has_value())
                reset();
            else if (other.has_value())
                emplace(std::move(*other));

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
                swap
*/
    void swap(Optional& other) /* noexcept() */
    {
        if (this == &other)
            return;

        if (has_value() && other.has_value())
        {
            using std::swap;
            swap(**this, *other);
        }
        else if (has_value())
        {
            other = std::move(*this);
            reset();
        }
        else if (other.has_value())
        {
            *this = std::move(other);
            other.reset();
        }
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
    T* operator->() noexcept { return ptr(); }
    constexpr const T* operator->() const noexcept { return ptr(); }
    T& operator*() & noexcept { return *ptr(); }
    constexpr const T& operator*() const& noexcept { return *ptr(); }
    T&& operator*() && noexcept { return *ptr(); }
    constexpr const T&& operator*() const&& noexcept { return *ptr(); }
    constexpr explicit operator bool() const noexcept { return has_value(); }
    constexpr bool has_value() const noexcept { return valid_; }
    T& value() &
    {
        if (!has_value())
            throw qinmo::bad_optional_access();

        return *ptr();
    }
    const T& value() const &
    {
        if (!has_value())
            throw qinmo::bad_optional_access();

        return *ptr();
    }
    T&& value() &&
    {
        if (!has_value())
            throw qinmo::bad_optional_access();

        return std::move(*ptr());
    }
    const T&& value() const &&
    {
        if (!has_value())
            throw qinmo::bad_optional_access();

        return std::move(*ptr());
    }


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

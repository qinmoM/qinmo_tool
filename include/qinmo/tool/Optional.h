/**
 * @brief span class
 */

#pragma once

#include <initializer_list>
#include <utility>
#include <stdexcept>



namespace qinmo
{

/// @brief tag type used to represent an empty value
struct nullopt_t
{
    enum class Construct { Token };

    explicit constexpr nullopt_t(Construct) noexcept { }
};

extern const nullopt_t nullopt;


/// @brief tag type for in-place construction
struct in_place_t
{
    explicit constexpr in_place_t() = default;
};

extern const in_place_t in_place;


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


/// @brief for  enable_if
template<typename T>
class Optional;

template<typename T>
struct is_optional : std::false_type { };
template<typename T>
struct is_optional<Optional<T>> : std::true_type { };

template<typename U>
struct is_valid_optional_value
    : std::integral_constant<
        bool,
        !std::is_void<U>::value &&
        !std::is_reference<U>::value &&
        !std::is_function<U>::value
    >
{ };



template<typename T>
class Optional
{
public:
/*
                limit type
*/
    static_assert(!std::is_void<T>::value, "Optional cannot be use void");
    static_assert(!std::is_reference<T>::value, "Optional cannot be use reference type");
    static_assert(!std::is_function<T>::value, "Optional cannot be use function");
    static_assert(!std::is_same<typename std::remove_cv<T>::type, nullopt_t>::value, "Optional cannot be use nullopt_t");
    static_assert(!std::is_same<typename std::remove_cv<T>::type, in_place_t>::value, "Optional cannot be use in_place_t");

/*
                types
*/
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

    template<class... Args>
        explicit Optional(in_place_t, Args&&... args)
        {
            new (value_) T(std::forward<Args>(args)...);
            valid_ = true;
        }

    template<class U, class... Args>
        explicit Optional(in_place_t, std::initializer_list<U> list, Args&&... args)
        {
            new (value_) T(list, std::forward<Args>(args)...);
            valid_ = true;
        }

    template<class U = typename std::remove_cv<T>::type, typename std::enable_if<!std::is_convertible<U&&, T>::value, int>::type = 0>
        explicit Optional(U&& value) { emplace(std::forward<U>(value)); }
    template<class U = typename std::remove_cv<T>::type, typename std::enable_if<std::is_convertible<U&&, T>::value, int>::type = 0>
        Optional(U&& value) { emplace(std::forward<U>(value)); }

    template<class U, typename std::enable_if<!std::is_convertible<const U&, T>::value, int>::type = 0>
        explicit Optional(const Optional<U>& other) { if (other.has_value()) emplace(*other); }
    template<class U, typename std::enable_if<std::is_convertible<const U&, T>::value, int>::type = 0>
        Optional(const Optional<U>& other) { if (other.has_value()) emplace(*other); }

    template<class U, typename std::enable_if<!std::is_convertible<U&&, T>::value, int>::type = 0>
        explicit Optional(Optional<U>&& other){ if (other.has_value()) emplace(std::move(*other));}
    template<class U, typename std::enable_if<std::is_convertible<U&&, T>::value, int>::type = 0>
        Optional(Optional<U>&& other){ if (other.has_value()) emplace(std::move(*other));}

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
        if (has_value() && other.has_value())
            **this = *other;
        else if (has_value())
            reset();
        else if (other.has_value())
            emplace(*other);

        return *this;
    }

    Optional& operator=(Optional&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
        if (has_value() && other.has_value())
            **this = std::move(*other);
        else if (has_value())
            reset();
        else if (other.has_value())
            emplace(std::move(*other));

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
    template<class U, typename std::enable_if<std::is_constructible<T, U&&>::value, int>::type = 0>
        T value_or(U&& value) const & noexcept(
                std::is_nothrow_copy_constructible<T>::value &&
                std::is_nothrow_constructible<T, U&&>::value
            )
        {
            if (has_value())
                return **this;

            return std::forward<U>(value);
        }
    template<class U, typename std::enable_if<std::is_constructible<T, U&&>::value, int>::type = 0>
        T value_or(U&& value) && noexcept(
                std::is_nothrow_move_constructible<T>::value &&
                std::is_nothrow_constructible<T, U&&>::value
            )
        {
            if (has_value())
                return std::move(**this);

            return std::forward<U>(value);
        }


/*
                monadic operations
*/
    template <
        class F,
        typename std::enable_if<
            is_optional<
                decltype( std::declval<F>()(std::declval<T&>()) )
            >::value, int
        >::type = 0
    >
    auto and_then(F&& f) &
        -> decltype( std::declval<F>()(std::declval<T&>()) )
    {
        using return_type = decltype( std::declval<F>()(std::declval<T&>()) );

        if (has_value())
            return std::forward<F>(f)(**this);

        return return_type(nullopt);
    }

    template <
        class F,
        typename std::enable_if<
            is_optional<
                decltype( std::declval<F>()(std::declval<T&&>()) )
            >::value, int
        >::type = 0
    >
        auto and_then(F&& f) &&
        -> decltype( std::declval<F>()(std::declval<T&&>()) )
    {
        using return_type = decltype( std::declval<F>()(std::declval<T&&>()) );

        if (has_value())
            return std::forward<F>(f)(std::move(**this));

        return return_type(nullopt);
    }

    template <
        class F,
        typename std::enable_if<
            is_optional<
                decltype( std::declval<F>()(std::declval<const T&>()) )
            >::value, int
        >::type = 0
    >
    auto and_then(F&& f) const &
        -> decltype( std::declval<F>()(std::declval<const T&>()) )
    {
        using return_type = decltype( std::declval<F>()(std::declval<const T&>()) );

        if (has_value())
            return std::forward<F>(f)(**this);

        return return_type(nullopt);
    }

    template <
        class F,
        typename std::enable_if<
            is_optional<
                decltype( std::declval<F>()(std::declval<const T&&>()) )
            >::value, int
        >::type = 0
    >
    auto and_then(F&& f) const &&
        -> decltype( std::declval<F>()(std::declval<const T&&>()) )
    {
        using return_type = decltype( std::declval<F>()(std::declval<const T&&>()) );

        if (has_value())
            return std::forward<F>(f)(std::move(**this));

        return return_type(nullopt);
    }

    template <
        class F,
        typename std::enable_if<
            is_valid_optional_value<
                decltype( std::declval<F>()(std::declval<T&>()) )
            >::value, int
        >::type = 0
    >
    auto transform(F&& f) &
        -> Optional<decltype( std::declval<F>()(std::declval<T&>()) )>
    {
        using U = decltype( std::declval<F>()(std::declval<T&>()) );

        if (has_value())
            return Optional<U>(std::forward<F>(f)(**this));

        return Optional<U>(nullopt);
    }

    template <
        class F,
        typename std::enable_if<
            is_valid_optional_value<
                decltype( std::declval<F>()(std::declval<T&&>()) )
            >::value, int
        >::type = 0
    >
    auto transform(F&& f) &&
        -> Optional<decltype( std::declval<F>()(std::declval<T&&>()) )>
    {
        using U = decltype( std::declval<F>()(std::declval<T&&>()) );

        if (has_value())
            return Optional<U>(std::forward<F>(f)(std::move(**this)));

        return Optional<U>(nullopt);
    }

    template <
        class F,
        typename std::enable_if<
            is_valid_optional_value<
                decltype( std::declval<F>()(std::declval<const T&>()) )
            >::value, int
        >::type = 0
    >
    auto transform(F&& f) const &
        -> Optional<decltype( std::declval<F>()(std::declval<const T&>()) )>
    {
        using U = decltype( std::declval<F>()(std::declval<const T&>()) );

        if (has_value())
            return Optional<U>(std::forward<F>(f)(**this));

        return Optional<U>(nullopt);
    }

    template <
        class F,
        typename std::enable_if<
            is_valid_optional_value<
                decltype( std::declval<F>()(std::declval<const T&&>()) )
            >::value, int
        >::type = 0
    >
    auto transform(F&& f) const &&
        -> Optional<decltype( std::declval<F>()(std::declval<const T&&>()) )>
    {
        using U = decltype( std::declval<F>()(std::declval<const T&&>()) );

        if (has_value())
            return Optional<U>(std::forward<F>(f)(std::move(**this)));

        return Optional<U>(nullopt);
    }

    template <
        class F,
        typename std::enable_if<
            std::is_same<
                decltype( std::declval<F>()() ),
                Optional<T>
            >::value, int
        >::type = 0
    >
    Optional<T> or_else(F&& f) &&
    {
        if (has_value())
            return std::move(*this);

        return std::forward<F>(f)();
    }

    template <
        class F,
        typename std::enable_if<
            std::is_same<
                decltype( std::declval<F>()() ),
                Optional<T>
            >::value, int
        >::type = 0
    >
    Optional<T> or_else(F&& f) const &
    {
        if (has_value())
            return *this;

        return std::forward<F>(f)();
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

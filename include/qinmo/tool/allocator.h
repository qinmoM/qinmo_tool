#pragma once

#include <memory>
#include <limits>
#include <stdexcept>
#include <cstddef>

#if defined(_WIN32)
#include <malloc.h>
#elif defined(__linux__)
#include <stdlib.h>
#else
#error "Platform not supported"
#endif

namespace qinmo
{

/// @brief check if value is power of two during compile
constexpr bool isPowerOfTwo(std::size_t n) noexcept
{
    return n != 0 && (n & (n - 1)) == 0;
}


#if defined(__cpp_aligned_new)
    template <class T>
    using allocator = std::allocator<T>;
#else
    template <class T>
    struct allocator
    {
        static_assert(isPowerOfTwo(alignof(T)), "alignment must be power of two");

        using value_type = T;

        T* allocate(std::size_t num)
        {
            if (std::numeric_limits<std::size_t>::max() / sizeof(T) < num)
                throw std::bad_array_new_length();

            if (0 == num)
                return nullptr;

            const std::size_t alignment = alignof(T) < sizeof(void*) ? sizeof(void*) : alignof(T);
#if defined(_WIN32)
            T* p = static_cast<T*>(_aligned_malloc(sizeof(T) * num, alignment));
            if (nullptr == p)
                throw std::bad_alloc();
#else
            T* p = nullptr;
            if (alignof(T) <= sizeof(void*))
            {
                if ((p = malloc(sizeof(T) * num)) == nullptr)
                    throw std::bad_alloc();
            }
            else
            {
                if (0 != posix_memalign(reinterpret_cast<void**>(&p), alignment, sizeof(T) * num))
                    throw std::bad_alloc();
            }
#endif
            return p;
        }

        void deallocate(T* p, std::size_t)
        {
#if defined(_WIN32)
            _aligned_free(reinterpret_cast<void*>(p));
#else
            free(reinterpret_cast<void*>(p));
#endif
        }
    };
#endif

} // namespace qinmo

/**
 * @brief random
 * @note Get random in thread-safe
 */

#pragma once

#include <random>
#include <cstdint>



namespace qinmo
{

namespace detail
{
/// @brief thread-safe access to the random engine
class RandomEngine
{
public:
    static std::mt19937_64& getMt()
    {
        thread_local static RandomEngine randEngine;
        return randEngine.mt_;
    }

private:
    RandomEngine()
    {
        std::random_device rd;
        mt_.seed(rd());
    }

    RandomEngine(const RandomEngine&) = delete;
    RandomEngine& operator=(const RandomEngine&) = delete;
    RandomEngine(RandomEngine&&) = delete;
    RandomEngine& operator=(RandomEngine&&) = delete;

    std::mt19937_64 mt_;
};


/// @brief template for all of fundamental-types
template<typename T, template<typename> class Distribution>
class RandomUniform
{
public:
    /// @brief 
    RandomUniform(T min, T max)
        : dis_(min, max)
    { }

public:
    /// @brief Temporarity acquire a random-number via static-access
    static T rand(T min, T max)
    {
        Distribution<T> dis(min, max);
        return dis(RandomEngine::getMt());
    }

    /// @brief Acquire a random-number
    T rand()
    {
        return dis_(RandomEngine::getMt());
    }

private:
    Distribution<T> dis_;

};
} // namespace detail


using RandomInt16 = detail::RandomUniform<int16_t, std::uniform_int_distribution>;
using RandomUInt16 = detail::RandomUniform<uint16_t, std::uniform_int_distribution>;

using RandomInt32 = detail::RandomUniform<int32_t, std::uniform_int_distribution>;
using RandomUInt32 = detail::RandomUniform<uint32_t, std::uniform_int_distribution>;

using RandomInt64 = detail::RandomUniform<int64_t, std::uniform_int_distribution>;
using RandomUInt64 = detail::RandomUniform<uint64_t, std::uniform_int_distribution>;

using RandomInt = RandomInt32;
using RandomFloat = detail::RandomUniform<float, std::uniform_real_distribution>;
using RandomDouble = detail::RandomUniform<double, std::uniform_real_distribution>;

} // namespace qinmo
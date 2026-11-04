#pragma once

#include <chrono>
#include <string>


#if defined(__linux__) || defined(_WIN32)
#define _QINMO_SUPPORTED_PLATFORM_TIME
#endif


namespace qinmo
{

/// @namespace qinmo::detail
/// @note For internal use only, do NOT use it from outside the library
namespace detail
{

/// @brief convert a time_t to UTC time
/// @return 
inline bool gmtime(const ::time_t& time, std::tm& tm)
{
#if defined(__linux__)
    return nullptr != ::gmtime_r(&time, &tm);
#elif defined(_WIN32)
    return 0 == ::gmtime_s(&tm, &time);
#else
    return false;
#endif
}



/// @brief current UTC time string including microseconds
/// @return empty string on failure
inline std::string nowMicrosecondsString()
{
#if !defined(_QINMO_SUPPORTED_PLATFORM_TIME)
    return "";
#endif
    // UTC time
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    time_t t = microseconds / 1000000;
    std::tm tm;
    if (!detail::gmtime(t, tm))
        return "";

    char buf[32] = "";
    int len = std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);

    if (0 == len) return "";
    len += std::snprintf(buf + len, sizeof(buf) - len, ".%06lld", microseconds % 1000000);

    return std::string(buf, len);
}
} // namespace detail
} // namespace qinmo

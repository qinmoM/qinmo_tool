#pragma once

#include <stdint.h>
#include <thread>

#if defined(__linux__) || defined(_WIN32)
#define QINMO_DETAIL_SUPPORTED_PLATFORM_TID
#endif


namespace qinmo
{

/// @namespace qinmo::detail
/// @note For internal use only, do NOT use it from outside the library
namespace detail
{

/// @brief true if fetching the system thread ID is supported
extern bool g_isOsTid;

#ifdef QINMO_DETAIL_SUPPORTED_PLATFORM_TID
/// @brief try to fetch system tid
/// @return unsigned 64-bit integer
uint64_t getNativeTid();
#else
/// @brief try to fetch system tid
/// @return cpp-api thread::id classes
std::thread::id getNativeTid();
#endif

} // namespace detail
} // namespace qinmo

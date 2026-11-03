#pragma once

#include <stdint.h>
#include <thread>

#if defined(__linux__) || defined(_WIN32)
#define QINMO_USE_SYSTEM_TID
#endif


namespace qinmo
{

/// @namespace qinmo::detail
/// @note For internal use only, do NOT use it from outside the library
namespace detail
{

extern bool g_isOsTid;

#ifdef QINMO_USE_SYSTEM_TID
uint64_t getNativeTid();
#else
std::thread::id getNativeTid();
#endif

} // namespace detail
} // namespace qinmo
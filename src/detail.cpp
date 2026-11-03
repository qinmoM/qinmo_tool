#include "qinmo/tool/detail/SystemTid.h"

#if defined(__linux__)
#include <unistd.h>
#include <sys/syscall.h>
#elif defined(_WIN32)
#include <windows.h>
#endif


namespace qinmo
{

namespace detail
{

#if defined(__linux__)
    bool g_isOsTid = true;
    thread_local uint64_t g_cache_thread_id = 0;

    uint64_t getNativeTid()
    {
        if (0 == g_cache_thread_id)
            g_cache_thread_id = ::syscall(SYS_gettid);

        return g_cache_thread_id;
    }
#elif defined(_WIN32)
    bool g_isOsTid = true;
    thread_local uint64_t g_cache_thread_id = 0;

    uint64_t getNativeTid()
    {
        if (0 == g_cache_thread_id)
            g_cache_thread_id = ::GetCurrentThreadId();

        return g_cache_thread_id;
    }
#else
    bool g_isOsTid = false;

    std::thread::id getNativeTid()
    {
        return std::this_thread::get_id();
    }
#endif
} // namespace detail
} // namespace qinmo

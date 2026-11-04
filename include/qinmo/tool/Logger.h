/**
 * @note logs maybe not write into file if terminater
 * @note use `QINMO_LOG_LEVEL` to specify output level in cmake
 */
#pragma once

#include "StringConcat.h"
#include "detail/SystemTime.h"
#include "detail/SystemTid.h"
#include <mutex>
#include <fstream>
#include <queue>

namespace qinmo
{

/// @note log level
/// if macro is defined, logs of the level will be written into the log file
/// if not needed, simply comment out the macro
#ifndef QINMO_LOG_LEVEL
#define QINMO_LOG_LEVEL 3
#endif

#if defined(_QINMO_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 0
#define _QINMO_ENABLE_TRACE
#endif
#if defined(_QINMO_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 1
#define _QINMO_ENABLE_DEBUG
#endif
#if defined(_QINMO_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 2
#define _QINMO_ENABLE_INFO
#endif
#if defined(_QINMO_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 3
#define _QINMO_ENABLE_WARN
#endif
#if defined(_QINMO_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 4
#define _QINMO_ENABLE_ERROR
#endif
#if defined(_QINMO_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 5
#define _QINMO_ENABLE_FATAL
#endif


enum class LogOutputModel
{
    None,               // ignore
    Console,            // print to console
    SingleFile          // output to the single file
    // MultiFileDate       // output to difference files by date
};


/// @namespace qinmo::log::detail
/// @warning For internal use only, do NOT use it from outside the library
namespace detail
{
///
/// @brief write logger class
///
class Logger
{
public:
    /// @brief get instance of Logger
    static Logger& instance()
    {
        static Logger logger;
        return logger;
    }

    /// @brief write into the log
    /// @param ...args any number and type of parameters
    template<typename... Args>
    void log(Args&&... args)
    {
        if (LogOutputModel::None == model_)
            return;

        std::string time = detail::nowMicrosecondsString();
        std::string tid = "[TID:" + std::string(detail::g_isOsTid ? "OS:" : "CPP-API:");
        std::string str = qinmo::concat("[UTC:", time, "] ", tid, detail::getNativeTid(), "] ", args..., '\n');

        if (LogOutputModel::Console == model_)
        {
            qinmo::print(str);
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(str));
        if (queue_.size() >= 1024)
            cv_.notify_one();
    }

    /// @brief flush buffer
    void flush()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            needtoFlush_ = true;
        }
        cv_.notify_one();
    }

private:
    void threadWorkFunc()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::seconds(1), [this]() -> bool { return 1024 <= queue_.size() || !running_ || needtoFlush_; });

            if (!running_ && queue_.empty())
                break;

            needtoFlush_ = false;
            std::queue<std::string> temp;
            std::swap(temp, queue_);
            lock.unlock();

            while (!temp.empty())
            {
                ofs_ << temp.front();
                temp.pop();
            }

            ofs_.flush();
        }
    }

    Logger()
        // : ofs_("logs.log", std::ios::app)
        : model_(LogOutputModel::Console)
        , needtoFlush_(false)
        , running_(true)
    {
        ofs_ << '\n';
        thread_ = std::thread([this]() -> void { threadWorkFunc(); });
    }

    ~Logger()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }

        cv_.notify_one();
        if (thread_.joinable())
            thread_.join();

        ofs_.flush();
        ofs_.close();
    }

private:
    std::ofstream ofs_;
    std::thread thread_;
    std::queue<std::string> queue_;

    std::mutex mutex_;
    std::condition_variable cv_;

    LogOutputModel model_;
    bool needtoFlush_;
    bool running_;

};


template<typename... Args>
void trace_implicit(const char* file, int line, Args&&... args)
{
    detail::Logger::instance().log("[TRACE] ", args..., " - ", file, ':', line);
}

template<typename... Args>
void debug_implicit(const char* file, int line, Args&&... args)
{
    detail::Logger::instance().log("[DEBUG] ", args..., " - ", file, ':', line);
}

template<typename... Args>
void info_implicit(const char* file, int line, Args&&... args)
{
    detail::Logger::instance().log("[INFO] ", args..., " - ", file, ':', line);
}

template<typename... Args>
void warn_implicit(const char* file, int line, Args&&... args)
{
    detail::Logger::instance().log("[WARN] ", args..., " - ", file, ':', line);
}

template<typename... Args>
void error_implicit(const char* file, int line, Args&&... args)
{
    Logger& logger = detail::Logger::instance();
    logger.log("[ERROR] ", args..., " - ", file, ':', line);
    logger.flush();
}

template<typename... Args>
void fatal_implicit(const char* file, int line, Args&&... args)
{
    Logger& logger = detail::Logger::instance();
    logger.log("[FATAL] ", args..., " - ", file, ':', line);
    logger.flush();
}

} // namespace detail



/// @brief Used for tracking details and recording state changes
#ifdef _QINMO_ENABLE_TRACE
    #define QINMO_TRACE(...) do { qinmo::detail::trace_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_TRACE(...) ((void)0)
#endif

/// @brief Debug only, should be turned off in production
#ifdef _QINMO_ENABLE_DEBUG
    #define QINMO_DEBUG(...) do { qinmo::detail::debug_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_DEBUG(...) ((void)0)
#endif

/// @brief normal runtime information
#ifdef _QINMO_ENABLE_INFO
    #define QINMO_INFO(...) do { qinmo::detail::info_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_INFO(...) ((void)0)
#endif

/// @brief Not an error, but requires attention
#ifdef _QINMO_ENABLE_WARN
    #define QINMO_WARN(...) do { qinmo::detail::warn_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_WARN(...) ((void)0)
#endif

/// @brief Error, affects a single operation, but the program remain running.
/// @note flush the buffer immediately
#ifdef _QINMO_ENABLE_ERROR
    #define QINMO_ERROR(...) do { qinmo::detail::error_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_ERROR(...) ((void)0)
#endif

/// @brief Fatal error, the program cannot run normally.
/// @note flush the buffer immediately
#ifdef _QINMO_ENABLE_FATAL
    #define QINMO_FATAL(...) do { qinmo::detail::fatal_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_FATAL(...) ((void)0)
#endif



inline bool logSetModel(const LogOutputModel& model, const std::string& path = "")
{
    // return detail::Logger::instance().setModel(model, path);
    return false;
}

inline void logFlush()
{
    return detail::Logger::instance().flush();
}

} // namespace qinmo
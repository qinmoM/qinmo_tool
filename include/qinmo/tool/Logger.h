/**
 * @note Logs maybe not write into file if terminater
 * @note Buffered logs may be lost if the process terminates abnormally.
 * @note Use options to specify output level in cmake:
 *   - `QINMO_LOG_LEVEL`        -- 0 ~ 5 : trace ~ fatal
 *   - `QINMO_LOG_OUTPUT_MODEL` -- 0: ignore; 1: print to console; 2: output to the single file; 3: output to the diference files for date.
 *   - `QINMO_LOG_PATH`         -- semantics different depending on the model: file, path or nothing
 */

#pragma once

#include "StringConcat.h"
#include "detail/SystemTime.h"
#include "detail/SystemTid.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <queue>

namespace qinmo
{

/*
        log level
*/
#ifndef QINMO_LOG_LEVEL
#define QINMO_LOG_LEVEL 3
#endif

#if defined(QINMO_DETAIL_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 0
#define QINMO_DETAIL_ENABLE_TRACE
#endif
#if defined(QINMO_DETAIL_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 1
#define QINMO_DETAIL_ENABLE_DEBUG
#endif
#if defined(QINMO_DETAIL_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 2
#define QINMO_DETAIL_ENABLE_INFO
#endif
#if defined(QINMO_DETAIL_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 3
#define QINMO_DETAIL_ENABLE_WARN
#endif
#if defined(QINMO_DETAIL_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 4
#define QINMO_DETAIL_ENABLE_ERROR
#endif
#if defined(QINMO_DETAIL_SUPPORTED_PLATFORM_TIME) && QINMO_LOG_LEVEL <= 5
#define QINMO_DETAIL_ENABLE_FATAL
#endif

#if QINMO_LOG_LEVEL < 0 || QINMO_LOG_LEVEL > 5
#error "QINMO_LOG_LEVEL must be between 0 and 5"
#endif


/*
        log output model
*/
#ifndef QINMO_LOG_OUTPUT_MODEL
#define QINMO_LOG_OUTPUT_MODEL 1
#endif

#if 3 == QINMO_LOG_OUTPUT_MODEL && !defined(QINMO_DETAIL_SUPPORTED_PLATFORM_TIME)
#error "This platform does not support the `MultiFilesByDate` model"
#endif


/*
        log output path
*/
#ifndef QINMO_LOG_PATH
#if 3 == QINMO_LOG_OUTPUT_MODEL
#define QINMO_LOG_PATH "qinmoLogs/"
#else
#define QINMO_LOG_PATH "qinmo.log"
#endif
#endif


/// @namespace qinmo::log::detail
/// @warning For internal use only, do NOT use it from outside the library
namespace detail
{

enum class LogOutputModel : int
{
    None = 0,               // ignore
    Console = 1,            // print to console
    SingleFile = 2,         // output to the single file
    MultiFilesByDate = 3    // output to the difference files for date
};

static_assert(
    QINMO_LOG_OUTPUT_MODEL >= 0 && QINMO_LOG_OUTPUT_MODEL <= 3,
    "Invalid QINMO_LOG_OUTPUT_MODEL"
);


#if 3 == QINMO_LOG_OUTPUT_MODEL
struct LogEntry
{
    std::string date;
    std::string content;
};
#else
using LogEntry = std::string;
#endif


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
            return qinmo::print(str);

        if (LogOutputModel::SingleFile == model_)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(std::move(str));
            if (queue_.size() >= 1024)
            {
                lock.unlock();
                cv_.notify_one();
            }
        }
        else if (LogOutputModel::MultiFilesByDate == model_)
        {
            ;
        }
    }

    /// @brief flush buffer
    void flush()
    {
        if (LogOutputModel::SingleFile != model_ && LogOutputModel::MultiFilesByDate != model_)
            return;

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
            std::queue<LogEntry> temp;
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

    void closeThread()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }

        cv_.notify_one();

        if (thread_.joinable())
            thread_.join();

        if (LogOutputModel::SingleFile == model_ || LogOutputModel::MultiFilesByDate == model_)
        {
            ofs_.flush();
            ofs_.close();
        }
    }

    Logger()
        : model_(static_cast<LogOutputModel>(QINMO_LOG_OUTPUT_MODEL))
        , needtoFlush_(false)
        , running_(true)
    {
        if (LogOutputModel::Console == model_)
        {
            qinmo::println();
        }
        else if (LogOutputModel::SingleFile == model_)
        {
            ofs_.open(QINMO_LOG_PATH, std::ios::app);
            if (!ofs_.is_open())
                throw std::logic_error("Logger::Logger: log file does not exist.");

            ofs_ << '\n';
            thread_ = std::thread([this]() -> void { threadWorkFunc(); });
        }
        else if (LogOutputModel::MultiFilesByDate == model_)
        {
            ;
        }
    }

    ~Logger()
    {
        closeThread();
    }

private:
    std::ofstream ofs_;
    std::thread thread_;
    std::queue<LogEntry> queue_;

    std::mutex mutex_;
    std::condition_variable cv_;

    const LogOutputModel model_;
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
#ifdef QINMO_DETAIL_ENABLE_TRACE
    #define QINMO_TRACE(...) do { qinmo::detail::trace_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_TRACE(...) ((void)0)
#endif

/// @brief Debug only, should be turned off in production
#ifdef QINMO_DETAIL_ENABLE_DEBUG
    #define QINMO_DEBUG(...) do { qinmo::detail::debug_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_DEBUG(...) ((void)0)
#endif

/// @brief normal runtime information
#ifdef QINMO_DETAIL_ENABLE_INFO
    #define QINMO_INFO(...) do { qinmo::detail::info_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_INFO(...) ((void)0)
#endif

/// @brief Not an error, but requires attention
#ifdef QINMO_DETAIL_ENABLE_WARN
    #define QINMO_WARN(...) do { qinmo::detail::warn_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_WARN(...) ((void)0)
#endif

/// @brief Error, affects a single operation, but the program remain running.
/// @note flush the buffer immediately
#ifdef QINMO_DETAIL_ENABLE_ERROR
    #define QINMO_ERROR(...) do { qinmo::detail::error_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_ERROR(...) ((void)0)
#endif

/// @brief Fatal error, the program cannot run normally.
/// @note flush the buffer immediately
#ifdef QINMO_DETAIL_ENABLE_FATAL
    #define QINMO_FATAL(...) do { qinmo::detail::fatal_implicit(__FILE__, __LINE__, __VA_ARGS__); } while(false)
#else
    #define QINMO_FATAL(...) ((void)0)
#endif



/// @brief try to flush buffer in file model
inline void logFlush()
{
    return detail::Logger::instance().flush();
}

} // namespace qinmo
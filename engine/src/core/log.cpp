#include <mutex>
#include <core/log.hpp>
#include <iostream>
#include <chrono>
#if _WIN32
#include <Windows.h>
#endif

namespace mental::core::log
{
static std::mutex gLogMutex;

constexpr const char* levelToString(Level level)
{
    switch (level)
    {
        case Level::eFatal: return "FATAL";
        case Level::eError: return "ERROR";
        case Level::eWarn: return "WARN";
        case Level::eInfo: return "INFO";
        case Level::eDebug: return "DEBUG";
        case Level::eTrace: return "TRACE";
    }
}
Logger& Logger::getInstance()
{
    static Logger logger;
    return logger;
}

Logger::~Logger()
{
    flush();
}

void Logger::log(Level level, const std::string& message, const std::source_location& location)
{
    auto timestamp = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto tm = std::localtime(&time_t);

    std::lock_guard<std::mutex> guard(gLogMutex);

    std::string formatStr = std::format("[{:02}:{:02}:{:02}] [{}] {} ({}:{})\n", tm->tm_hour, tm->tm_min, tm->tm_sec, levelToString(level),
        message, location.file_name(), location.line());
    std::cout << formatStr;

#if _WIN32
    if (mOutputToDebug)
    {
        OutputDebugStringA(formatStr.c_str());
    }
#endif
}

void Logger::enableOutputToDebug(bool enable)
{
    mOutputToDebug = enable;
}

void Logger::flush()
{
    std::cout.flush();
}
}  // namespace mental::core::log

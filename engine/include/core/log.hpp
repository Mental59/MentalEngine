#pragma once
#include <format>
#include <source_location>
#include <string>

namespace mental::core::log
{
enum class Level : uint8_t
{
  eFatal = 0,
  eError,
  eWarn,
  eInfo,
  eDebug,
  eTrace
};

constexpr const char* levelToString(Level level);

class Logger
{
 public:
  static Logger& getInstance();

  ~Logger();

  void log(Level level, const std::string& message, const std::source_location& location);
  void enableOutputToDebug(bool enable);

  void flush();

  template <typename... Args>
  void fatal(const std::source_location& location, const std::string& format, Args&&... args)
  {
    log(Level::eFatal, std::vformat(format, std::make_format_args(args...)), location);
  }

  template <typename... Args>
  void error(const std::source_location& location, const std::string& format, Args&&... args)
  {
    log(Level::eError, std::vformat(format, std::make_format_args(args...)), location);
  }

  template <typename... Args> void warn(const std::source_location& location, const std::string& format, Args&&... args)
  {
    log(Level::eWarn, std::vformat(format, std::make_format_args(args...)), location);
  }

  template <typename... Args> void info(const std::source_location& location, const std::string& format, Args&&... args)
  {
    log(Level::eInfo, std::vformat(format, std::make_format_args(args...)), location);
  }

  template <typename... Args>
  void debug(const std::source_location& location, const std::string& format, Args&&... args)
  {
    log(Level::eDebug, std::vformat(format, std::make_format_args(args...)), location);
  }

  template <typename... Args>
  void trace(const std::source_location& location, const std::string& format, Args&&... args)
  {
    log(Level::eTrace, std::vformat(format, std::make_format_args(args...)), location);
  }

  void assertion_failed(const std::source_location& location, const std::string& expr, const std::string& message)
  {
    log(Level::eFatal, std::format("Assertion failure: {}, message: '{}'", expr, message), location);
  }

 private:
  Logger() = default;
  bool mOutputToDebug = false;
};
} // namespace mental::core::log

#define MENTAL_FATAL(message, ...)                                                                                     \
  mental::core::log::Logger::getInstance().fatal(std::source_location::current(), message, __VA_ARGS__)

#define MENTAL_ERROR(message, ...)                                                                                     \
  mental::core::log::Logger::getInstance().error(std::source_location::current(), message, __VA_ARGS__)

#ifdef MENTAL_LOG_WARNINGS
#define MENTAL_WARN(message, ...)                                                                                      \
  mental::core::log::Logger::getInstance().warn(std::source_location::current(), message, __VA_ARGS__)
#else
#define MENTAL_WARN(message, ...)
#endif

#ifdef MENTAL_LOG_INFO
#define MENTAL_INFO(message, ...)                                                                                      \
  mental::core::log::Logger::getInstance().info(std::source_location::current(), message, __VA_ARGS__)
#else
#define MENTAL_INFO(message, ...)
#endif

#ifdef _DEBUG
#define MENTAL_DEBUG(message, ...)                                                                                     \
  mental::core::log::Logger::getInstance().debug(std::source_location::current(), message, __VA_ARGS__)
#else
#define MENTAL_DEBUG(message, ...)
#endif

#ifdef MENTAL_LOG_TRACES
#define MENTAL_TRACE(message, ...)                                                                                     \
  mental::core::log::Logger::getInstance().trace(std::source_location::current(), message, __VA_ARGS__)
#else
#define MENTAL_TRACE(message, ...)
#endif

#ifdef MENTAL_ASSERTS

#if defined(_MSC_VER)
#include <intrin.h>
#define MENTAL_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__)
#define MENTAL_DEBUG_BREAK() __builtin_trap()
#else
#include <signal.h>
#ifdef SIGTRAP
#define MENTAL_DEBUG_BREAK() raise(SIGTRAP)
#else
#define MENTAL_DEBUG_BREAK() raise(SIGABRT)
#endif
#endif

#define MENTAL_ASSERT(expr)                                                                                            \
  {                                                                                                                    \
    if (expr)                                                                                                          \
    {                                                                                                                  \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
      mental::core::log::Logger::getInstance().assertion_failed(std::source_location::current(), #expr, "");           \
      MENTAL_DEBUG_BREAK();                                                                                            \
    }                                                                                                                  \
  }

#define MENTAL_ASSERT_MESSAGE(expr, message)                                                                           \
  {                                                                                                                    \
    if (expr)                                                                                                          \
    {                                                                                                                  \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
      mental::core::log::Logger::getInstance().assertion_failed(std::source_location::current(), #expr, message);      \
      MENTAL_DEBUG_BREAK();                                                                                            \
    }                                                                                                                  \
  }

#ifdef _DEBUG
#define MENTAL_ASSERT_DEBUG(expr)                                                                                      \
  {                                                                                                                    \
    if (expr)                                                                                                          \
    {                                                                                                                  \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
      mental::core::log::Logger::getInstance().assertion_failed(std::source_location::current(), #expr, "");           \
      MENTAL_DEBUG_BREAK();                                                                                            \
    }                                                                                                                  \
  }
#else
#define MENTAL_ASSERT_DEBUG(expr)
#endif

#else

#define MENTAL_ASSERT(expr)
#define MENTAL_ASSERT_MESSAGE(expr, message, ...)
#define MENTAL_ASSERT_DEBUG(expr)

#endif

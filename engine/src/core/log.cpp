#include <cstdarg>
#include <cstdio>
#include <iterator>
#include <mutex>
#include <core/log.hpp>
#if _WIN32
#include <Windows.h>
#endif

namespace mental::core::log
{
static constexpr size_t gMessageBufferSize = 4096;

static std::string gErrorMessageCaption = "Error";

static bool gOutputToMessageBox = true;
static bool gOutputToDebug = false;
static bool gOutputToConsole = true;

static std::mutex gLogMutex;

void defaultCallback(Severity severity, const char* message)
{
    const char* severityText = "";
    switch (severity)
    {
        case Severity::Debug:
        {
            severityText = "DEBUG";
            break;
        }

        case Severity::Info:
        {
            severityText = "INFO";
            break;
        }

        case Severity::Warning:
        {
            severityText = "WARNING";
            break;
        }

        case Severity::Error:
        {
            severityText = "ERROR";
            break;
        }

        case Severity::Fatal:
        {
            severityText = "FATAL ERROR";
            break;
        }
    }

    char buf[gMessageBufferSize];
    snprintf(buf, std::size(buf), "[%s] %s", severityText, message);

    {
        std::lock_guard<std::mutex> lockGuard(gLogMutex);

#if _WIN32
        if (gOutputToDebug)
        {
            OutputDebugStringA(buf);
            OutputDebugStringA("\n");
        }

        if (gOutputToMessageBox)
        {
            if (severity == Severity::Error || severity == Severity::Fatal)
            {
                MessageBoxA(0, buf, gErrorMessageCaption.c_str(), MB_ICONERROR);
            }
        }

#endif
        if (gOutputToConsole)
        {
            if (severity == Severity::Error || severity == Severity::Fatal)
                fprintf(stderr, "%s\n", buf);
            else
                fprintf(stdout, "%s\n", buf);
        }
    }

    if (severity == Severity::Fatal) abort();
}

void setErrorMessageCaption(const char* caption)
{
    gErrorMessageCaption = (caption) ? caption : "";
}

static Callback gCallback = &defaultCallback;
static Severity gMinSeverity = Severity::Info;

void setMinSeverity(Severity severity)
{
    gMinSeverity = severity;
}

void setCallback(Callback func)
{
    gCallback = func;
}

Callback getCallback()
{
    return gCallback;
}

void resetCallback()
{
    gCallback = &defaultCallback;
}

void enableOutputToMessageBox(bool enable)
{
    gOutputToMessageBox = enable;
}

void enableOutputToConsole(bool enable)
{
    gOutputToConsole = enable;
}

void enableOutputToDebug(bool enable)
{
    gOutputToDebug = enable;
}

void consoleApplicationMode()
{
    gOutputToConsole = true;
    gOutputToDebug = true;
    gOutputToMessageBox = false;
}

void message(Severity severity, const char* fmt...)
{
    if (static_cast<int>(gMinSeverity) > static_cast<int>(severity)) return;

    char buffer[gMessageBufferSize];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, std::size(buffer), fmt, args);

    gCallback(severity, buffer);

    va_end(args);
}

void debug(const char* fmt...)
{
    if (static_cast<int>(gMinSeverity) > static_cast<int>(Severity::Debug)) return;

    char buffer[gMessageBufferSize];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, std::size(buffer), fmt, args);

    gCallback(Severity::Debug, buffer);

    va_end(args);
}

void info(const char* fmt...)
{
    if (static_cast<int>(gMinSeverity) > static_cast<int>(Severity::Info)) return;

    char buffer[gMessageBufferSize];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, std::size(buffer), fmt, args);

    gCallback(Severity::Info, buffer);

    va_end(args);
}

void warning(const char* fmt...)
{
    if (static_cast<int>(gMinSeverity) > static_cast<int>(Severity::Warning)) return;

    char buffer[gMessageBufferSize];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, std::size(buffer), fmt, args);

    gCallback(Severity::Warning, buffer);

    va_end(args);
}

void error(const char* fmt...)
{
    if (static_cast<int>(gMinSeverity) > static_cast<int>(Severity::Error)) return;

    char buffer[gMessageBufferSize];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, std::size(buffer), fmt, args);

    gCallback(Severity::Error, buffer);

    va_end(args);
}

void fatal(const char* fmt...)
{
    char buffer[gMessageBufferSize];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, std::size(buffer), fmt, args);

    gCallback(Severity::Fatal, buffer);

    va_end(args);
}
}  // namespace mental::core::log

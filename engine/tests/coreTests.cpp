#include <core/log.hpp>
#include <core/types.hpp>

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace
{
void require(bool condition, const char* message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

void exerciseLogMacrosWithoutFormatArguments()
{
  MENTAL_ERROR("Core error");
  MENTAL_WARN("Core warning");
  MENTAL_INFO("Core info");
  MENTAL_TRACE("Core trace");
}

void testResultToStringFormatsUnknownValues()
{
  const char* text = mental::core::resultToString(static_cast<mental::core::Result>(999));
  require(std::strcmp(text, "Unknown (999)") == 0, "Unknown result values should format as integers");
}
} // namespace

int main()
{
  try
  {
    exerciseLogMacrosWithoutFormatArguments();
    testResultToStringFormatsUnknownValues();
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Unknown exception\n";
  }

  return 1;
}

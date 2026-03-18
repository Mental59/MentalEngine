#pragma once

namespace mental::core
{
enum class Result
{
  eSuccess = 0,
  eInitializationFailed,
  eOperationFailed,
  eSuboptimal,
  eOutOfDate,
  eNotReady,
  eTimeout,
};

const char* resultToString(Result res);
} // namespace mental::core

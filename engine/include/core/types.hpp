#pragma once

namespace mental::core
{
  enum class Result
  {
    eSuccess = 0,
    eInitializationFailed,
    eBufferMapFailed,
    eBufferCopyFailed,
    eQueueSubmitFailed,
    eCommandListOperationFailed
  };

  const char* resultToString(Result res);
}  // namespace mental::core

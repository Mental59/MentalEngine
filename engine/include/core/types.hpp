#pragma once

namespace mental::core
{
  enum class Result
  {
    eSuccess = 0,
    eResourceCreationFailed,
    eInitializationFailed,
    eBufferMapFailed,
    eBufferCopyFailed,
    eQueueSubmitFailed,
    eCommandListOperationFailed,
    eFenceOperationFailed,
  };

  const char* resultToString(Result res);
}  // namespace mental::core

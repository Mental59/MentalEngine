#include <core/types.hpp>
#include <cstdio>

const char* mental::core::resultToString(mental::core::Result res)
{
  switch (res)
  {
    case mental::core::Result::eSuccess: return "RHI_SUCCESS";
    case mental::core::Result::eInitializationFailed: return "RHI_INITIALIZATION_FAILED";
    case mental::core::Result::eBufferMapFailed: return "RHI_BUFFER_MAP_FAILED";
    case mental::core::Result::eBufferCopyFailed: return "RHI_BUFFER_COPY_FAILED";
    case mental::core::Result::eQueueSubmitFailed: return "RHI_QUEUE_SUBMIT_FAILED";
    case mental::core::Result::eCommandListOperationFailed: return "RHI_COMMAND_LIST_OPERATION_FAILED";

    default:
    {
      static char buf[24];
      snprintf(buf, sizeof(buf), "Unknown (%d)", res);
      return buf;
    }
  }
}

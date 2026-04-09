#include <core/types.hpp>
#include <cstdio>

const char* mental::core::resultToString(mental::core::Result res)
{
  switch (res)
  {
    case mental::core::Result::eSuccess:
      return "RHI_SUCCESS";
    case mental::core::Result::eInitializationFailed:
      return "RHI_INITIALIZATION_FAILED";
    case mental::core::Result::eOperationFailed:
      return "RHI_OPERATION_FAILED";
    case mental::core::Result::eSuboptimal:
      return "RHI_SUBOPTIMAL";
    case mental::core::Result::eOutOfDate:
      return "RHI_OUT_OF_DATE";
    case mental::core::Result::eNotReady:
      return "RHI_NOT_READY";
    case mental::core::Result::eTimeout:
      return "RHI_TIMEOUT";

    default:
    {
      static char buf[24];
      snprintf(buf, sizeof(buf), "Unknown (%d)", static_cast<int>(res));
      return buf;
    }
  }
}

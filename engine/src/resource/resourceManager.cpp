#include <resource/resourceManager.hpp>

bool mental::resource::FrameData::isValid() const
{
  const bool isValidCmdList = cmdList && cmdList->isValid();
  const bool isValidFence = fence && fence->isValid();
  const bool isValidImageAvailableSemaphore = imageAvailableSemaphore && imageAvailableSemaphore->isValid();
  const bool isValidRenderFinishedSemaphore = renderFinishedSemaphore && renderFinishedSemaphore->isValid();
  return isValidCmdList && isValidFence && isValidImageAvailableSemaphore && isValidRenderFinishedSemaphore;
}

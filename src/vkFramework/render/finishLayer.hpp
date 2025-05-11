#pragma once
#include "baseRenderLayer.hpp"
#include <vulkan/vulkan_core.h>

namespace vkFramework::render {

class FinishLayer : public BaseRenderLayer {
public:
  FinishLayer(VulkanRenderDevice& vkDev, VulkanImage depthTexture);

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) override;
};

} // namespace vkFramework::render
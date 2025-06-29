#pragma once

#include "baseRenderLayer.hpp"
#include <vulkan/vulkan_core.h>

namespace vkFramework::render {

class ClearLayer : public BaseRenderLayer {
public:
  explicit ClearLayer() = default;
  void init(const VulkanRenderDevice* vkDev, VulkanImage* depthTexture);

  virtual bool createFramebuffers() override;

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) override;

private:
  bool mShouldClearDepth;
};

} // namespace vkFramework::render
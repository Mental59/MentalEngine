#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace mental {
struct RenderPassCreateInfo final {
  bool clearColor_ = false;
  bool clearDepth_ = false;
  uint8_t flags_ = 0;
};

enum ERenderPassBit : uint8_t {
  RenderPassBit_First = 0x01,
  RenderPassBit_Last = 0x02,
  RenderPassBit_Offscreen = 0x04,
  RenderPassBit_OffscreenInternal = 0x08,
};

bool createPipelineLayout(VkDevice device, VkDescriptorSetLayout dsLayout,
                          VkPipelineLayout* pipelineLayout);

bool createColorAndDepthRenderPass(
    VulkanRenderDevice& device, bool useDepth, VkRenderPass* renderPass,
    const RenderPassCreateInfo& ci,
    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM);
} // namespace mental

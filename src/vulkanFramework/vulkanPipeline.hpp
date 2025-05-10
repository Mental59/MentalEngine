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

bool createGraphicsPipeline(
    VulkanRenderDevice& vkDev, VkRenderPass renderPass,
    VkPipelineLayout pipelineLayout,
    const std::vector<const char*>& shaderFiles, VkPipeline* pipeline,
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    bool useDepth = true, bool useBlending = true,
    bool dynamicScissorState = true, bool dynamicViewportState = true,
    int32_t customWidth = -1, int32_t customHeight = -1,
    uint32_t numPatchControlPoints = 0);
} // namespace mental

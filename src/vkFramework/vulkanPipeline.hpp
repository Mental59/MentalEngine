#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace vkFramework {
struct RenderPassCreateInfo final {
  bool clearColor = false;
  bool clearDepth = false;
  uint8_t flags = 0;
};

enum ERenderPassBit : uint8_t {
  RENDER_PASS_BIT_FIRST = 0x01,
  RENDER_PASS_BIT_LAST = 0x02,
  RENDER_PASS_BIT_OFFSCREEN = 0x04,
  RENDER_PASS_BIT_OFFSCREEN_INTERNAL = 0x08,
};

bool createPipelineLayout(VkDevice device, VkDescriptorSetLayout dsLayout,
                          VkPipelineLayout* pipelineLayout);

bool createPipelineLayoutWithConstants(VkDevice device,
                                       VkDescriptorSetLayout dsLayout,
                                       uint32_t vtxConstSize,
                                       uint32_t fragConstSize,
                                       VkPipelineLayout* pipelineLayout);

bool createColorAndDepthRenderPass(
    const VulkanRenderDevice& device, bool useDepth, VkRenderPass* renderPass,
    const RenderPassCreateInfo& ci,
    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM);

bool createGraphicsPipeline(
    const VulkanRenderDevice& vkDev, VkRenderPass renderPass,
    VkPipelineLayout pipelineLayout,
    const std::vector<const char*>& shaderFiles, VkPipeline* pipeline,
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    bool useDepth = true, bool useBlending = true,
    bool dynamicScissorState = true, bool dynamicViewportState = true,
    int32_t customWidth = -1, int32_t customHeight = -1,
    uint32_t numPatchControlPoints = 0);
} // namespace vkFramework

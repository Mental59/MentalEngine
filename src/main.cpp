#include "vulkanFramework/vulkanFramework.hpp"
#include "window/window.hpp"
#include <array>
#include <cstdio>
#include <glm/glm.hpp>
#include <volk.h>

namespace {
mental::VulkanInstance VK_INSTANCE;
mental::VulkanRenderDevice VK_RENDER_DEVICE;
constexpr VkClearColorValue CLEAR_VALUE_COLOR = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr uint32_t SCREEN_WIDTH = 1280;
constexpr uint32_t SCREEN_HEIGHT = 720;

struct UniformBuffer {
  glm::mat4 mvp;
};

struct VulkanState {
  // 1. Descriptor set (layout + pool + sets) -> uses uniform buffers, textures,
  // framebuffers
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  // 2.
  std::vector<VkFramebuffer> swapchainFramebuffers;

  // 3. Pipeline & render pass (using DescriptorSets & pipeline state options)
  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  // 4. Uniform buffer
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;

  // 5. Storage Buffer with index and vertex data
  VkBuffer storageBuffer;
  VkDeviceMemory storageBufferMemory;

  // 6. Depth buffer
  mental::VulkanImage depthTexture;

  VkSampler textureSampler;
  mental::VulkanImage texture;
} VK_STATE;

bool fillCommandBuffers(size_t i, uint32_t indexBufferCount) {
  const VkCommandBufferBeginInfo bufferBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
      .pInheritanceInfo = nullptr};

  const std::array<VkClearValue, 2> clearValues = {
      VkClearValue{.color = CLEAR_VALUE_COLOR},
      VkClearValue{.depthStencil = {1.0f, 0}}};

  const VkRect2D screenRect = {
      .offset = {0, 0},
      .extent = {.width = SCREEN_WIDTH, .height = SCREEN_HEIGHT}};

  MENTAL_VK_CHECK(vkBeginCommandBuffer(VK_RENDER_DEVICE.commandBuffers[i],
                                       &bufferBeginInfo));

  const VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
      .renderPass = VK_STATE.renderPass,
      .framebuffer = VK_STATE.swapchainFramebuffers[i],
      .renderArea = screenRect,
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data()};

  vkCmdBeginRenderPass(VK_RENDER_DEVICE.commandBuffers[i], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(VK_RENDER_DEVICE.commandBuffers[i],
                    VK_PIPELINE_BIND_POINT_GRAPHICS, VK_STATE.graphicsPipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(SCREEN_WIDTH);
  viewport.height = static_cast<float>(SCREEN_HEIGHT);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(VK_RENDER_DEVICE.commandBuffers[i], 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = {.width = SCREEN_WIDTH, .height = SCREEN_HEIGHT};
  vkCmdSetScissor(VK_RENDER_DEVICE.commandBuffers[i], 0, 1, &scissor);

  vkCmdBindDescriptorSets(VK_RENDER_DEVICE.commandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          VK_STATE.pipelineLayout, 0, 1,
                          VK_STATE.descriptorSets.data(), 0, nullptr);
  vkCmdDraw(VK_RENDER_DEVICE.commandBuffers[i], indexBufferCount, 1, 0, 0);

  vkCmdEndRenderPass(VK_RENDER_DEVICE.commandBuffers[i]);

  MENTAL_VK_CHECK(vkEndCommandBuffer(VK_RENDER_DEVICE.commandBuffers[i]));

  return true;
}

bool createUniformBuffers() {
  VkDeviceSize bufferSize = sizeof(UniformBuffer);

  VK_STATE.uniformBuffers.resize(VK_RENDER_DEVICE.swapchainImages.size());
  VK_STATE.uniformBuffersMemory.resize(VK_RENDER_DEVICE.swapchainImages.size());

  for (size_t i = 0; i < VK_RENDER_DEVICE.swapchainImages.size(); i++) {
    if (!mental::createBuffer(
            VK_RENDER_DEVICE.device, VK_RENDER_DEVICE.physicalDevice,
            bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_STATE.uniformBuffers[i], VK_STATE.uniformBuffersMemory[i])) {
      return false;
    }
  }

  return true;
}

void updateUniformBuffer(uint32_t currentImage, const void* uboData,
                         size_t uboSize) {
  void* data = nullptr;
  vkMapMemory(VK_RENDER_DEVICE.device,
              VK_STATE.uniformBuffersMemory[currentImage], 0, uboSize, 0,
              &data);
  memcpy(data, uboData, uboSize);
  vkUnmapMemory(VK_RENDER_DEVICE.device,
                VK_STATE.uniformBuffersMemory[currentImage]);
}
} // namespace

int main() {
  volkInitialize();

  mental::Window window(1280, 720, "Mental engine editor");

  mental::initVulkanInstance(VK_INSTANCE, &window);

  while (!window.shouldClose()) {
    window.pollEvents();
  }

  mental::destroyVulkanInstance(VK_INSTANCE);

  return 0;
}

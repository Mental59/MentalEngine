#pragma once

#include "vulkanFramework/vulkanFramework.hpp"
#include "window/window.hpp"
#include <vector>
#include <volk.h>

namespace mental {

class DemoVulkanApp {
public:
  struct VulkanState {
    // 1. Descriptor set (layout + pool + sets) -> uses uniform buffers,
    // textures, framebuffers
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    // 2.
    std::vector<VkFramebuffer> swapchainFramebuffers;

    // 3. Pipeline & render pass (using DescriptorSets & pipeline state options)
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    // 4. Uniform buffer
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    // 5. Storage Buffer with index and vertex data
    VkBuffer storageBuffer = VK_NULL_HANDLE;
    VkDeviceMemory storageBufferMemory = VK_NULL_HANDLE;

    // 6. Depth buffer
    mental::VulkanImage depthTexture;

    VkSampler textureSampler = VK_NULL_HANDLE;
    mental::VulkanImage texture;
  };

  DemoVulkanApp();
  ~DemoVulkanApp();

  void run();

private:
  void init();
  void initVulkan();

  void cleanup();

  bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
  int findQueueFamily(VkPhysicalDevice physicalDevice);
  bool fillCommandBuffers(size_t i, uint32_t indexBufferCount);
  bool createUniformBuffers();
  void updateUniformBuffer(uint32_t currentImage, const void* uboData,
                           size_t uboSize);
  bool createDescriptorSet(size_t vertexBufferSize, size_t indexBufferSize);

  mental::VulkanInstance mVulkanInstance;
  mental::VulkanRenderDevice mVulkanRenderDevice;
  VulkanState mVulkanState;

  mental::Window mWindow;
};

} // namespace mental

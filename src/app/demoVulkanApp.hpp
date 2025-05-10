#pragma once

#include "baseApp.hpp"
#include "vulkanFramework/vulkanFramework.hpp"
#include "window/window.hpp"
#include <vector>
#include <volk.h>

namespace mental {

class DemoVulkanApp : public BaseApp {
public:
  struct VulkanState {
    // 1. Descriptor set (layout + pool + sets) -> uses uniform buffers,
    // textures, framebuffers
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    // 2. Pipeline & render pass (using DescriptorSets & pipeline state options)
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    // 3. Uniform buffer
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    // 4. Storage Buffer with index and vertex data
    VkBuffer storageBuffer = VK_NULL_HANDLE;
    VkDeviceMemory storageBufferMemory = VK_NULL_HANDLE;
    size_t vertexBufferSize = 0;
    size_t indexBufferSize = 0;

    // 5. Depth buffer
    mental::VulkanImage depthTexture{};

    mental::VulkanImage texture{};
  };

  DemoVulkanApp();
  ~DemoVulkanApp();

  void run();

private:
  void init();
  void initVulkan();

  void render(uint32_t currentFrame);

  void cleanup();
  void cleanupSwapchain(VkSwapchainKHR swapchain);
  void destroyVulkanState();

  void recreateSwapchain(uint32_t currentFrame);

  bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
  int findQueueFamily(VkPhysicalDevice physicalDevice);
  bool fillCommandBuffers(size_t imageIndex, uint32_t frameIndex,
                          uint32_t indexBufferCount);
  bool createUniformBuffers();
  void updateUniformBuffer(uint32_t frameIndex, const void* uboData,
                           size_t uboSize);
  bool createDescriptorSet(size_t vertexBufferSize, size_t indexBufferSize);

  mental::VulkanInstance mVulkanInstance;
  mental::VulkanRenderDevice mVulkanRenderDevice;
  VulkanState mVulkanState;

  mental::Window mWindow;
};

} // namespace mental

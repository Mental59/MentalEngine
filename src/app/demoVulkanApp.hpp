#pragma once

#include "baseApp.hpp"
#include "vkFramework/includes.hpp"
#include "window/window.hpp"
#include <vector>
#include <volk.h>

namespace app {

class DemoVulkanApp : public BaseApp {
public:
  struct VulkanState {
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkBuffer storageBuffer = VK_NULL_HANDLE;
    VkDeviceMemory storageBufferMemory = VK_NULL_HANDLE;
    size_t vertexBufferSize = 0;
    size_t indexBufferSize = 0;

    vkFramework::VulkanImage depthTexture{};

    vkFramework::VulkanImage texture{};

    std::vector<VkFramebuffer> swapchainFramebuffers;
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

  vkFramework::VulkanInstance mVulkanInstance;
  vkFramework::VulkanRenderDevice mVulkanRenderDevice;
  VulkanState mVulkanState;

  window::Window mWindow;
};

} // namespace app

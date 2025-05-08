#include "demoVulkanApp.hpp"
#include <array>
#include <volk.h>

namespace {

constexpr uint32_t SCREEN_WIDTH = 1280;
constexpr uint32_t SCREEN_HEIGHT = 720;
const char* WINDOW_TITLE = "Demo Vulkan App";

constexpr VkClearColorValue CLEAR_VALUE_COLOR = {1.0f, 1.0f, 1.0f, 1.0f};

static constexpr std::array<const char*, 1> DEVICE_EXTENSIONS{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct UniformBuffer {
  glm::mat4 mvp;
};

} // namespace

mental::DemoVulkanApp::DemoVulkanApp()
    : mVulkanInstance(), mVulkanRenderDevice(), mVulkanState(),
      mWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE) {
  init();
}

mental::DemoVulkanApp::~DemoVulkanApp() {}

void mental::DemoVulkanApp::run() {
  while (!mWindow.shouldClose()) {
    mWindow.pollEvents();
  }
}

void mental::DemoVulkanApp::init() {
  volkInitialize();
  initVulkan();
}

void mental::DemoVulkanApp::initVulkan() {
  glslang_initialize_process();

  mental::initVulkanInstance(mVulkanInstance, &mWindow);
  mental::initVulkanRenderDevice(
      mVulkanInstance, SCREEN_WIDTH, SCREEN_HEIGHT,
      [this](VkPhysicalDevice physicalDevice) {
        return isDeviceSuitable(physicalDevice);
      },
      [this](VkPhysicalDevice physicalDevice) {
        return findQueueFamily(physicalDevice);
      },
      VkPhysicalDeviceFeatures{.geometryShader = VK_TRUE},
      static_cast<uint32_t>(DEVICE_EXTENSIONS.size()), DEVICE_EXTENSIONS.data(),
      mVulkanRenderDevice);

  glslang_finalize_process();
}

void mental::DemoVulkanApp::cleanup() {
  mental::destroyVulkanRenderDevice(mVulkanRenderDevice);
  mental::destroyVulkanInstance(mVulkanInstance);
}

bool mental::DemoVulkanApp::createUniformBuffers() {
  VkDeviceSize bufferSize = sizeof(UniformBuffer);

  mVulkanState.uniformBuffers.resize(
      mVulkanRenderDevice.swapchainImages.size());
  mVulkanState.uniformBuffersMemory.resize(
      mVulkanRenderDevice.swapchainImages.size());

  for (size_t i = 0; i < mVulkanRenderDevice.swapchainImages.size(); i++) {
    bool isBufferCreated = mental::createBuffer(
        mVulkanRenderDevice.device, mVulkanRenderDevice.physicalDevice,
        bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mVulkanState.uniformBuffers[i], mVulkanState.uniformBuffersMemory[i]);

    if (!isBufferCreated) {
      return false;
    }
  }

  return true;
}

void mental::DemoVulkanApp::updateUniformBuffer(uint32_t currentImage,
                                                const void* uboData,
                                                size_t uboSize) {
  void* data = nullptr;
  vkMapMemory(mVulkanRenderDevice.device,
              mVulkanState.uniformBuffersMemory[currentImage], 0, uboSize, 0,
              &data);
  memcpy(data, uboData, uboSize);
  vkUnmapMemory(mVulkanRenderDevice.device,
                mVulkanState.uniformBuffersMemory[currentImage]);
}

bool mental::DemoVulkanApp::createDescriptorSet(size_t vertexBufferSize,
                                                size_t indexBufferSize) {
  const std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
      mental::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         VK_SHADER_STAGE_VERTEX_BIT),
      mental::descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                         VK_SHADER_STAGE_VERTEX_BIT),
      mental::descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                         VK_SHADER_STAGE_VERTEX_BIT),
      mental::descriptorSetLayoutBinding(
          3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT)};

  const VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data()};

  MENTAL_VK_CHECK(
      vkCreateDescriptorSetLayout(mVulkanRenderDevice.device, &layoutInfo,
                                  nullptr, &mVulkanState.descriptorSetLayout));
  size_t numSwapchainImages = mVulkanRenderDevice.swapchainImages.size();
  std::vector<VkDescriptorSetLayout> layouts(numSwapchainImages,
                                             mVulkanState.descriptorSetLayout);

  const VkDescriptorSetAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = mVulkanState.descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(numSwapchainImages),
      .pSetLayouts = layouts.data()};

  mVulkanState.descriptorSets.resize(numSwapchainImages);

  MENTAL_VK_CHECK(vkAllocateDescriptorSets(mVulkanRenderDevice.device,
                                           &allocInfo,
                                           mVulkanState.descriptorSets.data()));

  for (size_t i = 0; i < numSwapchainImages; i++) {
    const VkDescriptorBufferInfo uniformBufferInfo = {
        .buffer = mVulkanState.uniformBuffers[i],
        .offset = 0,
        .range = sizeof(UniformBuffer)};
    const VkDescriptorBufferInfo vertexBufferInfo = {
        .buffer = mVulkanState.storageBuffer,
        .offset = 0,
        .range = vertexBufferSize};
    const VkDescriptorBufferInfo indexBufferInfo = {
        .buffer = mVulkanState.storageBuffer,
        .offset = vertexBufferSize,
        .range = indexBufferSize};
    const VkDescriptorImageInfo imageInfo = {
        .sampler = mVulkanState.textureSampler,
        .imageView = mVulkanState.texture.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    const std::array<VkWriteDescriptorSet, 4> descriptorWrites = {
        VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                             .dstSet = mVulkanState.descriptorSets[i],
                             .dstBinding = 0,
                             .dstArrayElement = 0,
                             .descriptorCount = 1,
                             .descriptorType =
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                             .pBufferInfo = &uniformBufferInfo},
        VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                             .dstSet = mVulkanState.descriptorSets[i],
                             .dstBinding = 1,
                             .dstArrayElement = 0,
                             .descriptorCount = 1,
                             .descriptorType =
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             .pBufferInfo = &vertexBufferInfo},
        VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                             .dstSet = mVulkanState.descriptorSets[i],
                             .dstBinding = 2,
                             .dstArrayElement = 0,
                             .descriptorCount = 1,
                             .descriptorType =
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             .pBufferInfo = &indexBufferInfo},
        VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                             .dstSet = mVulkanState.descriptorSets[i],
                             .dstBinding = 3,
                             .dstArrayElement = 0,
                             .descriptorCount = 1,
                             .descriptorType =
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                             .pImageInfo = &imageInfo},
    };

    vkUpdateDescriptorSets(mVulkanRenderDevice.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }

  return true;
}

bool mental::DemoVulkanApp::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
  bool extensionsSupported = checkDeviceExtensionSupport(
      physicalDevice, DEVICE_EXTENSIONS.data(),
      static_cast<uint32_t>(DEVICE_EXTENSIONS.size()));
  if (!extensionsSupported) {
    return false;
  }

  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

  const bool isDiscreteGPU =
      deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  const bool isIntegratedGPU =
      deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
  const bool isGPU = isDiscreteGPU || isIntegratedGPU;

  int queueFamilies = findQueueFamily(physicalDevice);

  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice, mVulkanInstance.surface);
  bool swapchainCompatible = !swapChainSupport.formats.empty() &&
                             !swapChainSupport.presentModes.empty();

  return isGPU && deviceFeatures.geometryShader && queueFamilies != -1 &&
         swapchainCompatible;
}

int mental::DemoVulkanApp::findQueueFamily(VkPhysicalDevice physicalDevice) {
  return findQueueFamiliesWithPresentSupport(
      physicalDevice, VK_QUEUE_GRAPHICS_BIT, mVulkanInstance.surface);
}

bool mental::DemoVulkanApp::fillCommandBuffers(size_t i,
                                               uint32_t indexBufferCount) {
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

  MENTAL_VK_CHECK(vkBeginCommandBuffer(mVulkanRenderDevice.commandBuffers[i],
                                       &bufferBeginInfo));

  const VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
      .renderPass = mVulkanState.renderPass,
      .framebuffer = mVulkanState.swapchainFramebuffers[i],
      .renderArea = screenRect,
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data()};

  vkCmdBeginRenderPass(mVulkanRenderDevice.commandBuffers[i], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(mVulkanRenderDevice.commandBuffers[i],
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mVulkanState.graphicsPipeline);

  vkCmdBindDescriptorSets(mVulkanRenderDevice.commandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mVulkanState.pipelineLayout, 0, 1,
                          mVulkanState.descriptorSets.data(), 0, nullptr);
  vkCmdDraw(mVulkanRenderDevice.commandBuffers[i], indexBufferCount, 1, 0, 0);

  vkCmdEndRenderPass(mVulkanRenderDevice.commandBuffers[i]);

  MENTAL_VK_CHECK(vkEndCommandBuffer(mVulkanRenderDevice.commandBuffers[i]));

  return true;
}

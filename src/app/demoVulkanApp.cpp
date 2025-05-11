#include "demoVulkanApp.hpp"
#include <array>
#include <cstdio>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <volk.h>

namespace {

constexpr uint32_t SCREEN_WIDTH = 1280;
constexpr uint32_t SCREEN_HEIGHT = 720;
const char* WINDOW_TITLE = "Demo Vulkan App";
constexpr bool FULLSCREEN_MODE = false;

constexpr VkClearColorValue CLEAR_VALUE_COLOR = {1.0f, 1.0f, 1.0f, 1.0f};

static constexpr std::array<const char*, 1> DEVICE_EXTENSIONS{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct UniformBuffer {
  glm::mat4 mvp;
};

} // namespace

app::DemoVulkanApp::DemoVulkanApp()
    : mVulkanInstance(), mVulkanRenderDevice(), mVulkanState(),
      mWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE, FULLSCREEN_MODE) {
  init();
}

app::DemoVulkanApp::~DemoVulkanApp() {}

void app::DemoVulkanApp::run() {

  uint32_t currentFrame = 0;
  while (!mWindow.shouldClose()) {
    render(currentFrame);

    mWindow.pollEvents();

    currentFrame = (currentFrame + 1) % mVulkanRenderDevice.maxFramesInFlight;
  }
}

void app::DemoVulkanApp::init() {
  volkInitialize();
  initVulkan();
}

void app::DemoVulkanApp::initVulkan() {
  glslang_initialize_process();

  vkFramework::initVulkanInstance(mVulkanInstance, &mWindow);
  bool isRenderDeviceInitialized = vkFramework::initVulkanRenderDevice(
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
  if (!isRenderDeviceInitialized) {
    exit(EXIT_FAILURE);
  }

  bool isTexturedBufferCreated = vkFramework::createTexturedVertexBuffer(
      mVulkanRenderDevice, "data/rubber_duck/scene.gltf",
      &mVulkanState.storageBuffer, &mVulkanState.storageBufferMemory,
      &mVulkanState.vertexBufferSize, &mVulkanState.indexBufferSize);
  bool uniformBuffersCreated = createUniformBuffers();
  if (!isTexturedBufferCreated || !uniformBuffersCreated) {
    printf("FATAL ERROR: Failed to create buffers");
    exit(EXIT_FAILURE);
  }

  if (!vkFramework::createVulkanImage(
          mVulkanRenderDevice, "data/rubber_duck/textures/Duck_baseColor.png",
          mVulkanState.texture)) {
    printf("FATAL ERROR: Failed to create model texture");
    exit(EXIT_FAILURE);
  }

  if (!vkFramework::createDepthResources(
          mVulkanRenderDevice, mVulkanRenderDevice.swapchainExtent.width,
          mVulkanRenderDevice.swapchainExtent.height,
          mVulkanState.depthTexture)) {
    printf("FATAL ERROR: Failed to create depth resources");
    exit(EXIT_FAILURE);
  }

  if (!vkFramework::createDescriptorPool(mVulkanRenderDevice, 1, 2, 1,
                                         &mVulkanState.descriptorPool) ||
      !createDescriptorSet(mVulkanState.vertexBufferSize,
                           mVulkanState.indexBufferSize) ||
      !vkFramework::createColorAndDepthRenderPass(
          mVulkanRenderDevice, true, &mVulkanState.renderPass,
          vkFramework::RenderPassCreateInfo{
              .clearColor = true,
              .clearDepth = true,
              .flags = vkFramework::RENDER_PASS_BIT_FIRST |
                       vkFramework::RENDER_PASS_BIT_LAST}) ||
      !vkFramework::createPipelineLayout(mVulkanRenderDevice.device,
                                         mVulkanState.descriptorSetLayout,
                                         &mVulkanState.pipelineLayout) ||
      !vkFramework::createGraphicsPipeline(mVulkanRenderDevice,
                                           mVulkanState.renderPass,
                                           mVulkanState.pipelineLayout,
                                           {"data/shaders/chapter03/VK02.vert",
                                            "data/shaders/chapter03/VK02.frag",
                                            "data/shaders/chapter03/VK02.geom"},
                                           &mVulkanState.graphicsPipeline)) {
    printf("FATAL ERROR: Failed to create graphics pipeline");
    exit(EXIT_FAILURE);
  }

  if (!vkFramework::createColorAndDepthFramebuffers(
          mVulkanRenderDevice, mVulkanState.renderPass,
          mVulkanState.depthTexture.imageView,
          mVulkanState.swapchainFramebuffers)) {
    printf("FATAL ERROR: Failed to create framebuffers");
    exit(EXIT_FAILURE);
  }

  glslang_finalize_process();
}

void app::DemoVulkanApp::render(uint32_t currentFrame) {
  vkWaitForFences(mVulkanRenderDevice.device, 1,
                  &mVulkanRenderDevice.inflightFences[currentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex = 0;
  VkResult acquireNextImageRes = vkAcquireNextImageKHR(
      mVulkanRenderDevice.device, mVulkanRenderDevice.swapchain, UINT64_MAX,
      mVulkanRenderDevice.swapchainImageSemaphores[currentFrame],
      VK_NULL_HANDLE, &imageIndex);
  if (acquireNextImageRes == VK_ERROR_OUT_OF_DATE_KHR || mFramebufferResized) {
    mFramebufferResized = false;
    recreateSwapchain(currentFrame);
    return;
  } else if (acquireNextImageRes != VK_SUCCESS &&
             acquireNextImageRes != VK_SUBOPTIMAL_KHR) {
    CHECK_BOOL(false);
  }

  vkResetFences(mVulkanRenderDevice.device, 1,
                &mVulkanRenderDevice.inflightFences[currentFrame]);

  VK_CHECK(vkResetCommandBuffer(
      mVulkanRenderDevice.commandBuffers[currentFrame], 0));

  float aspectRatio =
      static_cast<float>(mVulkanRenderDevice.swapchainExtent.width) /
      static_cast<float>(mVulkanRenderDevice.swapchainExtent.height);
  const glm::mat4 m1 = glm::rotate(
      glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.5f, -1.5f)) *
          glm::rotate(glm::mat4(1.f), glm::pi<float>(), glm::vec3(1, 0, 0)),
      (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::mat4 p = glm::perspective(45.0f, aspectRatio, 0.1f, 1000.0f);

  const UniformBuffer ubo{.mvp = p * m1};

  updateUniformBuffer(currentFrame, &ubo, sizeof(ubo));

  uint32_t indexBufferCount = static_cast<uint32_t>(
      mVulkanState.indexBufferSize / (sizeof(unsigned int)));
  fillCommandBuffers(imageIndex, currentFrame, indexBufferCount);

  const VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  const VkSubmitInfo si = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &mVulkanRenderDevice.swapchainImageSemaphores[currentFrame],
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &mVulkanRenderDevice.commandBuffers[currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &mVulkanRenderDevice.renderSemaphores[currentFrame]};

  VK_CHECK(vkQueueSubmit(mVulkanRenderDevice.graphicsQueue, 1, &si,
                         mVulkanRenderDevice.inflightFences[currentFrame]));

  const VkPresentInfoKHR pi = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &mVulkanRenderDevice.renderSemaphores[currentFrame],
      .swapchainCount = 1,
      .pSwapchains = &mVulkanRenderDevice.swapchain,
      .pImageIndices = &imageIndex};

  VkResult presentRes =
      vkQueuePresentKHR(mVulkanRenderDevice.graphicsQueue, &pi);
  if (presentRes == VK_ERROR_OUT_OF_DATE_KHR ||
      presentRes == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
    mFramebufferResized = false;
    recreateSwapchain(currentFrame);
  } else if (presentRes != VK_SUCCESS) {
    CHECK_BOOL(false);
  }
}

void app::DemoVulkanApp::cleanup() {
  destroyVulkanState();
  vkFramework::destroyVulkanRenderDevice(mVulkanRenderDevice);
  vkFramework::destroyVulkanInstance(mVulkanInstance);
}

void app::DemoVulkanApp::cleanupSwapchain(VkSwapchainKHR swapchain) {
  vkFramework::destroyVulkanImage(mVulkanRenderDevice.device,
                                  mVulkanState.depthTexture);

  for (VkFramebuffer framebuffer : mVulkanState.swapchainFramebuffers) {
    vkDestroyFramebuffer(mVulkanRenderDevice.device, framebuffer, nullptr);
  }

  for (VkImageView imageView : mVulkanRenderDevice.swapchainImageViews) {
    vkDestroyImageView(mVulkanRenderDevice.device, imageView, nullptr);
  }

  vkDestroySwapchainKHR(mVulkanRenderDevice.device, swapchain, nullptr);
}

void app::DemoVulkanApp::destroyVulkanState() {
  vkDestroyBuffer(mVulkanRenderDevice.device, mVulkanState.storageBuffer,
                  nullptr);
  vkFreeMemory(mVulkanRenderDevice.device, mVulkanState.storageBufferMemory,
               nullptr);

  for (size_t i = 0; i < mVulkanState.uniformBuffers.size(); i++) {
    vkDestroyBuffer(mVulkanRenderDevice.device, mVulkanState.uniformBuffers[i],
                    nullptr);
  }

  for (size_t i = 0; i < mVulkanState.uniformBuffersMemory.size(); i++) {
    vkFreeMemory(mVulkanRenderDevice.device,
                 mVulkanState.uniformBuffersMemory[i], nullptr);
  }

  vkDestroyDescriptorSetLayout(mVulkanRenderDevice.device,
                               mVulkanState.descriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(mVulkanRenderDevice.device,
                          mVulkanState.descriptorPool, nullptr);

  for (VkFramebuffer framebuffer : mVulkanState.swapchainFramebuffers) {
    vkDestroyFramebuffer(mVulkanRenderDevice.device, framebuffer, nullptr);
  }

  destroyVulkanImage(mVulkanRenderDevice.device, mVulkanState.texture);

  destroyVulkanImage(mVulkanRenderDevice.device, mVulkanState.depthTexture);

  vkDestroyRenderPass(mVulkanRenderDevice.device, mVulkanState.renderPass,
                      nullptr);

  vkDestroyPipelineLayout(mVulkanRenderDevice.device,
                          mVulkanState.pipelineLayout, nullptr);
  vkDestroyPipeline(mVulkanRenderDevice.device, mVulkanState.graphicsPipeline,
                    nullptr);
}

void app::DemoVulkanApp::recreateSwapchain(uint32_t currentFrame) {
  window::Window::Size framebufferSize{};
  do {
    framebufferSize = mWindow.getSize();
    mWindow.waitEvents();
  } while (framebufferSize.width == 0 || framebufferSize.height == 0);

  vkWaitForFences(mVulkanRenderDevice.device, 1,
                  &mVulkanRenderDevice.inflightFences[currentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t newWidth = static_cast<uint32_t>(framebufferSize.width);
  uint32_t newHeight = static_cast<uint32_t>(framebufferSize.height);

  VkSwapchainKHR oldSwapchain = mVulkanRenderDevice.swapchain;
  VK_CHECK(
      vkFramework::createSwapchain(mVulkanRenderDevice, mVulkanInstance.surface,
                                   mVulkanRenderDevice.graphicsFamily, newWidth,
                                   newHeight, &mVulkanRenderDevice.swapchain));

  cleanupSwapchain(oldSwapchain);

  size_t imageCount = vkFramework::createSwapchainImages(
      mVulkanRenderDevice.device, mVulkanRenderDevice.swapchain,
      mVulkanRenderDevice.swapchainImages,
      mVulkanRenderDevice.swapchainImageViews);
  CHECK_BOOL(static_cast<uint32_t>(imageCount) ==
             mVulkanRenderDevice.swapchainImageCount);

  CHECK_BOOL(vkFramework::createDepthResources(
      mVulkanRenderDevice, mVulkanRenderDevice.swapchainExtent.width,
      mVulkanRenderDevice.swapchainExtent.height, mVulkanState.depthTexture));

  CHECK_BOOL(vkFramework::createColorAndDepthFramebuffers(
      mVulkanRenderDevice, mVulkanState.renderPass,
      mVulkanState.depthTexture.imageView, mVulkanState.swapchainFramebuffers));
}

bool app::DemoVulkanApp::createUniformBuffers() {
  VkDeviceSize bufferSize = sizeof(UniformBuffer);

  mVulkanState.uniformBuffers.resize(mVulkanRenderDevice.maxFramesInFlight);
  mVulkanState.uniformBuffersMemory.resize(
      mVulkanRenderDevice.maxFramesInFlight);

  for (size_t i = 0; i < mVulkanState.uniformBuffers.size(); i++) {
    bool isBufferCreated = vkFramework::createBuffer(
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

void app::DemoVulkanApp::updateUniformBuffer(uint32_t frameIndex,
                                             const void* uboData,
                                             size_t uboSize) {
  void* data = nullptr;
  vkMapMemory(mVulkanRenderDevice.device,
              mVulkanState.uniformBuffersMemory[frameIndex], 0, uboSize, 0,
              &data);
  memcpy(data, uboData, uboSize);
  vkUnmapMemory(mVulkanRenderDevice.device,
                mVulkanState.uniformBuffersMemory[frameIndex]);
}

bool app::DemoVulkanApp::createDescriptorSet(size_t vertexBufferSize,
                                             size_t indexBufferSize) {
  const std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
      vkFramework::descriptorSetLayoutBinding(
          0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
      vkFramework::descriptorSetLayoutBinding(
          1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
      vkFramework::descriptorSetLayoutBinding(
          2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
      vkFramework::descriptorSetLayoutBinding(
          3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT)};

  const VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data()};

  VK_CHECK(vkCreateDescriptorSetLayout(mVulkanRenderDevice.device, &layoutInfo,
                                       nullptr,
                                       &mVulkanState.descriptorSetLayout));
  std::vector<VkDescriptorSetLayout> layouts(
      mVulkanRenderDevice.maxFramesInFlight, mVulkanState.descriptorSetLayout);

  const VkDescriptorSetAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = mVulkanState.descriptorPool,
      .descriptorSetCount = mVulkanRenderDevice.maxFramesInFlight,
      .pSetLayouts = layouts.data()};

  mVulkanState.descriptorSets.resize(mVulkanRenderDevice.maxFramesInFlight);
  VK_CHECK(vkAllocateDescriptorSets(mVulkanRenderDevice.device, &allocInfo,
                                    mVulkanState.descriptorSets.data()));

  for (uint32_t i = 0; i < mVulkanRenderDevice.maxFramesInFlight; i++) {
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
        .sampler = mVulkanState.texture.sampler,
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

bool app::DemoVulkanApp::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
  bool extensionsSupported = vkFramework::checkDeviceExtensionSupport(
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

  vkFramework::SwapChainSupportDetails swapChainSupport =
      vkFramework::querySwapChainSupport(physicalDevice,
                                         mVulkanInstance.surface);
  bool swapchainCompatible = !swapChainSupport.formats.empty() &&
                             !swapChainSupport.presentModes.empty();

  return isGPU && deviceFeatures.geometryShader && queueFamilies != -1 &&
         swapchainCompatible;
}

int app::DemoVulkanApp::findQueueFamily(VkPhysicalDevice physicalDevice) {
  return vkFramework::findQueueFamiliesWithPresentSupport(
      physicalDevice, VK_QUEUE_GRAPHICS_BIT, mVulkanInstance.surface);
}

bool app::DemoVulkanApp::fillCommandBuffers(size_t imageIndex,
                                            uint32_t frameIndex,
                                            uint32_t indexBufferCount) {
  const VkCommandBufferBeginInfo bufferBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
      .pInheritanceInfo = nullptr};

  const std::array<VkClearValue, 2> clearValues = {
      VkClearValue{.color = CLEAR_VALUE_COLOR},
      VkClearValue{.depthStencil = {1.0f, 0}}};

  const VkRect2D screenRect = {.offset = {0, 0},
                               .extent = mVulkanRenderDevice.swapchainExtent};

  VK_CHECK(vkBeginCommandBuffer(mVulkanRenderDevice.commandBuffers[frameIndex],
                                &bufferBeginInfo));

  const VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
      .renderPass = mVulkanState.renderPass,
      .framebuffer = mVulkanState.swapchainFramebuffers[imageIndex],
      .renderArea = screenRect,
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data()};

  vkCmdBeginRenderPass(mVulkanRenderDevice.commandBuffers[frameIndex],
                       &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(mVulkanRenderDevice.commandBuffers[frameIndex],
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mVulkanState.graphicsPipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width =
      static_cast<float>(mVulkanRenderDevice.swapchainExtent.width);
  viewport.height =
      static_cast<float>(mVulkanRenderDevice.swapchainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(mVulkanRenderDevice.commandBuffers[frameIndex], 0, 1,
                   &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = mVulkanRenderDevice.swapchainExtent;
  vkCmdSetScissor(mVulkanRenderDevice.commandBuffers[frameIndex], 0, 1,
                  &scissor);

  vkCmdBindDescriptorSets(mVulkanRenderDevice.commandBuffers[frameIndex],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mVulkanState.pipelineLayout, 0, 1,
                          mVulkanState.descriptorSets.data(), 0, nullptr);
  vkCmdDraw(mVulkanRenderDevice.commandBuffers[frameIndex], indexBufferCount, 1,
            0, 0);

  vkCmdEndRenderPass(mVulkanRenderDevice.commandBuffers[frameIndex]);

  VK_CHECK(vkEndCommandBuffer(mVulkanRenderDevice.commandBuffers[frameIndex]));

  return true;
}

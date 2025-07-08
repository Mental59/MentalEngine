#include "demoVulkanApp.hpp"
#include "camera/camera.hpp"
#include "camera/firstPersonCameraPositioner.hpp"
#include "utils/fpsCounter.hpp"
#include "vkFramework/render/layers.hpp"
#include <array>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <volk.h>

namespace {

constexpr uint32_t gScreenWidth = 1280;
constexpr uint32_t gScreenHeight = 720;
const char* gWindowTitle = "Demo Vulkan App";
constexpr bool gFullScreenMode = false;

constexpr glm::vec3 gInitialCameraPos = glm::vec3(0.0f, 2.0f, 0.0f);
constexpr glm::vec3 gInitialCameraTarget = glm::vec3(0.f, 0.5f, -1.5f);

static constexpr std::array<const char*, 2> gDeviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME};

FpsCounter gFpsCounter(0.1f);

camera::FirstPersonCameraPositioner gFpsPositioner(gInitialCameraPos,
                                                   gInitialCameraTarget);
camera::Camera gCamera(gFpsPositioner);

} // namespace

app::DemoVulkanApp::DemoVulkanApp()
    : mVulkanInstance(), mVulkanRenderDevice(),
      mWindow(gScreenWidth, gScreenHeight, gWindowTitle, gFullScreenMode, this),
      mImguiLayer(), mModelRenderer(), mClearLayer(), mCanvasLayer(),
      mFinishLayer() {
  init();
}

app::DemoVulkanApp::~DemoVulkanApp() { cleanup(); }

void app::DemoVulkanApp::run() {

  uint32_t currentFrame = 0;

  double timeStamp = mWindow.getTime();
  float deltaSeconds = 0.0f;

  while (!mWindow.shouldClose()) {
    const double newTimeStamp = mWindow.getTime();
    deltaSeconds = static_cast<float>(newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;

    app::MouseState prevMouseState = mMouseState;
    mWindow.pollEvents();

    const bool prevShouldUpdatePositioner = prevMouseState.pressedRight;
    const bool curShouldUpdatePositioner = mMouseState.pressedRight;

    if (!prevShouldUpdatePositioner && curShouldUpdatePositioner) {
      gFpsPositioner.resetMousePosition(mMouseState.posNormalized);
    }

    if (curShouldUpdatePositioner) {
      gFpsPositioner.updateRotation(mMouseState.posNormalized);
      gFpsPositioner.updatePosition(deltaSeconds);
    }

    bool frameRendered = render(currentFrame);
    gFpsCounter.tick(deltaSeconds, frameRendered);

    currentFrame = (currentFrame + 1) % mVulkanRenderDevice.maxFramesInFlight;
  }
}

void app::DemoVulkanApp::onMousePosUpdate(float x, float y, int width,
                                          int height) {
  BaseApp::onMousePosUpdate(x, y, width, height);
  ImGui::GetIO().MousePos = ImVec2(x, y);
}

void app::DemoVulkanApp::onMousePressedLeft(bool pressed) {
  BaseApp::onMousePressedLeft(pressed);

  ImGuiIO& io = ImGui::GetIO();
  io.MouseDown[0] = pressed;
}

void app::DemoVulkanApp::onMousePressedRight(bool pressed) {
  BaseApp::onMousePressedRight(pressed);

  ImGuiIO& io = ImGui::GetIO();
  io.MouseDown[2] = pressed;

  if (pressed) {
    mWindow.hideCursor();
  } else {
    mWindow.showCursor();
  }
}

void app::DemoVulkanApp::onMousePressedMiddle(bool pressed) {
  BaseApp::onMousePressedMiddle(pressed);

  ImGuiIO& io = ImGui::GetIO();
  io.MouseDown[1] = pressed;
}

void app::DemoVulkanApp::onKeyPressed(int key, bool pressed) {
  if (key == GLFW_KEY_W) {
    gFpsPositioner.mMovement.forward = pressed;
  }

  if (key == GLFW_KEY_S) {
    gFpsPositioner.mMovement.backward = pressed;
  }

  if (key == GLFW_KEY_A) {
    gFpsPositioner.mMovement.left = pressed;
  }

  if (key == GLFW_KEY_D) {
    gFpsPositioner.mMovement.right = pressed;
  }

  if (key == GLFW_KEY_E) {
    gFpsPositioner.mMovement.up = pressed;
  }

  if (key == GLFW_KEY_Q) {
    gFpsPositioner.mMovement.down = pressed;
  }

  if (key == GLFW_KEY_L) {
    gFpsPositioner.lookAt(gInitialCameraTarget);
  }
}

void app::DemoVulkanApp::init() {
  volkInitialize();
  initGUI();
  initVulkan();
  initCanvas();
}

void app::DemoVulkanApp::initVulkan() {
  glslang_initialize_process();

  vkFramework::initVulkanInstance(mVulkanInstance, &mWindow);

  CHECK_BOOL(vkFramework::initVulkanRenderDevice(
      mVulkanInstance, gScreenWidth, gScreenHeight,
      [this](VkPhysicalDevice physicalDevice) {
        return isDeviceSuitable(physicalDevice);
      },
      [this](VkPhysicalDevice physicalDevice) {
        return findQueueFamily(physicalDevice);
      },
      VkPhysicalDeviceFeatures{.geometryShader = VK_TRUE},
      static_cast<uint32_t>(gDeviceExtensions.size()), gDeviceExtensions.data(),
      mVulkanRenderDevice));

  CHECK_BOOL(createDepthResources(
      mVulkanRenderDevice, mVulkanRenderDevice.swapchainExtent.width,
      mVulkanRenderDevice.swapchainExtent.height, &mDepthTexture));

  mImguiLayer.init(&mVulkanRenderDevice);
  mModelRenderer.init(&mVulkanRenderDevice, "data/rubber_duck/scene.gltf",
                      "data/rubber_duck/textures/Duck_baseColor.png",
                      (uint32_t)sizeof(glm::mat4), &mDepthTexture);
  mCanvasLayer.init(&mVulkanRenderDevice, &mDepthTexture);
  mClearLayer.init(&mVulkanRenderDevice, &mDepthTexture);
  mFinishLayer.init(&mVulkanRenderDevice, &mDepthTexture);
  mGridLayer.init(&mVulkanRenderDevice, &mDepthTexture);

  mLayers.reserve(6);
  mLayers.push_back(&mClearLayer);
  mLayers.push_back(&mModelRenderer);
  mLayers.push_back(&mCanvasLayer);
  mLayers.push_back(&mGridLayer);
  mLayers.push_back(&mImguiLayer);
  mLayers.push_back(&mFinishLayer);

  glslang_finalize_process();
}

void app::DemoVulkanApp::initGUI() { ImGui::CreateContext(); }

void app::DemoVulkanApp::initCanvas() {
  mCanvasLayer.plane3d(glm::vec3(0, +1.5, 0), glm::vec3(1, 0, 0),
                       glm::vec3(0, 0, 1), 10, 10, 100.0f, 100.0f,
                       glm::vec4(1, 0, 0, 1), glm::vec4(0, 1, 0, 1));

  for (uint32_t i = 0; i < mVulkanRenderDevice.maxFramesInFlight; i++) {
    mCanvasLayer.updateBuffer(i);
  }
}

void app::DemoVulkanApp::composeFrame(uint32_t currentFrame,
                                      uint32_t currentImage) {
  update3D(currentFrame);
  updateGUI(currentFrame);

  VkCommandBuffer commandBuffer =
      mVulkanRenderDevice.commandBuffers[currentFrame];

  const VkCommandBufferBeginInfo bi = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
      .pInheritanceInfo = nullptr};

  VK_CHECK(vkResetCommandBuffer(
      mVulkanRenderDevice.commandBuffers[currentFrame], 0));
  VK_CHECK(vkBeginCommandBuffer(commandBuffer, &bi));

  for (vkFramework::render::BaseRenderLayer* layer : mLayers) {
    layer->fillCommandBuffer(commandBuffer, currentFrame, currentImage);
  }

  VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

bool app::DemoVulkanApp::render(uint32_t currentFrame) {
  vkWaitForFences(mVulkanRenderDevice.device, 1,
                  &mVulkanRenderDevice.inflightFences[currentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex = 0;
  VkResult acquireNextImageRes = vkAcquireNextImageKHR(
      mVulkanRenderDevice.device, mVulkanRenderDevice.swapchain, UINT64_MAX,
      mVulkanRenderDevice.swapchainImageSemaphores[currentFrame],
      VK_NULL_HANDLE, &imageIndex);
  if (acquireNextImageRes == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain(currentFrame);
    return false;
  } else if (acquireNextImageRes != VK_SUCCESS &&
             acquireNextImageRes != VK_SUBOPTIMAL_KHR) {
    CHECK_BOOL(false);
  }

  vkResetFences(mVulkanRenderDevice.device, 1,
                &mVulkanRenderDevice.inflightFences[currentFrame]);

  composeFrame(currentFrame, imageIndex);

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
    return false;
  } else if (presentRes != VK_SUCCESS) {
    CHECK_BOOL(false);
  }

  return true;
}

void app::DemoVulkanApp::updateGUI(uint32_t currentFrame) {
  window::Window::Size framebufferSize = mWindow.getSize();

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(framebufferSize.width),
                          static_cast<float>(framebufferSize.height));
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));

  const ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize;
  ImGui::Begin("Statistics", nullptr, flags);
  ImGui::Text("FPS: %.2f", gFpsCounter.getFPS());
  ImGui::End();

  ImGui::Render();
  mImguiLayer.updateBuffers(currentFrame, ImGui::GetDrawData());
}

void app::DemoVulkanApp::update3D(uint32_t currentFrame) {
  window::Window::Size framebufferSize = mWindow.getSize();

  glm::mat4 m1(1.0f);
  m1 = glm::translate(m1, glm::vec3(0.f, 0.5f, -1.5f));
  m1 = glm::rotate(m1, static_cast<float>(mWindow.getTime()),
                   glm::vec3(0.0f, 1.0f, 0.0f));

  glm::mat4 p =
      glm::perspectiveRH_ZO(45.0f, framebufferSize.ratio, 0.01f, 1000.0f);
  p[1][1] *= -1.0f; // invert y axis

  const glm::mat4 view = gCamera.getViewMatrix();
  const glm::mat4 mtx = p * view * m1;

  mModelRenderer.updateUniformBuffer(currentFrame, glm::value_ptr(mtx),
                                     sizeof(glm::mat4));
  // mCanvasLayer.updateUniformBuffer(
  //     p * view * glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 2.f, 0.f)),
  //     0.0f, currentFrame);

  mGridLayer.updateUniformBuffer(p * view, gCamera.getPosition(), currentFrame);
}

void app::DemoVulkanApp::cleanup() {
  vkDeviceWaitIdle(mVulkanRenderDevice.device);

  cleanupGUI();

  for (vkFramework::render::BaseRenderLayer* layer : mLayers) {
    layer->destroy();
  }

  vkFramework::destroyVulkanImage(mVulkanRenderDevice.device, &mDepthTexture);

  vkFramework::destroyVulkanRenderDevice(mVulkanRenderDevice);
  vkFramework::destroyVulkanInstance(mVulkanInstance);
}

void app::DemoVulkanApp::cleanupSwapchain(VkSwapchainKHR swapchain) {
  vkFramework::destroyVulkanImage(mVulkanRenderDevice.device, &mDepthTexture);

  destroyFramebuffers();

  for (VkImageView imageView : mVulkanRenderDevice.swapchainImageViews) {
    vkDestroyImageView(mVulkanRenderDevice.device, imageView, nullptr);
  }

  vkDestroySwapchainKHR(mVulkanRenderDevice.device, swapchain, nullptr);
}

void app::DemoVulkanApp::cleanupGUI() { ImGui::DestroyContext(); }

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
      mVulkanRenderDevice.swapchainExtent.height, &mDepthTexture));

  recreateFramebuffers(mDepthTexture);
}

void app::DemoVulkanApp::destroyFramebuffers() {
  for (vkFramework::render::BaseRenderLayer* layer : mLayers) {
    layer->destroyFramebuffers();
  }
}

void app::DemoVulkanApp::recreateFramebuffers(
    vkFramework::VulkanImage newDepthTexture) {
  for (vkFramework::render::BaseRenderLayer* layer : mLayers) {
    CHECK_BOOL(layer->createFramebuffers());
  }
}

bool app::DemoVulkanApp::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
  bool extensionsSupported = vkFramework::checkDeviceExtensionSupport(
      physicalDevice, gDeviceExtensions.data(),
      static_cast<uint32_t>(gDeviceExtensions.size()));
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

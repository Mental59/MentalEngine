#pragma once

#include "baseApp.hpp"
#include "vkFramework/includes.hpp"
#include "vkFramework/render/layers.hpp"
#include "window/window.hpp"
#include <vector>
#include <volk.h>

namespace app {

class DemoVulkanApp : public BaseApp {
public:
  DemoVulkanApp();
  ~DemoVulkanApp();

  void run();

  virtual void onMousePosUpdate(float x, float y, int width,
                                int height) override;
  virtual void onMousePressedLeft(bool pressed) override;
  virtual void onMousePressedRight(bool pressed) override;
  virtual void onMousePressedMiddle(bool pressed) override;
  virtual void onKeyPressed(int key, bool pressed) override;

private:
  void init();
  void initVulkan();
  void initGUI();
  void initCanvas();

  void composeFrame(uint32_t currentFrame, uint32_t currentImage);

  bool render(uint32_t currentFrame);

  void updateGUI(uint32_t currentFrame);
  void update3D(uint32_t currentFrame);
  void update2D(uint32_t currentFrame);

  void cleanup();
  void cleanupSwapchain(VkSwapchainKHR swapchain);
  void cleanupGUI();

  void recreateSwapchain(uint32_t currentFrame);

  bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
  int findQueueFamily(VkPhysicalDevice physicalDevice);

  void destroyFramebuffers();
  void recreateFramebuffers(vkFramework::VulkanImage newDepthTexture);

  vkFramework::VulkanInstance mVulkanInstance;
  vkFramework::VulkanRenderDevice mVulkanRenderDevice;
  vkFramework::VulkanImage mDepthTexture;

  vkFramework::render::ImGuiLayer mImguiLayer;
  vkFramework::render::ModelRenderLayer mModelRenderer;
  vkFramework::render::CanvasLayer mCanvasLayer;
  vkFramework::render::ClearLayer mClearLayer;
  vkFramework::render::FinishLayer mFinishLayer;

  std::vector<vkFramework::render::BaseRenderLayer*> mLayers;

  window::Window mWindow;
};

} // namespace app

#include "vulkanFramework/vulkanFramework.hpp"
#include "window/window.hpp"
#include <volk.h>

int main() {
  volkInitialize();

  mental::Window window(1280, 720, "Mental engine editor");

  mental::VulkanInstance vkInstance;
  mental::initVulkanInstance(vkInstance, &window);

  while (!window.shouldClose()) {
    window.pollEvents();
  }

  mental::destroyVulkanInstance(vkInstance);

  return 0;
}

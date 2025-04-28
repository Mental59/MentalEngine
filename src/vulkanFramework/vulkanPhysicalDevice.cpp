#include "vulkanPhysicalDevice.hpp"
#include "vulkanUtils.hpp"
#include <vector>

VkResult
mental::findSuitablePhysicalDevice(VkInstance instance,
                                   PhysicalDeviceSelectorFunction selector,
                                   VkPhysicalDevice* physicalDevice) {
  uint32_t deviceCount = 0;
  MENTAL_VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

  if (!deviceCount) {
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  MENTAL_VK_CHECK(
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

  for (const VkPhysicalDevice device : devices) {
    if (selector(device)) {
      *physicalDevice = device;
      return VK_SUCCESS;
    }
  }

  return VK_ERROR_INITIALIZATION_FAILED;
}

int mental::findQueueFamilies(VkPhysicalDevice physicalDevice,
                              VkQueueFlags desiredFlags) {
  uint32_t familyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> families(familyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount,
                                           families.data());

  for (int i = 0; i != families.size(); i++) {
    if (families[i].queueCount > 0 && families[i].queueFlags & desiredFlags) {
      return i;
    }
  }

  return -1;
}

int mental::findQueueFamiliesWithPresentSupport(VkPhysicalDevice physicalDevice,
                                                VkQueueFlags desiredFlags,
                                                VkSurfaceKHR surface) {
  uint32_t familyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> families(familyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount,
                                           families.data());

  for (int i = 0; i != families.size(); i++) {
    VkBool32 presentSupport;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
                                         &presentSupport);

    if (families[i].queueCount > 0 && families[i].queueFlags & desiredFlags &&
        presentSupport) {
      return i;
    }
  }

  return -1;
}

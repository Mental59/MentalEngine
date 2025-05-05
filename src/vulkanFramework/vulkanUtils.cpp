#include "vulkanUtils.hpp"
#include "utils/utils.hpp"
#include <cassert>
#include <cstdlib>
#include <string>

void mental::check(bool check, const char* fileName, int lineNumber) {
  if (!check) {
    printf("CHECK() failed at %s:%i\n", fileName, lineNumber);
    assert(false);
    exit(EXIT_FAILURE);
  }
}

VkResult mental::createSemaphore(VkDevice device, VkSemaphore* outSemaphore) {
  const VkSemaphoreCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  return vkCreateSemaphore(device, &createInfo, nullptr, outSemaphore);
}

bool mental::hasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t mental::bytesPerTexFormat(VkFormat fmt) {
  switch (fmt) {
  case VK_FORMAT_R8_SINT:
  case VK_FORMAT_R8_UNORM:
    return 1;
  case VK_FORMAT_R16_SFLOAT:
    return 2;
  case VK_FORMAT_R16G16_SFLOAT:
    return 4;
  case VK_FORMAT_R16G16_SNORM:
    return 4;
  case VK_FORMAT_B8G8R8A8_UNORM:
    return 4;
  case VK_FORMAT_R8G8B8A8_UNORM:
    return 4;
  case VK_FORMAT_R16G16B16A16_SFLOAT:
    return 4 * sizeof(uint16_t);
  case VK_FORMAT_R32G32B32A32_SFLOAT:
    return 4 * sizeof(float);
  default:
    break;
  }
  return 0;
}

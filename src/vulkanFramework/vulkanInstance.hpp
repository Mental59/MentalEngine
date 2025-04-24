#pragma once

#include <volk.h>

namespace mental
{
VkInstance createVulkanInstance();
void destroyVulkanInstance(VkInstance instance);
}

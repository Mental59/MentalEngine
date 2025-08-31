#pragma once
#include <render/rhi/vulkan/device.hpp>

namespace mental::rhi::vk
{
class DeviceFactory
{
public:
    DeviceHandle create();
};
}  // namespace mental::rhi::vk

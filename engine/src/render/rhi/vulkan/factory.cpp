#include <render/rhi/vulkan/factory.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <vulkan/vulkan.hpp>

namespace mental::rhi::vk
{
DeviceHandle DeviceFactory::create()
{
    ::vk::InstanceCreateInfo instanceCreateInfo;
    auto instanceCreateRes = ::vk::createInstance(instanceCreateInfo);

    if (instanceCreateRes.result != ::vk::Result::eSuccess)
    {
        return nullptr;
    }

    return createDevice({.instance = instanceCreateRes.value});
}
}  // namespace mental::rhi::vk

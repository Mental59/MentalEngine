#pragma once
#include <render/rhi/vulkan/device.hpp>
#include <vulkan/vulkan.hpp>
#include <vector>

namespace mental::platform
{
class IWindow;
}

namespace mental::rhi::vk
{
class DeviceFactory
{
public:
    DeviceHandle create(const mental::platform::IWindow* const window) const;

private:
    ::vk::Instance createInstance() const;

    bool checkInstanceExtensionSupport(const std::vector<const char*>& extensions) const;
    bool checkInstanceLayerSupport(const std::vector<const char*>& layers) const;

#if defined(_DEBUG)
    void setupDebugMessenger(::vk::Instance instance) const;
#endif
};
}  // namespace mental::rhi::vk

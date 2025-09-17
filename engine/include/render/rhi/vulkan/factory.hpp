#pragma once
#include <render/rhi/vulkan/device.hpp>
#include <vulkan/vulkan.hpp>
#include <vector>

namespace mental::rhi::vk
{
struct DebugMessenger
{
    ::vk::DebugUtilsMessengerEXT utilsMessenger;
    ::vk::DebugReportCallbackEXT reportCallback;
};

class DeviceFactory
{
public:
    DeviceHandle create(const ::vk::Instance& instance, const ::vk::SurfaceKHR& surface) const;
    ::vk::Instance createInstance() const;

private:
    bool checkInstanceExtensionSupport(const std::vector<const char*>& extensions) const;
    bool checkInstanceLayerSupport(const std::vector<const char*>& layers) const;

#if defined(_DEBUG)
    DebugMessenger createDebugMessenger(::vk::Instance instance) const;
#endif
};
}  // namespace mental::rhi::vk

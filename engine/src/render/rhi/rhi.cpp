#include <render/rhi/rhi.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#if defined MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/factory.hpp>
#endif

namespace mental::rhi
{
const char* resultToString(Result res)
{
    switch (res)
    {
        case mental::rhi::Result::eSuccess: return "RHI_SUCCESS";
        case mental::rhi::Result::eInstanceInitializationFailed: return "RHI_INSTANCE_INITIALIZATION_FAILED";
        case mental::rhi::Result::ePhysicalDeviceInitializationFailed: return "RHI_PHYSICAL_DEVICE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eLogicalDeviceInitializationFailed: return "RHI_LOGICAL_DEVICE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eBufferInitializationFailed: return "RHI_BUFFER_INITIALIZATION_FAILED";
        case mental::rhi::Result::eBufferUploadFailed: return "RHI_BUFFER_UPLOAD_FAILED";

        default:
        {
            static char buf[24];
            snprintf(buf, sizeof(buf), "Unknown (%d)", res);
            return buf;
        }
    }
}

const char* graphicsApiToString(GraphicsApi api)
{
    switch (api)
    {
        case GraphicsApi::Vulkan:
        {
            return "Vulkan";
        }
        default:
        {
            return "";
        }
    }
}

DeviceHandle createDevice(GraphicsApi api, const mental::platform::IWindow* const window)
{
    DeviceHandle device{};

    switch (api)
    {
#if defined MENTAL_WITH_VULKAN
        case GraphicsApi::Vulkan:
        {

            rhi::Result res;
            rhi::vk::DeviceFactory factory;

            rhi::vk::InstanceInfo instanceInfo;
            res = factory.createInstance(instanceInfo);
            if (res != rhi::Result::eSuccess) mental::core::log::fatal("Failed to create vulkan instance. Error: %s", resultToString(res));

            VkSurfaceKHR surface = window->createSurface(instanceInfo.getInstance());
            res = factory.create(instanceInfo, surface, device);
            if (res != rhi::Result::eSuccess) mental::core::log::fatal("Failed to create vulkan device. Error: %s", resultToString(res));

            break;
        }
#endif

        default:
        {
            mental::core::log::fatal("Unsupported graphics api %s", graphicsApiToString(api));
        }
    }

    return device;
}
}  // namespace mental::rhi

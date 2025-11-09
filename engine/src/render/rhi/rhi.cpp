#include <render/rhi/rhi.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#include <format>
#if defined MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/factory.hpp>
#endif

namespace mental::rhi
{
static IDevice* gDevice = nullptr;

const char* resultToString(Result res)
{
    switch (res)
    {
        case mental::rhi::Result::eSuccess: return "RHI_SUCCESS";
        case mental::rhi::Result::eDeviceInitializationFailed: return "RHI_DEVICE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eInstanceInitializationFailed: return "RHI_INSTANCE_INITIALIZATION_FAILED";
        case mental::rhi::Result::ePhysicalDeviceInitializationFailed: return "RHI_PHYSICAL_DEVICE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eLogicalDeviceInitializationFailed: return "RHI_LOGICAL_DEVICE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eSurfaceInitializationFailed: return "RHI_SURFACE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eBufferInitializationFailed: return "RHI_BUFFER_INITIALIZATION_FAILED";
        case mental::rhi::Result::eBufferMapFailed: return "RHI_BUFFER_MAP_FAILED";
        case mental::rhi::Result::eBufferCopyFailed: return "RHI_BUFFER_COPY_FAILED";
        case mental::rhi::Result::eCommandQueueInitializationFailed: return "RHI_COMMAND_QUEUE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eQueueSubmitFailed: return "RHI_QUEUE_SUBMIT_FAILED";
        case mental::rhi::Result::eSemaphoreInitializationFailed: return "RHI_SEMAPHORE_INITIALIZATION_FAILED";

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

void initDevice(GraphicsApi api, const mental::platform::IWindow* const window)
{
    switch (api)
    {
#if defined MENTAL_WITH_VULKAN
        case GraphicsApi::Vulkan:
        {
            rhi::Result res;
            rhi::vk::DeviceFactory factory;

            rhi::vk::InstanceInfo instanceInfo;
            res = factory.createInstance(instanceInfo);
            MENTAL_ASSERT_MESSAGE(
                res == rhi::Result::eSuccess, std::format("Failed to create vulkan instance. Error: {}", resultToString(res)));

            VkSurfaceKHR surface;
            res = window->createSurface(instanceInfo.getInstance(), surface);
            MENTAL_ASSERT_MESSAGE(
                res == rhi::Result::eSuccess, std::format("Failed to create vulkan surface. Error: {}", resultToString(res)));

            res = factory.initDevice(instanceInfo, surface);
            MENTAL_ASSERT_MESSAGE(
                res == rhi::Result::eSuccess, std::format("Failed to create vulkan device. Error: {}", resultToString(res)));

            gDevice = &rhi::vk::getDevice();
            break;
        }
#endif

        default:
        {
            MENTAL_ASSERT_MESSAGE(false, std::format("Unsupported graphics api {}", graphicsApiToString(api)));
        }
    }
}

IDevice& getDevice()
{
    return *gDevice;
}

}  // namespace mental::rhi

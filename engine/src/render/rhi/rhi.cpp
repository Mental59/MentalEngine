#include <render/rhi/rhi.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#if defined MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/factory.hpp>
#endif

namespace mental::rhi
{
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

IDevice* createDevice(GraphicsApi api, const mental::platform::IWindow* const window)
{
    IDevice* device;

    switch (api)
    {
#if defined MENTAL_WITH_VULKAN
        case GraphicsApi::Vulkan:
        {
            vk::DeviceFactory factory;

            vk::InstanceInfo instanceInfo = factory.createInstance();
            ::vk::SurfaceKHR surface = window->createSurface(instanceInfo.getInstance());

            device = factory.create(instanceInfo, surface);

            if (!device)
            {
                mental::core::log::fatal("Failed to create Vulkan device");
            }

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

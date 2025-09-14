#include <render/rhi/rhi.hpp>
#include <core/log.hpp>
#if defined MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/factory.hpp>
#endif

namespace mental::rhi
{
static const char* graphicsApiToString(GraphicsApi api)
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

DeviceHandle createDevice(GraphicsApi api)
{
    DeviceHandle device;

    switch (api)
    {
#if defined MENTAL_WITH_VULKAN
        case GraphicsApi::Vulkan:
        {
            vk::DeviceFactory factory;
            device = factory.create();
            if (!device) mental::core::log::fatal("Failed to create Vulkan device");
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

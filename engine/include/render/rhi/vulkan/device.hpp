#pragma once

#include <render/rhi/rhi.hpp>
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <unordered_set>
#include <string>
#include <vector>
#include <render/rhi/vulkan/commandQueue.hpp>

namespace mental::rhi::vk
{
struct DeviceDesc
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t apiVersion;

    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    VkQueue graphicsQueue;
    int graphicsQueueIndex = -1;

    VkDebugUtilsMessengerEXT debugUtilsMessenger;
    VkDebugReportCallbackEXT debugReportCallback;

    std::vector<const char*> instanceExtensions;
    std::vector<const char*> deviceExtensions;
};

struct Context
{
    VkInstance mInstance;
    VkSurfaceKHR mSurface;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;
    VmaAllocator mAllocator;

    VkDebugReportCallbackEXT mDebugReportCallback;
    VkDebugUtilsMessengerEXT mDebugUtilsMessenger;

    VkSurfaceCapabilitiesKHR mCapabilities;
    std::vector<VkSurfaceFormatKHR> mFormats;
    std::vector<VkPresentModeKHR> mPresentModes;

    std::unordered_set<std::string> mInstanceExtensions;
    std::unordered_set<std::string> mDeviceExtensions;

    Context() = default;

    rhi::Result init(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t apiVersion,
        VkDebugReportCallbackEXT debugReportCallback, VkDebugUtilsMessengerEXT debugUtilsMessenger, VkSurfaceCapabilitiesKHR capabilities,
        const std::vector<VkSurfaceFormatKHR>& formats, const std::vector<VkPresentModeKHR>& presentModes,
        const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions);

    void destroy();
};

class Device : public IDevice
{
public:
    static Device& instance();
    rhi::Result init(const DeviceDesc& desc);
    ~Device();

    virtual void waitIdle() override;
    virtual GraphicsApi getGraphicsApi() override;
    virtual rhi::Result createBuffer(BufferDesc desc, BufferHandle& buffer) override;
    virtual ICommandQueue* getGraphicsQueue() override;
    vk::CommandQueue& getVulkanGraphicsQueue();

private:
    Context mContext;
    CommandQueue mGraphicsQueue;
};

}  // namespace mental::rhi::vk

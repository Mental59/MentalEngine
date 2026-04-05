#include <render/rhi/vulkan/device.hpp>
#include <Volk/volk.h>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/allocator.hpp>
#include <array>
#include <memory>
#include <vector>
#include "core/resource.hpp"
#include "core/types.hpp"
#include "render/rhi/rhi.hpp"

namespace mental::rhi::vk
{
core::Result Context::init(VkInstance instance,
  VkSurfaceKHR surface,
  VkPhysicalDevice physicalDevice,
  VkDevice device,
  VkDebugReportCallbackEXT debugReportCallback,
  VkDebugUtilsMessengerEXT debugUtilsMessenger,
  const std::vector<VkSurfaceFormatKHR>& formats,
  const std::vector<VkPresentModeKHR>& presentModes,
  const std::vector<const char*>& instanceExtensions,
  const std::vector<const char*>& deviceExtensions)

{
  mInstance = instance;
  mSurface = surface;
  mPhysicalDevice = physicalDevice;
  mDevice = device;
  mFormats = formats;
  mPresentModes = presentModes;
  mDebugReportCallback = debugReportCallback;
  mDebugUtilsMessenger = debugUtilsMessenger;

  MENTAL_ASSERT_DEBUG(instance != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(surface != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(physicalDevice != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(device != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(instance != VK_NULL_HANDLE);

  for (const char* extensionName : instanceExtensions)
    mInstanceExtensions.insert(extensionName);
  for (const char* extensionName : deviceExtensions)
    mDeviceExtensions.insert(extensionName);

  return core::Result::eSuccess;
}

void Context::destroy()
{
  vkDestroyDevice(mDevice, VK_NULL_HANDLE);
  vkDestroySurfaceKHR(mInstance, mSurface, VK_NULL_HANDLE);
  if (mDebugUtilsMessenger)
    vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, VK_NULL_HANDLE);
  if (mDebugReportCallback)
    vkDestroyDebugReportCallbackEXT(mInstance, mDebugReportCallback, VK_NULL_HANDLE);
  vkDestroyInstance(mInstance, VK_NULL_HANDLE);
}

void Device::waitIdle()
{
  vkDeviceWaitIdle(mContext.mDevice);
}

GraphicsApi Device::getGraphicsApi()
{
  return GraphicsApi::Vulkan;
}

std::unique_ptr<mental::rhi::IShaderModule> mental::rhi::vk::Device::createShaderModule()
{
  return std::make_unique<ShaderModule>();
}

std::unique_ptr<mental::rhi::IResourceLayout> mental::rhi::vk::Device::createResourceLayout()
{
  return std::make_unique<ResourceLayout>();
}

std::unique_ptr<mental::rhi::IResourceSet> mental::rhi::vk::Device::createResourceSet()
{
  return std::make_unique<ResourceSet>();
}

std::unique_ptr<mental::rhi::IPipelineLayout> mental::rhi::vk::Device::createPipelineLayout()
{
  return std::make_unique<PipelineLayout>();
}

std::unique_ptr<mental::rhi::IGraphicsPipeline> mental::rhi::vk::Device::createGraphicsPipeline()
{
  return std::make_unique<GraphicsPipeline>();
}

mental::core::Result mental::rhi::vk::Device::updateResourceSets(const ResourceWriteDesc* writes, uint32_t writeCount)
{
  if (writes == nullptr || writeCount == 0u)
  {
    return core::Result::eSuccess;
  }

  std::vector<VkDescriptorBufferInfo> bufferInfos(writeCount);
  std::vector<VkWriteDescriptorSet> vkWrites(writeCount);

  for (uint32_t writeIndex = 0; writeIndex < writeCount; ++writeIndex)
  {
    const ResourceWriteDesc& write = writes[writeIndex];
    if (write.resourceSet == nullptr || write.buffer.buffer == nullptr)
    {
      MENTAL_ERROR("Resource write requires both a resource set and buffer");
      return core::Result::eOperationFailed;
    }

    const VkDescriptorSet descriptorSet =
      write.resourceSet->getNativeObject(core::resource::ObjectType::eVkDescriptorSet);
    const VkBuffer buffer = write.buffer.buffer->getNativeObject(core::resource::ObjectType::eVkBuffer);
    MENTAL_ASSERT_DEBUG(descriptorSet != VK_NULL_HANDLE);
    MENTAL_ASSERT_DEBUG(buffer != VK_NULL_HANDLE);

    bufferInfos[writeIndex].buffer = buffer;
    bufferInfos[writeIndex].offset = write.buffer.offset;
    bufferInfos[writeIndex].range =
      write.buffer.range == 0u ? write.buffer.buffer->getDesc().byteSize - write.buffer.offset : write.buffer.range;

    vkWrites[writeIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWrites[writeIndex].dstSet = descriptorSet;
    vkWrites[writeIndex].dstBinding = write.binding;
    vkWrites[writeIndex].descriptorCount = 1;
    vkWrites[writeIndex].descriptorType = convertResourceBindingType(write.type);
    vkWrites[writeIndex].pBufferInfo = &bufferInfos[writeIndex];
  }

  vkUpdateDescriptorSets(mContext.mDevice, writeCount, vkWrites.data(), 0, nullptr);
  return core::Result::eSuccess;
}

ICommandQueue* Device::getGraphicsQueue()
{
  return &mGraphicsQueue;
}

ISwapchain* Device::getSwapchain()
{
  return &mSwapchain;
}

Device& Device::instance()
{
  static Device device;
  return device;
}

core::Result Device::init(const DeviceDesc& desc)
{
  if (mIsInit)
  {
    MENTAL_INFO("Trying to initialize an already initialized vk::Device");
    return core::Result::eInitializationFailed;
  }

  core::Result res = mContext.init(desc.instance,
    desc.surface,
    desc.physicalDevice,
    desc.device,
    desc.debugReportCallback,
    desc.debugUtilsMessenger,
    desc.formats,
    desc.presentModes,
    desc.instanceExtensions,
    desc.deviceExtensions);
  if (res != core::Result::eSuccess)
  {
    return core::Result::eInitializationFailed;
  }

  if (!desc.graphicsQueue || desc.graphicsQueueIndex < 0)
  {
    MENTAL_ERROR("Vulkan graphics queue is invalid");
    return core::Result::eInitializationFailed;
  }
  res = mGraphicsQueue.init(desc.graphicsQueue, desc.graphicsQueueIndex);
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to initialize graphics queue, error: {}", core::resultToString(res));
    return core::Result::eInitializationFailed;
  }

  AllocatorDesc allocatorDesc {};
  allocatorDesc.device = desc.device;
  allocatorDesc.physicalDevice = desc.physicalDevice;
  allocatorDesc.instance = desc.instance;
  allocatorDesc.vulkanApiVersion = desc.apiVersion;
  res = initAllocator(allocatorDesc);
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to initialize allocator, error: {}", core::resultToString(res));
    return core::Result::eInitializationFailed;
  }

  res = initResourceDescriptorPool();
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to initialize the shared resource descriptor pool, error: {}", core::resultToString(res));
    return core::Result::eInitializationFailed;
  }

  SwapchainDesc swapchainDesc {};
  swapchainDesc.enableVerticalSync = desc.enableTripleBuffering;
  swapchainDesc.textureCount = desc.enableTripleBuffering ? 3 : 2;
  res = mSwapchain.init(swapchainDesc);
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to create swapchain, error: {}", core::resultToString(res));
    return core::Result::eInitializationFailed;
  }

  MENTAL_INFO("Vulkan device initialized");

  mIsInit = true;
  return core::Result::eSuccess;
}

void Device::destroy()
{
  if (!mIsInit)
  {
    MENTAL_INFO("Trying to destroy uninitialized vk::Device");
    return;
  }

  destroyResourceDescriptorPool();
  destroyAllocator();
  mSwapchain.destroy();
  mGraphicsQueue.destroy();
  mContext.destroy();
  mIsInit = false;

  MENTAL_INFO("Vulkan device destroyed");
}

core::Result Device::initResourceDescriptorPool()
{
  constexpr std::uint32_t kMaxResourceSetCount = 256u;
  constexpr std::uint32_t kUniformBufferDescriptorCount = 512u;
  constexpr std::uint32_t kStorageBufferDescriptorCount = 512u;

  std::array<VkDescriptorPoolSize, 2> poolSizes {
    VkDescriptorPoolSize {
                          .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                          .descriptorCount = kUniformBufferDescriptorCount,
                          },
    VkDescriptorPoolSize {
                          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                          .descriptorCount = kStorageBufferDescriptorCount,
                          },
  };

  VkDescriptorPoolCreateInfo createInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  createInfo.maxSets = kMaxResourceSetCount;
  createInfo.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
  createInfo.pPoolSizes = poolSizes.data();

  const VkResult result = vkCreateDescriptorPool(mContext.mDevice, &createInfo, nullptr, &mResourceDescriptorPool);
  if (result != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to create the shared Vulkan resource descriptor pool, error: {}", vkResultToString(result));
    return core::Result::eInitializationFailed;
  }

  return core::Result::eSuccess;
}

void Device::destroyResourceDescriptorPool()
{
  if (mResourceDescriptorPool == VK_NULL_HANDLE)
  {
    return;
  }

  vkDestroyDescriptorPool(mContext.mDevice, mResourceDescriptorPool, nullptr);
  mResourceDescriptorPool = VK_NULL_HANDLE;
}

bool Device::isValid() const
{
  return mIsInit;
}

core::resource::Object Device::getNativeObject(core::resource::ObjectType objectType)
{
  return nullptr;
}

} // namespace mental::rhi::vk

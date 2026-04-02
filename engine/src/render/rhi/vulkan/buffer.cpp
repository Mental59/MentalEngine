#include <render/rhi/vulkan/buffer.hpp>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/rhi.hpp>
#include <render/rhi/vulkan/device.hpp>
#include "render/rhi/vulkan/allocator.hpp"

mental::core::Result mental::rhi::vk::Buffer::init(const BufferDesc& desc)
{
  if (mIsInit)
  {
    MENTAL_WARN("Trying to initialize an already initialized vk::Buffer");
    return core::Result::eInitializationFailed;
  }

  if (desc.byteSize == 0)
    return core::Result::eInitializationFailed;

  VkBufferUsageFlags usage = 0;
  if (desc.usage & BufferUsageFlagBits::eBufferUsageStorageBit)
    usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (desc.usage & BufferUsageFlagBits::eBufferUsageUniformBit)
    usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if (desc.usage & BufferUsageFlagBits::eBufferUsageTransferSrcBit)
    usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if (desc.usage & BufferUsageFlagBits::eBufferUsageTransferDstBit)
    usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  VkBufferCreateInfo bufferCreateInfo {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCreateInfo.size = desc.byteSize;
  bufferCreateInfo.usage = usage;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo {};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  switch (desc.cpuAccess)
  {
    case BufferCpuAccess::Write:
      allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
      break;
    case BufferCpuAccess::ReadWrite:
      allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
      break;
    case BufferCpuAccess::None:
      break;
  }

  VkBuffer vkBuffer;
  VmaAllocation allocation;
  VkResult createBufferRes =
    vmaCreateBuffer(vk::getAllocator(), &bufferCreateInfo, &allocInfo, &vkBuffer, &allocation, nullptr);

  if (createBufferRes != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to create buffer {}", vkResultToString(createBufferRes));
    return core::Result::eInitializationFailed;
  }

  mDesc = desc;
  mBuffer = vkBuffer;
  mAllocation = allocation;
  mIsMapped = false;
  mIsInit = true;

  MENTAL_INFO("Vulkan buffer initialized");

  return core::Result::eSuccess;
}

void mental::rhi::vk::Buffer::destroy()
{
  if (!mIsInit)
  {
    MENTAL_WARN("Trying to destroy an uninitialized vk::Buffer");
    return;
  }

  vmaDestroyBuffer(vk::getAllocator(), mBuffer, mAllocation);
  mDesc = {};
  mBuffer = VK_NULL_HANDLE;
  mAllocation = nullptr;
  mIsMapped = false;
  mIsInit = false;

  MENTAL_INFO("Vulkan buffer destroyed");
}

bool mental::rhi::vk::Buffer::isValid() const
{
  return mIsInit;
}

mental::core::resource::Object mental::rhi::vk::Buffer::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkBuffer:
      return mBuffer;
    default:
      return nullptr;
  }
}

mental::core::Result mental::rhi::vk::Buffer::map(void** mappedData)
{
  VkResult res = vmaMapMemory(vk::getAllocator(), mAllocation, mappedData);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Vulkan buffer map failed, error: {}", vkResultToString(res));
    return core::Result::eOperationFailed;
  }
  mIsMapped = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::Buffer::unmap()
{
  vmaUnmapMemory(vk::getAllocator(), mAllocation);
  mIsMapped = false;
}

mental::core::Result mental::rhi::vk::Buffer::copy(void* data, uint64_t size, uint64_t offset)
{
  if (!mIsMapped)
  {
    MENTAL_ERROR("Buffer is not mapped");
    return core::Result::eOperationFailed;
  }

  VmaAllocationInfo allocationInfo;
  vmaGetAllocationInfo(vk::getAllocator(), mAllocation, &allocationInfo);
  memcpy((uint8_t*)allocationInfo.pMappedData + offset, data, size);

  return core::Result::eSuccess;
}

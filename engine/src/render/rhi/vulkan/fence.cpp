#include <render/rhi/vulkan/fence.hpp>
#include "core/resource.hpp"
#include "render/rhi/vulkan/constants.hpp"
#include "render/rhi/vulkan/device.hpp"
#include "core/log.hpp"
#include "core/types.hpp"

mental::core::resource::Object mental::rhi::vk::Fence::getNativeObject(mental::core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkFence: return mFence;
    default: return nullptr;
  }
}

mental::core::Result mental::rhi::vk::Fence::init(const mental::rhi::FenceDesc& desc)
{
  if (mIsInit)
  {
    MENTAL_INFO("Trying to initialize an already initialized vk::Fence");
    return core::Result::eInitializationFailed;
  }

  VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  if (desc.createSignaled)
  {
    createInfo.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
  }

  VkResult res = vkCreateFence(vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &mFence);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateFence, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  mIsInit = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::Fence::destroy()
{
  if (!mIsInit)
  {
    MENTAL_INFO("Trying to destroy uninitialized vk::Fence");
    return;
  }

  vkDestroyFence(vk::getDevice().getVirtualDevice(), mFence, nullptr);

  mIsInit = false;
  mFence = VK_NULL_HANDLE;
}

bool mental::rhi::vk::Fence::isValid() const
{
  return mIsInit;
}

mental::core::Result mental::rhi::vk::Fence::wait(uint64_t timeout)
{
  VkResult res = vkWaitForFences(vk::getDevice().getVirtualDevice(), 1, &mFence, VK_TRUE, timeout);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkWaitForFences, error: {}", vkResultToString(res));
    return core::Result::eOperationFailed;
  }
  return core::Result::eSuccess;
}

mental::core::Result mental::rhi::vk::Fence::reset()
{
  VkResult res = vkResetFences(vk::getDevice().getVirtualDevice(), 1, &mFence);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkResetFences, error: {}", vkResultToString(res));
    return core::Result::eOperationFailed;
  }
  return core::Result::eSuccess;
}

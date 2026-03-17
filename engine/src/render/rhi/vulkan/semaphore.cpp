#include <render/rhi/vulkan/semaphore.hpp>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/device.hpp>

mental::core::resource::Object mental::rhi::vk::Semaphore::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkSemaphore: return mSemaphore;
    default: return nullptr;
  }
}

mental::core::Result mental::rhi::vk::Semaphore::init()
{
  if (mIsInit)
  {
    MENTAL_INFO("Trying to initialize an already initialized vk::Semaphore");
    return core::Result::eInitializationFailed;
  }

  VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

  VkResult res = vkCreateSemaphore(vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &mSemaphore);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateSemaphore, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  mIsInit = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::Semaphore::destroy()
{
  if (!mIsInit)
  {
    MENTAL_INFO("Trying to destroy uninitialized vk::Semaphore");
    return;
  }

  vkDestroySemaphore(vk::getDevice().getVirtualDevice(), mSemaphore, nullptr);

  mSemaphore = VK_NULL_HANDLE;
  mIsInit = false;
}

bool mental::rhi::vk::Semaphore::isValid() const
{
  return mIsInit;
}

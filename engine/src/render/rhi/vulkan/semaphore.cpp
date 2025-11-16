#include <render/rhi/vulkan/semaphore.hpp>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/device.hpp>

mental::core::resource::Object mental::rhi::vk::Semaphore::getNativeObject(core::resource::ObjectType objectType)
{
  if (objectType == core::resource::ObjectType::eVkSemaphore)
    return mSemaphore;
  return mSemaphore;
}

mental::core::Result mental::rhi::vk::Semaphore::init()
{
  VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  VkResult res = vkCreateSemaphore(vk::getDevice().getVkDevice(), &createInfo, nullptr, &mSemaphore);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateSemaphore, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }
  return core::Result::eSuccess;
}

void mental::rhi::vk::Semaphore::destroy()
{
  vkDestroySemaphore(vk::getDevice().getVkDevice(), mSemaphore, nullptr);
}

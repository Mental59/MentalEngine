#include <render/rhi/vulkan/commandQueue.hpp>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <array>

mental::core::Result mental::rhi::vk::CommandQueue::init(VkQueue queue, uint32_t index)
{
  mQueue = queue;
  mIndex = index;
  return createCommandPool(index);
}

void mental::rhi::vk::CommandQueue::destroy()
{
  vkDestroyCommandPool(vk::getDevice().getVirtualDevice(), mCommandPool, nullptr);
}

mental::core::Result mental::rhi::vk::CommandQueue::submit(const SubmitInfo& submitInfo)
{
  VkSubmitInfo vkSubmitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
  if (submitInfo.waitSemaphore)
  {
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = submitInfo.waitSemaphore->getNativeObject(core::resource::ObjectType::eVkSemaphore);
  }
  if (submitInfo.signalSemaphore)
  {
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = submitInfo.signalSemaphore->getNativeObject(core::resource::ObjectType::eVkSemaphore);
  }

  MENTAL_ASSERT(submitInfo.cmdListCount <= kMaxSubmitCmdListCount);

  std::array<VkCommandBuffer, kMaxSubmitCmdListCount> commandBuffers{};
  for (uint32_t i = 0; i < submitInfo.cmdListCount; i++)
  {
    commandBuffers[i] = submitInfo.cmdLists[i]->getNativeObject(core::resource::ObjectType::eVkCommandBuffer);
  }
  vkSubmitInfo.commandBufferCount = submitInfo.cmdListCount;
  vkSubmitInfo.pCommandBuffers = commandBuffers.data();

  VkFence fence = VK_NULL_HANDLE;
  if (submitInfo.signalFence)
  {
    fence = submitInfo.signalFence->getNativeObject(core::resource::ObjectType::eVkFence);
  }

  VkResult res = vkQueueSubmit(mQueue, 1, &vkSubmitInfo, fence);
  if (res != VK_SUCCESS)
    return core::Result::eOperationFailed;

  return core::Result::eSuccess;
}

void mental::rhi::vk::CommandQueue::waitIdle()
{
  vkQueueWaitIdle(mQueue);
}

mental::core::resource::Object mental::rhi::vk::CommandQueue::getNativeObject(core::resource::ObjectType objectType)
{
  if (objectType == core::resource::ObjectType::eVkQueue)
    return mQueue;
  if (objectType == core::resource::ObjectType::eVkCommandPool)
    return mCommandPool;
  return nullptr;
}

mental::core::Result mental::rhi::vk::CommandQueue::createCommandPool(uint32_t queueFamilyIndex)
{
  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  createInfo.queueFamilyIndex = queueFamilyIndex;
  VkResult res = vkCreateCommandPool(vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &mCommandPool);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateCommandPool, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }
  return core::Result::eSuccess;
}

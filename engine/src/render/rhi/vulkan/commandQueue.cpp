#include <render/rhi/vulkan/commandQueue.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <core/log.hpp>
#include <array>

mental::rhi::Result mental::rhi::vk::CommandQueue::init(VkQueue queue, uint32_t index)
{
    mQueue = queue;
    mIndex = index;
    return createCommandPool(index);
}

mental::rhi::Result mental::rhi::vk::CommandQueue::submit(const SubmitInfo& info)
{
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    if (info.waitSemaphore)
    {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = info.waitSemaphore->getNativeObject(core::resource::ObjectTypes::vkSemaphore);
    }
    if (info.signalSemaphore)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = info.signalSemaphore->getNativeObject(core::resource::ObjectTypes::vkSemaphore);
    }

    MENTAL_ASSERT(info.cmdListCount <= gMaxSubmitCmdListCount);

    std::array<VkCommandBuffer, gMaxSubmitCmdListCount> commandBuffers{};
    for (uint32_t i = 0; i < info.cmdListCount; i++)
    {
        commandBuffers[i] = info.cmdLists[i]->getNativeObject(core::resource::ObjectTypes::vkCommandBuffer);
    }
    submitInfo.commandBufferCount = info.cmdListCount;
    submitInfo.pCommandBuffers = commandBuffers.data();

    VkFence fence = VK_NULL_HANDLE;
    if (info.signalFence)
    {
        fence = info.signalFence->getNativeObject(core::resource::ObjectTypes::vkFence);
    }

    VkResult res = vkQueueSubmit(mQueue, 1, &submitInfo, fence);
    if (res != VK_SUCCESS) return rhi::Result::eQueueSubmitFailed;

    return rhi::Result::eSuccess;
}

void mental::rhi::vk::CommandQueue::waitIdle()
{
    vkQueueWaitIdle(mQueue);
}

mental::core::resource::Object mental::rhi::vk::CommandQueue::getNativeObject(core::resource::ObjectType objectType)
{
    if (objectType == core::resource::ObjectTypes::vkQueue) return mQueue;
    if (objectType == core::resource::ObjectTypes::vkCommandPool) return mCommandPool;
    return nullptr;
}

mental::rhi::Result mental::rhi::vk::CommandQueue::createCommandPool(uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamilyIndex;
    VkResult res = vkCreateCommandPool(vk::Device::instance().getVkDevice(), &createInfo, nullptr, &mCommandPool);
    if (res != VK_SUCCESS)
    {
        MENTAL_ERROR("Failed to call vkCreateCommandPool, error: {}", vkResultToString(res));
        return rhi::Result::eCommandQueueInitializationFailed;
    }
    return rhi::Result::eSuccess;
}

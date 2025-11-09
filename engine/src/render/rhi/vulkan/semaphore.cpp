#include <render/rhi/vulkan/semaphore.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>

mental::core::resource::Object mental::rhi::vk::Semaphore::getNativeObject(core::resource::ObjectType objectType)
{
    if (objectType == core::resource::ObjectType::eVkSemaphore) return mSemaphore;
    return mSemaphore;
}

mental::rhi::Result mental::rhi::vk::Semaphore::init()
{
    VkSemaphoreCreateInfo createInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkResult res = vkCreateSemaphore(vk::Device::instance().getVkDevice(), &createInfo, nullptr, &mSemaphore);
    if (res != VK_SUCCESS)
    {
        MENTAL_ERROR("Failed to call vkCreateSemaphore, error: {}", vkResultToString(res));
        return rhi::Result::eSemaphoreInitializationFailed;
    }
    return rhi::Result::eSuccess;
}

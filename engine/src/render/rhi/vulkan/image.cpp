#include <render/rhi/vulkan/image.hpp>
#include <render/rhi/vulkan/device.hpp>

mental::core::resource::Object mental::rhi::vk::Image::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkImage: return mImage;
    default: return nullptr;
  }
}

void mental::rhi::vk::Image::destroy()
{
  VkDevice device = vk::getDevice().getVirtualDevice();

  if (mShouldDestroyImage)
  {
    vkDestroyImage(device, mImage, nullptr);
  }
}

void mental::rhi::vk::Image::initSwapchainImage(const mental::rhi::vk::SwapchainImageDesc& desc)
{
  mShouldDestroyImage = false;
  mImage = desc.image;
  mExtent = desc.extent;
}

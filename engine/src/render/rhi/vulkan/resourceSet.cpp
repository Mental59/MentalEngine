#include <render/rhi/vulkan/resourceSet.hpp>

#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/device.hpp>

#include <vector>

mental::core::Result mental::rhi::vk::ResourceLayout::init(const ResourceLayoutDesc& desc)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized vk::ResourceLayout");
    return core::Result::eInitializationFailed;
  }

  if (desc.bindings == nullptr || desc.bindingCount == 0u)
  {
    MENTAL_ERROR("Resource layout init requires at least one binding");
    return core::Result::eInitializationFailed;
  }

  mBindings.assign(desc.bindings, desc.bindings + desc.bindingCount);

  std::vector<VkDescriptorSetLayoutBinding> vkBindings {};
  vkBindings.reserve(mBindings.size());
  for (const ResourceBindingDesc& binding : mBindings)
  {
    VkDescriptorSetLayoutBinding vkBinding {};
    vkBinding.binding = binding.binding;
    vkBinding.descriptorType = convertResourceBindingType(binding.type);
    vkBinding.descriptorCount = binding.descriptorCount;
    vkBinding.stageFlags = convertShaderStageFlags(binding.stageFlags);
    vkBindings.push_back(vkBinding);
  }

  VkDescriptorSetLayoutCreateInfo createInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  createInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
  createInfo.pBindings = vkBindings.data();

  const VkResult result =
    vkCreateDescriptorSetLayout(vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &mDescriptorSetLayout);
  if (result != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to create Vulkan resource layout, error: {}", vkResultToString(result));
    return core::Result::eInitializationFailed;
  }

  mDesc.bindings = mBindings.data();
  mDesc.bindingCount = static_cast<uint32_t>(mBindings.size());
  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::ResourceLayout::destroy()
{
  if (!mIsInitialized)
  {
    return;
  }

  vkDestroyDescriptorSetLayout(vk::getDevice().getVirtualDevice(), mDescriptorSetLayout, nullptr);
  mBindings.clear();
  mDesc = {};
  mDescriptorSetLayout = VK_NULL_HANDLE;
  mIsInitialized = false;
}

bool mental::rhi::vk::ResourceLayout::isValid() const
{
  return mIsInitialized;
}

mental::core::resource::Object mental::rhi::vk::ResourceLayout::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkDescriptorSetLayout:
      return mDescriptorSetLayout;
    default:
      return nullptr;
  }
}

const mental::rhi::ResourceLayoutDesc& mental::rhi::vk::ResourceLayout::getDesc() const
{
  return mDesc;
}

mental::core::Result mental::rhi::vk::ResourceSet::init(const ResourceSetDesc& desc)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized vk::ResourceSet");
    return core::Result::eInitializationFailed;
  }

  if (desc.resourceLayout == nullptr)
  {
    MENTAL_ERROR("Resource set init requires a resource layout");
    return core::Result::eInitializationFailed;
  }

  const VkDescriptorSetLayout vkDescriptorSetLayout =
    desc.resourceLayout->getNativeObject(core::resource::ObjectType::eVkDescriptorSetLayout);
  const VkDescriptorPool vkDescriptorPool = vk::getDevice().getResourceDescriptorPool();
  MENTAL_ASSERT_DEBUG(vkDescriptorPool != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(vkDescriptorSetLayout != VK_NULL_HANDLE);

  VkDescriptorSetAllocateInfo allocInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocInfo.descriptorPool = vkDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &vkDescriptorSetLayout;

  const VkResult result = vkAllocateDescriptorSets(vk::getDevice().getVirtualDevice(), &allocInfo, &mDescriptorSet);
  if (result != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to allocate Vulkan resource set, error: {}", vkResultToString(result));
    return core::Result::eInitializationFailed;
  }

  mResourceLayout = desc.resourceLayout;
  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::ResourceSet::destroy()
{
  if (!mIsInitialized)
  {
    return;
  }

  mResourceLayout = nullptr;
  mDescriptorSet = VK_NULL_HANDLE;
  mIsInitialized = false;
}

bool mental::rhi::vk::ResourceSet::isValid() const
{
  return mIsInitialized;
}

mental::core::resource::Object mental::rhi::vk::ResourceSet::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkDescriptorSet:
      return mDescriptorSet;
    default:
      return nullptr;
  }
}

mental::rhi::IResourceLayout* mental::rhi::vk::ResourceSet::getResourceLayout() const
{
  return mResourceLayout;
}

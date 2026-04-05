#include <render/rhi/vulkan/descriptorSet.hpp>

#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/device.hpp>

#include <vector>

mental::core::Result mental::rhi::vk::DescriptorSetLayout::init(const DescriptorSetLayoutDesc& desc)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized vk::DescriptorSetLayout");
    return core::Result::eInitializationFailed;
  }

  if (desc.bindings == nullptr || desc.bindingCount == 0u)
  {
    MENTAL_ERROR("Descriptor set layout init requires at least one binding");
    return core::Result::eInitializationFailed;
  }

  mBindings.assign(desc.bindings, desc.bindings + desc.bindingCount);

  std::vector<VkDescriptorSetLayoutBinding> vkBindings {};
  vkBindings.reserve(mBindings.size());
  for (const DescriptorBindingDesc& binding : mBindings)
  {
    VkDescriptorSetLayoutBinding vkBinding {};
    vkBinding.binding = binding.binding;
    vkBinding.descriptorType = convertDescriptorType(binding.type);
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
    MENTAL_ERROR("Failed to create Vulkan descriptor set layout, error: {}", vkResultToString(result));
    return core::Result::eInitializationFailed;
  }

  mDesc.bindings = mBindings.data();
  mDesc.bindingCount = static_cast<uint32_t>(mBindings.size());
  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::DescriptorSetLayout::destroy()
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

bool mental::rhi::vk::DescriptorSetLayout::isValid() const
{
  return mIsInitialized;
}

mental::core::resource::Object mental::rhi::vk::DescriptorSetLayout::getNativeObject(
  core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkDescriptorSetLayout:
      return mDescriptorSetLayout;
    default:
      return nullptr;
  }
}

const mental::rhi::DescriptorSetLayoutDesc& mental::rhi::vk::DescriptorSetLayout::getDesc() const
{
  return mDesc;
}

mental::core::Result mental::rhi::vk::DescriptorPool::init(const DescriptorPoolDesc& desc)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized vk::DescriptorPool");
    return core::Result::eInitializationFailed;
  }

  if (desc.poolSizes == nullptr || desc.poolSizeCount == 0u || desc.maxSetCount == 0u)
  {
    MENTAL_ERROR("Descriptor pool init requires pool sizes and a non-zero set count");
    return core::Result::eInitializationFailed;
  }

  mPoolSizes.assign(desc.poolSizes, desc.poolSizes + desc.poolSizeCount);

  std::vector<VkDescriptorPoolSize> vkPoolSizes {};
  vkPoolSizes.reserve(mPoolSizes.size());
  for (const DescriptorPoolSizeDesc& poolSize : mPoolSizes)
  {
    VkDescriptorPoolSize vkPoolSize {};
    vkPoolSize.type = convertDescriptorType(poolSize.type);
    vkPoolSize.descriptorCount = poolSize.descriptorCount;
    vkPoolSizes.push_back(vkPoolSize);
  }

  VkDescriptorPoolCreateInfo createInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  createInfo.maxSets = desc.maxSetCount;
  createInfo.poolSizeCount = static_cast<uint32_t>(vkPoolSizes.size());
  createInfo.pPoolSizes = vkPoolSizes.data();

  const VkResult result =
    vkCreateDescriptorPool(vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &mDescriptorPool);
  if (result != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to create Vulkan descriptor pool, error: {}", vkResultToString(result));
    return core::Result::eInitializationFailed;
  }

  mDesc.poolSizes = mPoolSizes.data();
  mDesc.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size());
  mDesc.maxSetCount = desc.maxSetCount;
  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::DescriptorPool::destroy()
{
  if (!mIsInitialized)
  {
    return;
  }

  vkDestroyDescriptorPool(vk::getDevice().getVirtualDevice(), mDescriptorPool, nullptr);
  mPoolSizes.clear();
  mDesc = {};
  mDescriptorPool = VK_NULL_HANDLE;
  mIsInitialized = false;
}

bool mental::rhi::vk::DescriptorPool::isValid() const
{
  return mIsInitialized;
}

mental::core::resource::Object mental::rhi::vk::DescriptorPool::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkDescriptorPool:
      return mDescriptorPool;
    default:
      return nullptr;
  }
}

const mental::rhi::DescriptorPoolDesc& mental::rhi::vk::DescriptorPool::getDesc() const
{
  return mDesc;
}

mental::core::Result mental::rhi::vk::DescriptorSet::init(const DescriptorSetDesc& desc)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized vk::DescriptorSet");
    return core::Result::eInitializationFailed;
  }

  if (desc.descriptorPool == nullptr || desc.descriptorSetLayout == nullptr)
  {
    MENTAL_ERROR("Descriptor set init requires a pool and layout");
    return core::Result::eInitializationFailed;
  }

  const VkDescriptorPool vkDescriptorPool =
    desc.descriptorPool->getNativeObject(core::resource::ObjectType::eVkDescriptorPool);
  const VkDescriptorSetLayout vkDescriptorSetLayout =
    desc.descriptorSetLayout->getNativeObject(core::resource::ObjectType::eVkDescriptorSetLayout);
  MENTAL_ASSERT_DEBUG(vkDescriptorPool != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(vkDescriptorSetLayout != VK_NULL_HANDLE);

  VkDescriptorSetAllocateInfo allocInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocInfo.descriptorPool = vkDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &vkDescriptorSetLayout;

  const VkResult result = vkAllocateDescriptorSets(vk::getDevice().getVirtualDevice(), &allocInfo, &mDescriptorSet);
  if (result != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to allocate Vulkan descriptor set, error: {}", vkResultToString(result));
    return core::Result::eInitializationFailed;
  }

  mDescriptorPool = desc.descriptorPool;
  mDescriptorSetLayout = desc.descriptorSetLayout;
  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::DescriptorSet::destroy()
{
  if (!mIsInitialized)
  {
    return;
  }

  mDescriptorPool = nullptr;
  mDescriptorSetLayout = nullptr;
  mDescriptorSet = VK_NULL_HANDLE;
  mIsInitialized = false;
}

bool mental::rhi::vk::DescriptorSet::isValid() const
{
  return mIsInitialized;
}

mental::core::resource::Object mental::rhi::vk::DescriptorSet::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkDescriptorSet:
      return mDescriptorSet;
    default:
      return nullptr;
  }
}

mental::rhi::IDescriptorSetLayout* mental::rhi::vk::DescriptorSet::getDescriptorSetLayout() const
{
  return mDescriptorSetLayout;
}

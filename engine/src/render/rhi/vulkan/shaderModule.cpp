#include <render/rhi/vulkan/shaderModule.hpp>

#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/device.hpp>

mental::core::Result mental::rhi::vk::ShaderModule::init(const ShaderModuleDesc& desc)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized vk::ShaderModule");
    return core::Result::eInitializationFailed;
  }

  if (desc.spirvCode == nullptr || desc.wordCount == 0u || desc.entryPointName.empty())
  {
    MENTAL_ERROR("Shader module init requires SPIR-V code and an entry point name");
    return core::Result::eInitializationFailed;
  }

  VkShaderModuleCreateInfo createInfo {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  createInfo.codeSize = static_cast<std::size_t>(desc.wordCount * sizeof(uint32_t));
  createInfo.pCode = desc.spirvCode;

  const VkResult result =
    vkCreateShaderModule(vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &mShaderModule);
  if (result != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to create Vulkan shader module, error: {}", vkResultToString(result));
    return core::Result::eInitializationFailed;
  }

  mSpirvWords.assign(desc.spirvCode, desc.spirvCode + desc.wordCount);
  mDesc = desc;
  mDesc.spirvCode = mSpirvWords.data();
  mDesc.wordCount = static_cast<uint64_t>(mSpirvWords.size());
  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::ShaderModule::destroy()
{
  if (!mIsInitialized)
  {
    MENTAL_WARN("Trying to destroy an uninitialized vk::ShaderModule");
    return;
  }

  vkDestroyShaderModule(vk::getDevice().getVirtualDevice(), mShaderModule, nullptr);
  mSpirvWords.clear();
  mDesc = {};
  mShaderModule = VK_NULL_HANDLE;
  mIsInitialized = false;
}

bool mental::rhi::vk::ShaderModule::isValid() const
{
  return mIsInitialized;
}

mental::core::resource::Object mental::rhi::vk::ShaderModule::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkShaderModule:
      return mShaderModule;
    default:
      return nullptr;
  }
}

const mental::rhi::ShaderModuleDesc& mental::rhi::vk::ShaderModule::getDesc() const
{
  return mDesc;
}

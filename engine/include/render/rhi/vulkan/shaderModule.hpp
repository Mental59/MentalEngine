#pragma once

#include <render/rhi/rhi.hpp>

#include <vector>

#include <volk.h>

namespace mental::rhi::vk
{
class ShaderModule : public IShaderModule
{
 public:
  ShaderModule() = default;

  virtual core::Result init(const ShaderModuleDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual const ShaderModuleDesc& getDesc() const override;

 private:
  std::vector<uint32_t> mSpirvWords {};
  ShaderModuleDesc mDesc {};
  VkShaderModule mShaderModule = VK_NULL_HANDLE;
  bool mIsInitialized = false;
};
} // namespace mental::rhi::vk

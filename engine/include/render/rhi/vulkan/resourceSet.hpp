#pragma once

#include <render/rhi/rhi.hpp>

#include <vector>

#include <volk.h>

namespace mental::rhi::vk
{
class ResourceLayout : public IResourceLayout
{
 public:
  ResourceLayout() = default;

  virtual core::Result init(const ResourceLayoutDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual const ResourceLayoutDesc& getDesc() const override;

 private:
  std::vector<ResourceBindingDesc> mBindings {};
  ResourceLayoutDesc mDesc {};
  VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
  bool mIsInitialized = false;
};

class ResourceSet : public IResourceSet
{
 public:
  ResourceSet() = default;

  virtual core::Result init(const ResourceSetDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual IResourceLayout* getResourceLayout() const override;

 private:
  IResourceLayout* mResourceLayout = nullptr;
  VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
  bool mIsInitialized = false;
};
} // namespace mental::rhi::vk

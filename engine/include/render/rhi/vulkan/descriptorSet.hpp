#pragma once

#include <render/rhi/rhi.hpp>

#include <vector>

#include <volk/volk.h>

namespace mental::rhi::vk
{
class DescriptorSetLayout : public IDescriptorSetLayout
{
 public:
  DescriptorSetLayout() = default;

  virtual core::Result init(const DescriptorSetLayoutDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual const DescriptorSetLayoutDesc& getDesc() const override;

 private:
  std::vector<DescriptorBindingDesc> mBindings {};
  DescriptorSetLayoutDesc mDesc {};
  VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
  bool mIsInitialized = false;
};

class DescriptorPool : public IDescriptorPool
{
 public:
  DescriptorPool() = default;

  virtual core::Result init(const DescriptorPoolDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual const DescriptorPoolDesc& getDesc() const override;

 private:
  std::vector<DescriptorPoolSizeDesc> mPoolSizes {};
  DescriptorPoolDesc mDesc {};
  VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
  bool mIsInitialized = false;
};

class DescriptorSet : public IDescriptorSet
{
 public:
  DescriptorSet() = default;

  virtual core::Result init(const DescriptorSetDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual IDescriptorSetLayout* getDescriptorSetLayout() const override;

 private:
  IDescriptorPool* mDescriptorPool = nullptr;
  IDescriptorSetLayout* mDescriptorSetLayout = nullptr;
  VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
  bool mIsInitialized = false;
};
} // namespace mental::rhi::vk

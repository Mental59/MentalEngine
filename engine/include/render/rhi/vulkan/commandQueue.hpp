#pragma once

#include <volk.h>

#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
class CommandQueue : public rhi::ICommandQueue
{
 public:
  core::Result init(VkQueue queue, uint32_t index);
  virtual void destroy() override;

  virtual bool isValid() const override;

  virtual core::Result submit(const SubmitInfo& submitInfo) override;
  virtual void waitIdle() override;

  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

  inline uint32_t getIndex() const
  {
    return mIndex;
  }

 private:
  core::Result createCommandPool(uint32_t queueFamilyIndex);

  bool mIsInit = false;
  VkQueue mQueue;
  VkCommandPool mCommandPool;
  uint32_t mIndex;
};
} // namespace mental::rhi::vk

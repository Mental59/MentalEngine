#pragma once
#include <volk/volk.h>

#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
class Semaphore : public ISemaphore
{
 public:
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

  virtual core::Result init() override;
  virtual void destroy() override;

  virtual bool isValid() const override;

 private:
  bool mIsInit = false;
  VkSemaphore mSemaphore;
};
} // namespace mental::rhi::vk

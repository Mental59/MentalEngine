#pragma once
#include <volk/volk.h>

#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  class Semaphore : public ISemaphore
  {
   public:
    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
    core::Result init();
    virtual void destroy() override;

   private:
    VkSemaphore mSemaphore;
  };
}  // namespace mental::rhi::vk

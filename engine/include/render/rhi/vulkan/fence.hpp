#pragma once
#include <volk/volk.h>
#include <render/rhi/rhi.hpp>
#include <cstdint>
#include "core/types.hpp"

namespace mental::rhi::vk
{
  class Fence : public IFence
  {
   public:
    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

    virtual core::Result init(const FenceDesc& desc) override;
    virtual void destroy() override;

    virtual bool isValid() const override;

    virtual core::Result wait(uint64_t timeout = UINT64_MAX) override;
    virtual core::Result reset() override;

   private:
    bool mIsInit;
    VkFence mFence;
  };
}  // namespace mental::rhi::vk

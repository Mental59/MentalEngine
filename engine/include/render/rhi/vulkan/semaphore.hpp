#pragma once
#include <render/rhi/rhi.hpp>
#include <volk/volk.h>

namespace mental::rhi::vk
{
class Semaphore : public ISemaphore
{
public:
    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
    rhi::Result init();

private:
    VkSemaphore mSemaphore;
};
}  // namespace mental::rhi::vk
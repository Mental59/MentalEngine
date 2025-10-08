#pragma once

#include <render/rhi/rhi.hpp>
#include <core/memory.hpp>

namespace mental::rhi::vk
{
class Buffer : public core::memory::RefCounter<IBuffer>
{
public:
    explicit Buffer(const BufferDesc& desc);
    virtual ~Buffer() override;

    virtual const BufferDesc& getDesc() const override { return mDesc; };

private:
    BufferDesc mDesc;
};
}  // namespace mental::rhi::vk

#pragma once

#include <vma/vk_mem_alloc.h>
#include <volk.h>

#include <core/resource.hpp>
#include <render/rhi/rhi.hpp>
#include <core/types.hpp>

namespace mental::rhi::vk
{
class Buffer : public IBuffer
{
 public:
  Buffer() = default;

  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual core::Result init(const rhi::BufferDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;

  inline virtual const BufferDesc& getDesc() const override
  {
    return mDesc;
  };
  virtual core::Result map(void** mappedData) override;
  virtual void unmap() override;
  virtual core::Result copy(void* data, uint64_t size, uint64_t offset = 0) override;

 private:
  BufferDesc mDesc = {};
  VkBuffer mBuffer = VK_NULL_HANDLE;
  VmaAllocation mAllocation = nullptr;
  bool mIsMapped = false;
  bool mIsInit = false;
};
} // namespace mental::rhi::vk

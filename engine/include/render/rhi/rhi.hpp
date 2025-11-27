#pragma once

#include <core/resource.hpp>
#include <core/types.hpp>
#include <cstddef>

namespace mental::platform
{
  class IWindow;
}

namespace mental::rhi
{
  class IBuffer;
  class ISemaphore;
  class IFence;
  class ICommandList;
  class ICommandQueue;
  class IDevice;
  class ISwapchain;

  enum class GraphicsApi : uint8_t
  {
    Vulkan
  };

  const char* graphicsApiToString(GraphicsApi api);

  enum BufferUsageFlagBits : uint32_t
  {
    eStorageBuffer = 1,
    eUniformBuffer = 2,
    eTransferSrc = 4,
    eTransferDst = 8
  };
  using BufferUsageFlags = uint32_t;

  enum class BufferCpuAccess : uint8_t
  {
    None = 0,
    Write,
    ReadWrite
  };

  struct BufferDesc
  {
    BufferUsageFlags usage = 0;
    BufferCpuAccess cpuAccess = BufferCpuAccess::None;
    uint64_t byteSize = 0;
  };

  class IBuffer : public core::resource::IResource
  {
   public:
    virtual core::Result init(const BufferDesc& desc) = 0;
    virtual const BufferDesc& getDesc() const = 0;
    virtual core::Result map(void** mappedData) = 0;
    virtual core::Result unmap() = 0;
    virtual core::Result copy(void* data, uint64_t size, uint64_t offset = 0) = 0;
  };

  class ISemaphore : public core::resource::IResource
  {
   public:
    virtual core::Result init() = 0;
  };

  struct FenceDesc
  {
    bool createSignaled;
  };

  class IFence : public core::resource::IResource
  {
   public:
    virtual core::Result init(const FenceDesc& desc) = 0;
    virtual core::Result wait(uint64_t timeout = UINT64_MAX) = 0;
    virtual core::Result reset() = 0;
  };

  enum class PipelineStage : uint8_t
  {
    eColorAttachmentOutput = 0
  };

  struct CommandListDesc
  {
    ICommandQueue* commandQueue;
  };
  struct CommandListBegindDesc
  {
    bool isOneTimeSubmit;
  };
  class ICommandList : public core::resource::IResource
  {
   public:
    virtual core::Result init(const CommandListDesc& desc) = 0;
    virtual core::Result begin(const CommandListBegindDesc& desc) = 0;
    virtual core::Result end() = 0;
    virtual core::Result
    copyBuffer(IBuffer* srcBuffer, size_t srcOffset, IBuffer* dstBuffer, size_t dstOffset, size_t size) = 0;
  };

  constexpr uint32_t kMaxSubmitCmdListCount = 8;
  class ICommandQueue : public core::resource::IResource
  {
   public:
    struct SubmitInfo
    {
      ICommandList* const* cmdLists;
      uint32_t cmdListCount;
      ISemaphore* const waitSemaphore;    // Semaphore to wait on before execution
      PipelineStage waitStage;            // Stage at which to wait
      ISemaphore* const signalSemaphore;  // Semaphore to signal after execution
      IFence* const signalFence;          // Fence to signal after execution
    };

    virtual core::Result submit(const SubmitInfo& info) = 0;

    virtual void waitIdle() = 0;
  };

  struct SwapchainDesc
  {
    uint32_t imageCount;
    bool enableVerticalSync;
  };
  class ISwapchain : public core::resource::IResource
  {
   public:
    virtual core::Result init(const SwapchainDesc& desc) = 0;
  };

  class IDevice : public core::resource::IResource
  {
   public:
    virtual void waitIdle() = 0;
    virtual GraphicsApi getGraphicsApi() = 0;

    virtual ICommandQueue* getGraphicsQueue() = 0;
    virtual ISwapchain* getSwapchain() = 0;
  };

  void initDevice(GraphicsApi api, mental::platform::IWindow* window);
  IDevice& getDevice();
  void destroyDevice();

}  // namespace mental::rhi

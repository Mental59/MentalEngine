#pragma once

#include <core/resource.hpp>
#include <core/types.hpp>
#include <cstddef>
#include <optional>

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
  class IImage;
  class IImageView;

  enum class GraphicsApi : uint8_t
  {
    Vulkan
  };

  const char* graphicsApiToString(GraphicsApi api);

  enum BufferUsageFlagBits : uint32_t
  {
    eBufferUsageStorageBit = 1,
    eBufferUsageUniformBit = 2,
    eBufferUsageTransferSrcBit = 4,
    eBufferUsageTransferDstBit = 8
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
    virtual core::Result
    acquireNextImage(uint64_t timeout, ISemaphore* signalSemaphore, IFence* signalFence, uint32_t& imageIndex) = 0;
    virtual uint32_t getImageCount() const = 0;
    virtual IImage* getImage(uint32_t index) = 0;
  };

  enum class ImageFormat : uint8_t
  {
    eRGBA32_SRGB = 0,
    eBGRA32_SRGB,
    eRGBA32_UNORM,
    eBGRA32_UNORM,
    eD32_SFLOAT,
    eD32_SFLOAT_S8_UINT,
    eD24_UNORM_S8_UINT
  };
  enum class ImageLayout : uint8_t
  {
    eUndefined = 0,
    ePresent,
    eColorAttachment,
    eDepthStencilAttachment,
    eTransferSrc,
    eTransferDst,
    eShaderReadOnly
  };
  enum class ImageTiling : uint8_t
  {
    eOptimal = 0,
    eLinear
  };
  enum ImageUsageFlagBits : uint32_t
  {
    eImageUsageTransferSrcBit = 1,
    eImageUsageTransferDstBit = 2,
    eImageUsageSampledBit = 4,
    eImageUsageStorageBit = 8,
    eImageUsageColorAttachmentBit = 16,
    eImageUsageDepthStencilAttachmentBit = 32
  };
  using ImageUsageFlags = uint32_t;

  struct ImageExtent
  {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
  };
  struct ImageDesc
  {
    ImageFormat format;
    ImageLayout layout;
    ImageTiling tiling;
    ImageUsageFlags usage;
    ImageExtent extent;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    bool cubeCompatible;
  };
  class IImage : public core::resource::IResource
  {
   public:
    virtual core::Result init(const ImageDesc& desc) = 0;
    virtual const ImageDesc& getDesc() const = 0;
  };

  enum class ImageViewType : uint8_t
  {
    eTexture = 0,
    eDepthMap,
    eDepthStencilMap,
    eCubeMap
  };

  enum ImageViewAspectFlagBits : uint8_t
  {
    eImageViewColorAspectBit = 1,
    eImageViewDepthAspectBit = 2,
    eImageViewStencilAspectBit = 4
  };
  using ImageViewAspectFlags = uint8_t;

  struct ImageViewDesc
  {
    IImage* image;
    std::optional<ImageFormat> format;
    ImageViewType type;
  };
  class IImageView : public core::resource::IResource
  {
   public:
    virtual core::Result init(const ImageViewDesc& desc) = 0;
    virtual const ImageViewDesc& getDesc() const = 0;
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

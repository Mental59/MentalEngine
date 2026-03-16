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
  class ITexture;
  class ITextureView;

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
  struct TextureOffset3D
  {
    int32_t x;
    int32_t y;
    int32_t z;
  };
  struct ClearBufferValue
  {
    float color[4];
    float depth;
    uint32_t stencil;
  };
  struct RenderArea
  {
    uint32_t width;
    uint32_t height;
  };
  struct CommandListBeginRenderingInfo
  {
    ITextureView* swapchainImageView;
    ClearBufferValue clearValue;
    RenderArea renderArea;
  };

  class ICommandList : public core::resource::IResource
  {
   public:
    virtual core::Result init(const CommandListDesc& desc) = 0;

    virtual core::Result begin(const CommandListBegindDesc& desc) = 0;
    virtual core::Result end() = 0;

    virtual core::Result
    copyBuffer(IBuffer* srcBuffer, size_t srcOffset, IBuffer* dstBuffer, size_t dstOffset, size_t size) = 0;

    virtual core::Result copyBufferToImage(
        IBuffer* buffer,
        size_t bufferOffset,
        ITexture* texture,
        uint32_t mipLevel,
        const TextureOffset3D& textureOffset) = 0;

    virtual void beginRendering(CommandListBeginRenderingInfo& info) = 0;
    virtual void endRendering() = 0;
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
    uint32_t textureCount;
    bool enableVerticalSync;
  };
  class ISwapchain : public core::resource::IResource
  {
   public:
    virtual core::Result init(const SwapchainDesc& desc) = 0;
    virtual core::Result
    acquireNextTexture(uint64_t timeout, ISemaphore* signalSemaphore, IFence* signalFence, uint32_t& textureIndex) = 0;
    virtual uint32_t getTextureCount() const = 0;
    virtual ITexture* getTexture(uint32_t index) = 0;
    virtual ITextureView* getTextureView(uint32_t index) = 0;
  };

  enum class TextureFormat : uint8_t
  {
    eRGBA32_SRGB = 0,
    eBGRA32_SRGB,
    eRGBA32_UNORM,
    eBGRA32_UNORM,
    eD32_SFLOAT,
    eD32_SFLOAT_S8_UINT,
    eD24_UNORM_S8_UINT
  };
  enum class TextureLayout : uint8_t
  {
    eUndefined = 0,
    ePresent,
    eColorAttachment,
    eDepthStencilAttachment,
    eTransferSrc,
    eTransferDst,
    eShaderReadOnly
  };
  enum class TextureTiling : uint8_t
  {
    eOptimal = 0,
    eLinear
  };
  enum TextureUsageFlagBits : uint32_t
  {
    eTextureUsageTransferSrcBit = 1,
    eTextureUsageTransferDstBit = 2,
    eTextureUsageSampledBit = 4,
    eTextureUsageStorageBit = 8,
    eTextureUsageColorAttachmentBit = 16,
    eTextureUsageDepthStencilAttachmentBit = 32
  };
  using TextureUsageFlags = uint32_t;

  struct TextureExtent
  {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
  };
  struct TextureDesc
  {
    TextureFormat format;
    TextureLayout layout;
    TextureTiling tiling;
    TextureUsageFlags usage;
    TextureExtent extent;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    bool cubeCompatible;
  };
  class ITexture : public core::resource::IResource
  {
   public:
    virtual core::Result init(const TextureDesc& desc) = 0;
    virtual const TextureDesc& getDesc() const = 0;
  };

  enum class TextureType : uint8_t
  {
    eTexture2D = 0,
    eDepthMap,
    eDepthStencilMap,
    eCubeMap
  };

  struct TextureViewDesc
  {
    ITexture* texture;
    std::optional<TextureFormat> format;
    TextureType type;
  };
  class ITextureView : public core::resource::IResource
  {
   public:
    virtual core::Result init(const TextureViewDesc& desc) = 0;
    virtual const TextureViewDesc& getDesc() const = 0;
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

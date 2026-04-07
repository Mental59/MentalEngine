#pragma once

#include <core/resource.hpp>
#include <core/types.hpp>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#if defined MENTAL_WITH_VULKAN
#include <volk.h>
#endif

namespace mental::rhi
{
class IBuffer;
class ISemaphore;
class IFence;
class ICommandList;
class ICommandQueue;
class IDevice;
class IResourceSet;
class IResourceLayout;
class IShaderModule;
class IGraphicsPipeline;
class ISwapchain;
class ITexture;
class ITextureView;
enum class TextureLayout : uint8_t;

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
  virtual void unmap() = 0;
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

enum class PipelineStage : uint8_t
{
  eColorAttachmentOutput = 0
};

enum class ShaderStage : uint8_t
{
  eVertex = 0,
  eFragment
};

enum ShaderStageFlagBits : uint32_t
{
  eShaderStageVertexBit = 1,
  eShaderStageFragmentBit = 2,
};
using ShaderStageFlags = uint32_t;

struct ShaderModuleDesc
{
  ShaderStage stage = ShaderStage::eVertex;
  const uint32_t* spirvCode = nullptr;
  uint64_t wordCount = 0;
  std::string entryPointName {};
  std::string debugName {};
};

class IShaderModule : public core::resource::IResource
{
 public:
  virtual core::Result init(const ShaderModuleDesc& desc) = 0;
  virtual const ShaderModuleDesc& getDesc() const = 0;
};

enum class ResourceBindingType : uint8_t
{
  eUniformBuffer = 0,
  eStorageBuffer,
};

struct ResourceBindingDesc
{
  uint32_t binding = 0;
  ResourceBindingType type = ResourceBindingType::eUniformBuffer;
  uint32_t descriptorCount = 1;
  ShaderStageFlags stageFlags = 0;
};

struct ResourceLayoutDesc
{
  const ResourceBindingDesc* bindings = nullptr;
  uint32_t bindingCount = 0;
};

class IResourceLayout : public core::resource::IResource
{
 public:
  virtual core::Result init(const ResourceLayoutDesc& desc) = 0;
  virtual const ResourceLayoutDesc& getDesc() const = 0;
};

struct ResourceSetDesc
{
  IGraphicsPipeline* graphicsPipeline = nullptr;
  uint32_t resourceSetIndex = 0;
};

class IResourceSet : public core::resource::IResource
{
 public:
  virtual core::Result init(const ResourceSetDesc& desc) = 0;
  virtual IResourceLayout* getResourceLayout() const = 0;
};

struct BufferResourceWrite
{
  IBuffer* buffer = nullptr;
  uint64_t offset = 0;
  uint64_t range = 0;
};

struct ResourceWriteDesc
{
  IResourceSet* resourceSet = nullptr;
  uint32_t binding = 0;
  ResourceBindingType type = ResourceBindingType::eUniformBuffer;
  BufferResourceWrite buffer {};
};

struct PushConstantRangeDesc
{
  ShaderStageFlags stageFlags = 0;
  uint32_t offset = 0;
  uint32_t size = 0;
};

enum class PrimitiveTopology : uint8_t
{
  eTriangleList = 0
};

enum class PolygonMode : uint8_t
{
  eFill = 0
};

enum class CullMode : uint8_t
{
  eNone = 0,
  eBack,
};

enum class FrontFace : uint8_t
{
  eCounterClockwise = 0,
  eClockwise,
};

enum class CompareOp : uint8_t
{
  eNever = 0,
  eLess,
  eLessOrEqual,
  eAlways,
};

struct GraphicsPipelineDesc
{
  IShaderModule* vertexShaderModule = nullptr;
  IShaderModule* fragmentShaderModule = nullptr;
  const ResourceLayoutDesc* resourceLayoutDescs = nullptr;
  uint32_t resourceLayoutDescCount = 0;
  const PushConstantRangeDesc* pushConstantRanges = nullptr;
  uint32_t pushConstantRangeCount = 0;
  PrimitiveTopology topology = PrimitiveTopology::eTriangleList;
  PolygonMode polygonMode = PolygonMode::eFill;
  CullMode cullMode = CullMode::eBack;
  FrontFace frontFace = FrontFace::eCounterClockwise;
  bool depthTestEnable = false;
  bool depthWriteEnable = false;
  CompareOp depthCompareOp = CompareOp::eLess;
  TextureFormat colorAttachmentFormat = TextureFormat::eBGRA32_SRGB;
  TextureFormat depthAttachmentFormat = TextureFormat::eD32_SFLOAT;
  bool hasDepthAttachment = true;
};

class IGraphicsPipeline : public core::resource::IResource
{
 public:
  virtual core::Result init(const GraphicsPipelineDesc& desc) = 0;
  virtual const GraphicsPipelineDesc& getDesc() const = 0;
  virtual core::resource::Object getPipelineLayoutNativeObject() = 0;
  virtual IResourceLayout* getResourceLayout(uint32_t resourceSetIndex) const = 0;
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
  ITextureView* swapchainImageView = nullptr;
  ITextureView* depthAttachmentView = nullptr;
  ClearBufferValue clearValue {};
  RenderArea renderArea {};
};
struct TextureTransitionInfo
{
  ITexture* texture;
  TextureLayout oldLayout;
  TextureLayout newLayout;
};

class ICommandList : public core::resource::IResource
{
 public:
  virtual core::Result init(const CommandListDesc& desc) = 0;

  virtual core::Result begin(const CommandListBegindDesc& desc) = 0;
  virtual core::Result end() = 0;

  virtual core::Result copyBuffer(
    IBuffer* srcBuffer, size_t srcOffset, IBuffer* dstBuffer, size_t dstOffset, size_t size) = 0;

  virtual core::Result copyBufferToImage(IBuffer* buffer,
    size_t bufferOffset,
    ITexture* texture,
    uint32_t mipLevel,
    const TextureOffset3D& textureOffset) = 0;
  virtual core::Result transitionTexture(const TextureTransitionInfo& info) = 0;

  virtual void beginRendering(CommandListBeginRenderingInfo& info) = 0;
  virtual void endRendering() = 0;

  virtual void bindGraphicsPipeline(IGraphicsPipeline* pipeline) = 0;
  virtual core::Result bindResourceSets(IGraphicsPipeline* graphicsPipeline,
    uint32_t firstSet,
    IResourceSet* const* resourceSets,
    uint32_t resourceSetCount) = 0;
  virtual core::Result pushConstants(
    IGraphicsPipeline* graphicsPipeline, const PushConstantRangeDesc& range, const void* data) = 0;
  virtual void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
};

constexpr uint32_t kMaxSubmitCmdListCount = 8;
class ICommandQueue : public core::resource::IResource
{
 public:
  struct SubmitInfo
  {
    ICommandList* const* cmdLists;
    uint32_t cmdListCount;
    ISemaphore* const waitSemaphore;
    PipelineStage waitStage;
    ISemaphore* const signalSemaphore;
    IFence* const signalFence;
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
  virtual core::Result resize(uint32_t width, uint32_t height) = 0;
  virtual core::Result acquireNextTexture(
    uint64_t timeout, ISemaphore* signalSemaphore, IFence* signalFence, uint32_t& textureIndex) = 0;
  virtual core::Result present(uint32_t textureIndex, ISemaphore* waitSemaphore) = 0;
  virtual uint32_t getTextureCount() const = 0;
  virtual ITexture* getTexture(uint32_t index) = 0;
  virtual ITextureView* getTextureView(uint32_t index) = 0;
};
class ITexture : public core::resource::IResource
{
 public:
  virtual core::Result init(const TextureDesc& desc) = 0;
  virtual const TextureDesc& getDesc() const = 0;
  virtual void setLayout(TextureLayout layout) = 0;
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

  virtual std::unique_ptr<IShaderModule> createShaderModule() = 0;
  virtual std::unique_ptr<IResourceSet> createResourceSet() = 0;
  virtual std::unique_ptr<IGraphicsPipeline> createGraphicsPipeline() = 0;
  virtual core::Result updateResourceSets(const ResourceWriteDesc* writes, uint32_t writeCount) = 0;

  virtual ICommandQueue* getGraphicsQueue() = 0;
  virtual ISwapchain* getSwapchain() = 0;
};

#if defined MENTAL_WITH_VULKAN
struct VulkanSurfaceCreateInput
{
  using CreateSurfaceFn = core::Result (*)(VkInstance instance, void* userData, VkSurfaceKHR* surface);

  CreateSurfaceFn createSurface = nullptr;
  void* userData = nullptr;
};
#endif

struct DeviceInitInput
{
#if defined MENTAL_WITH_VULKAN
  VulkanSurfaceCreateInput vulkanSurface {};
#endif
};

[[nodiscard]] core::Result validateDeviceInitInput(GraphicsApi api, const DeviceInitInput& initInput);
[[nodiscard]] core::Result initDevice(GraphicsApi api, const DeviceInitInput& initInput);
IDevice& getDevice();
void destroyDevice();

} // namespace mental::rhi

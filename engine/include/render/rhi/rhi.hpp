#pragma once

#include <core/resource.hpp>

namespace mental::platform
{
class IWindow;
}

namespace mental::rhi
{

enum class Result
{
    eSuccess = 0,
    eDeviceInitializationFailed,
    eInstanceInitializationFailed,
    ePhysicalDeviceInitializationFailed,
    eLogicalDeviceInitializationFailed,
    eSurfaceInitializationFailed,
    eBufferInitializationFailed,
    eBufferMapFailed,
    eBufferCopyFailed,
    eCommandQueueInitializationFailed,
    eQueueSubmitFailed,
    eSemaphoreInitializationFailed
};

const char* resultToString(Result res);

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
    virtual const BufferDesc& getDesc() const = 0;
    virtual rhi::Result map(void** mappedData) = 0;
    virtual rhi::Result unmap() = 0;
    virtual rhi::Result copy(void* data, uint64_t size, uint64_t offset = 0) = 0;
};
using BufferHandle = core::memory::SharedHandle<IBuffer>;

class ISemaphore : public core::resource::IResource
{
    // TODO
};

class IFence : public core::resource::IResource
{
    // TODO
};

class ICommandList : public core::resource::IResource
{
    // TODO
};

enum class PipelineStage : uint8_t
{
    eColorAttachmentOutput = 0
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

    virtual rhi::Result submit(const SubmitInfo& info) = 0;
    virtual void waitIdle() = 0;
    // TODO
};

class IDevice : public core::resource::IResource
{
public:
    virtual void waitIdle() = 0;
    virtual GraphicsApi getGraphicsApi() = 0;

    virtual rhi::Result createBuffer(BufferDesc desc, core::memory::SharedHandle<IBuffer>& outBuffer) = 0;

    virtual ICommandQueue* getGraphicsQueue() = 0;

    // TODO: extend interface
};

void initDevice(GraphicsApi api, const mental::platform::IWindow* const window);
IDevice& getDevice();

}  // namespace mental::rhi

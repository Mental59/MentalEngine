#pragma once

#include <core/memory.hpp>

namespace mental::platform
{
class IWindow;
}

namespace mental::rhi
{
enum class Result
{
    eSuccess = 0,
    eInstanceInitializationFailed,
    ePhysicalDeviceInitializationFailed,
    eLogicalDeviceInitializationFailed,
    eSurfaceInitializationFailed,
    eBufferInitializationFailed,
    eBufferMapFailed,
    eBufferUploadFailed
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
typedef uint32_t BufferUsageFlags;

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

class IBuffer : public core::memory::IResource
{
public:
    virtual const BufferDesc& getDesc() const = 0;
    virtual rhi::Result map(void** mappedData) = 0;
    virtual rhi::Result unmap() = 0;
    virtual rhi::Result copy(void* data, uint64_t size, uint64_t offset = 0) = 0;
};
typedef core::memory::RefCountPtr<IBuffer> BufferHandle;

class IDevice : public core::memory::IResource
{
public:
    virtual void waitIdle() = 0;
    virtual GraphicsApi getGraphicsApi() = 0;

    virtual rhi::Result createBuffer(BufferDesc desc, BufferHandle& buffer) = 0;

    // TODO: extend interface
};
typedef core::memory::RefCountPtr<IDevice> DeviceHandle;

DeviceHandle createDevice(GraphicsApi api, const mental::platform::IWindow* const window);

}  // namespace mental::rhi

#pragma once

#include <core/memory.hpp>

namespace mental::platform
{
class IWindow;
}

namespace mental::rhi
{
enum class GraphicsApi : uint8_t
{
    Vulkan
};

const char* graphicsApiToString(GraphicsApi api);

enum class BufferType : uint8_t
{
    eStorage,
    eUniform,
    eNone
};

struct BufferDesc
{
    BufferType type = BufferType::eStorage;
    uint64_t byteSize = 0;
    bool isTransferDst = false;
    bool isTransferSrc = false;
};

class IBuffer : public core::memory::IResource
{
public:
    virtual const BufferDesc& getDesc() const = 0;
};
typedef core::memory::RefCountPtr<IBuffer> BufferHandle;

class IDevice : public core::memory::IObject
{
public:
    IDevice() = default;
    IDevice(const IDevice&) = delete;
    IDevice(const IDevice&&) = delete;
    IDevice& operator=(const IDevice&) = delete;
    IDevice& operator=(const IDevice&&) = delete;

    virtual void waitIdle() = 0;
    virtual void destroy() = 0;
    virtual GraphicsApi getGraphicsApi() = 0;

    virtual BufferHandle createBuffer(BufferDesc desc) = 0;

    // TODO: extend interface
};

IDevice* createDevice(GraphicsApi api, const mental::platform::IWindow* const window);

}  // namespace mental::rhi

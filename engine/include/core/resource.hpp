#pragma once

#include <core/memory.hpp>

namespace mental::core::resource
{
struct Object
{
    union
    {
        uint64_t integer;
        void* pointer;
    };

    Object(uint64_t i) : integer(i) {}
    Object(void* p) : pointer(p) {}

    template <typename T>
    operator T*() const
    {
        return static_cast<T*>(pointer);
    }
};

enum class ObjectType : uint8_t
{
    eVkCommandBuffer = 0,
    eVkQueue,
    eVkCommandPool,
    eVkBuffer,
    eVkSemaphore,
    eVkFence
};

class IResource : public core::memory::NonCopyable
{
public:
    virtual Object getNativeObject(ObjectType objectType) { return nullptr; }
};
}  // namespace mental::core::resource
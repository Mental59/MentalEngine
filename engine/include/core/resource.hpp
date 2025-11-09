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

using ObjectType = uint32_t;

namespace ObjectTypes
{
constexpr ObjectType vkCommandBuffer = 1;
constexpr ObjectType vkQueue = 2;
constexpr ObjectType vkCommandPool = 3;
constexpr ObjectType vkBuffer = 4;
constexpr ObjectType vkSemaphore = 5;
constexpr ObjectType vkFence = 6;
}  // namespace ObjectTypes

class IResource : public core::memory::NonCopyable
{
public:
    virtual Object getNativeObject(ObjectType objectType) { return nullptr; }
};
}  // namespace mental::core::resource
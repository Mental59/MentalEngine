#pragma once

#include <memory>

namespace mental::core::memory
{
template <typename T>
using UniqueHandle = std::unique_ptr<T>;

template <typename T>
using SharedHandle = std::shared_ptr<T>;

template <typename T>
using WeakHandle = std::weak_ptr<T>;

template <typename T, typename... Args>
SharedHandle<T> makeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
UniqueHandle<T> makeUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

class NonCopyable
{
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable& other) = delete;
    NonCopyable& operator=(const NonCopyable& other) = delete;
};
}  // namespace mental::core::memory

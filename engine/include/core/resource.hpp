#pragma once

#include <cstdint>

namespace mental::core::resource
{
  struct Object
  {
    union
    {
      uint64_t integer;
      void* pointer;
    };

    Object(uint64_t i) : integer(i)
    {
    }
    Object(void* p) : pointer(p)
    {
    }

    template<typename T>
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
    eVkFence,
    eVkDevice,
    eVkPhysicalDevice,
    eVkSurfaceKHR,
    eVkSwapchainKHR,
    eVkImage,
    eVkImageView,
    eGLFWwindow
  };

  class IResource
  {
   public:
    IResource() = default;
    IResource(const IResource& other) = delete;
    IResource& operator=(const IResource& other) = delete;
    IResource(IResource&& other) = default;  // allow moving
    IResource& operator=(IResource&& other) = delete;

    virtual Object getNativeObject(ObjectType objectType)
    {
      return nullptr;
    }
    virtual bool isValid() const
    {
      return true;
    }
    virtual void destroy() = 0;
  };

  template<typename T>
  class ResourceGuard
  {
   public:
    ResourceGuard(T* resource) : mResource(resource)
    {
    }

    ~ResourceGuard()
    {
      mResource->destroy();
    }

   private:
    T* mResource;
  };

  class ResourceHandle
  {
   public:
    bool isValid() const
    {
      return id > 0;
    }

    size_t id;
  };
}  // namespace mental::core::resource

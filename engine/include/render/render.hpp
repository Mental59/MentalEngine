#pragma once

#include <render/rhi/rhi.hpp>
#include "core/resource.hpp"

namespace mental::platform
{
  class IWindow;
}

namespace mental::render
{
  class BufferHandle;

  struct RenderSystemConfig
  {
    rhi::GraphicsApi graphicsApi;
    mental::platform::IWindow* window;
  };

  class RenderSystem : public core::resource::IResource
  {
   public:
    static RenderSystem& instance()
    {
      static RenderSystem renderSystem;
      return renderSystem;
    }

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem(const RenderSystem&&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&&) = delete;

    core::Result init(const RenderSystemConfig& conf);
    virtual void destroy() override;

   private:
    RenderSystem() = default;
    bool mIsInitialized = false;
  };

  inline RenderSystem& getRenderSystem()
  {
    return RenderSystem::instance();
  }

  class IResourceManager
  {
   public:
    IResourceManager() = default;
    IResourceManager(const IResourceManager& other) = delete;
    IResourceManager& operator=(const IResourceManager& other) = delete;
    IResourceManager(IResourceManager&& other) = delete;
    IResourceManager& operator=(IResourceManager&& other) = delete;

    virtual BufferHandle createBuffer(const rhi::BufferDesc& desc) = 0;
    virtual rhi::IBuffer* getBuffer(BufferHandle handle) = 0;
    virtual void destroyBuffer(BufferHandle handle) = 0;
  };

  void initResourceManager();
  void destroyResourceManager();
  IResourceManager& getResourceManager();

  class BufferHandle : public core::resource::ResourceHandle
  {
   public:
    static BufferHandle invalid()
    {
      return { 0 };
    }

    inline void destroy() const
    {
      getResourceManager().destroyBuffer(*this);
    }

    inline rhi::IBuffer* get() const
    {
      return getResourceManager().getBuffer(*this);
    }
  };
}  // namespace mental::render

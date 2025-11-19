#include <cstdint>
#include <vector>
#include <render/render.hpp>
#include <render/rhi/rhi.hpp>

#ifdef MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/buffer.hpp>

namespace mental::render
{
  constexpr uint32_t kMaxBuffers = 1 << 14;

  class ResourceManagerImpl : public IResourceManager
  {
   public:
    void init()
    {
      // TODO
    }

    void destroy()
    {
      // TODO
    }

    virtual BufferHandle createBuffer(const rhi::BufferDesc& desc) override
    {
      return BufferHandle::invalid();
      // TODO
    }

    virtual rhi::IBuffer* getBuffer(BufferHandle handle) override
    {
      return nullptr;
      // TODO
    }

    virtual void destroyBuffer(BufferHandle handle) override
    {
      (void)handle;
      // TODO
    }

   private:
    std::vector<rhi::vk::Buffer> mBuffers;
  };

  static ResourceManagerImpl gResourceManager;
  static bool gIsResourceManagerInit = false;

  void initResourceManager()
  {
    if (gIsResourceManagerInit)
      return;

    gResourceManager.init();
  }

  IResourceManager& getResourceManager()
  {
    return gResourceManager;
  }

  void destroyResourceManager()
  {
    if (!gIsResourceManagerInit)
      return;

    gResourceManager.destroy();
  }
}  // namespace mental::render

#endif

#include <queue>
#include <vector>
#include <render/render.hpp>
#include <render/rhi/rhi.hpp>
#include "core/log.hpp"
#include "core/types.hpp"

#ifdef MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/buffer.hpp>

namespace mental::render
{
  constexpr size_t kInitialBuffersVectorSize = 16384;

  class ResourceManagerImpl : public IResourceManager
  {
   public:
    void init()
    {
      mBuffers.resize(kInitialBuffersVectorSize);
      for (size_t i = 0; i < kInitialBuffersVectorSize; i++)
      {
        mFreeBuffersIndices.push(i);
      }
      mIsInit = true;
    }

    void destroy()
    {
      if (!mIsInit)
      {
        return;
      }
    }

    virtual BufferHandle createBuffer(const rhi::BufferDesc& desc) override
    {
      if (mFreeBuffersIndices.empty())
      {
        resize();
      }

      size_t freeIndex = mFreeBuffersIndices.front();

      BufferHandle handle{ freeIndex + 1 };
      rhi::vk::Buffer* buf = getBufferByHandle(handle);

      core::Result res = buf->init(desc);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create buffer, error: {}", core::resultToString(res));
        return BufferHandle::invalid();
      }

      mFreeBuffersIndices.pop();

      return handle;
    }

    virtual rhi::IBuffer* getBuffer(BufferHandle handle) override
    {
      rhi::vk::Buffer* buf = getBufferByHandle(handle);
      return buf->isValid() ? buf : nullptr;
    }

    virtual void destroyBuffer(BufferHandle handle) override
    {
      rhi::vk::Buffer* buf = getBufferByHandle(handle);
      if (buf->isValid())
      {
        buf->destroy();
      }
    }

   private:
    rhi::vk::Buffer* getBufferByHandle(BufferHandle handle)
    {
      size_t index = handle.id - 1;
      rhi::vk::Buffer* buf = &mBuffers[handle.id - 1];
      return buf;
    }

    void resize()
    {
      size_t curSize = mBuffers.size();
      mBuffers.resize(curSize * 2);
      for (size_t i = curSize; i < curSize * 2; i++)
      {
        mFreeBuffersIndices.push(i);
      }
    }

    std::vector<rhi::vk::Buffer> mBuffers;
    std::queue<size_t> mFreeBuffersIndices;
    bool mIsInit = false;
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

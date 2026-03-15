#include <queue>
#include <vector>
#include <array>
#include <render/rhi/rhi.hpp>
#include <resource/resourceManager.hpp>
#include "core/log.hpp"
#include "core/types.hpp"

#ifdef MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/buffer.hpp>
#include <render/rhi/vulkan/commandList.hpp>

namespace mental::resource
{
  constexpr size_t kInitialBuffersVectorSize = 16384;
  constexpr size_t kMaxCommandLists = 16;

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

      for (size_t i = 0; i < kMaxCommandLists; i++)
      {
        mFreeCmdListsIndices.push(i);
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
        resizeBuffersArray();
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
        mFreeBuffersIndices.push(handle.id - 1);
      }
    }

    virtual CommandListHandle createCommandList(const rhi::CommandListDesc& desc) override
    {
      size_t cmdListIndex = mFreeCmdListsIndices.front();

      CommandListHandle handle{ cmdListIndex + 1 };
      core::Result res = mCmdLists[cmdListIndex].init(desc);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create command list, error: {}", core::resultToString(res));
        return CommandListHandle::invalid();
      }

      mFreeCmdListsIndices.pop();

      return handle;
    };

    virtual rhi::ICommandList* getCommandList(CommandListHandle handle) override
    {
      size_t index = handle.id - 1;
      return mCmdLists[index].isValid() ? &mCmdLists[index] : nullptr;
    };

    virtual void destroyCommandList(CommandListHandle handle) override
    {
      size_t index = handle.id - 1;
      if (mCmdLists[index].isValid())
      {
        mCmdLists[index].destroy();
        mFreeCmdListsIndices.push(index);
      }
    };

   private:
    rhi::vk::Buffer* getBufferByHandle(BufferHandle handle)
    {
      size_t index = handle.id - 1;
      rhi::vk::Buffer* buf = &mBuffers[index];
      return buf;
    }

    void resizeBuffersArray()
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

    std::array<rhi::vk::CommandList, kMaxCommandLists> mCmdLists;
    std::queue<size_t> mFreeCmdListsIndices;

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
}  // namespace mental::resource

#endif

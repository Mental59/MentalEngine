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
#include <render/rhi/vulkan/fence.hpp>
#include <render/rhi/vulkan/semaphore.hpp>

namespace mental::resource
{
  constexpr size_t kInitialBuffersVectorSize = 16384;
  constexpr size_t kMaxCommandLists = 16;

  struct VulkanFrameData
  {
    rhi::vk::CommandList cmdList;
    rhi::vk::Fence fence;
    rhi::vk::Semaphore imageAvailableSemaphore;
    rhi::vk::Semaphore renderFinishedSemaphore;

    inline bool isValid() const
    {
      return cmdList.isValid() && fence.isValid() && imageAvailableSemaphore.isValid() && renderFinishedSemaphore.isValid();
    }
  };

  class ResourceManagerImpl : public IResourceManager
  {
   public:
    void init(uint32_t maxFramesInFlight)
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

      mFrameDataArray.resize(maxFramesInFlight);
      for (size_t i = 0; i < maxFramesInFlight; i++)
      {
        mFreeFrameDataIndices.push(i);
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

      size_t bufferIndex = mFreeBuffersIndices.front();
      core::Result res = mBuffers[bufferIndex].init(desc);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create buffer, error: {}", core::resultToString(res));
        return BufferHandle::invalid();
      }

      mFreeBuffersIndices.pop();

      BufferHandle handle{ bufferIndex + 1 };
      return handle;
    }

    virtual rhi::IBuffer* getBuffer(BufferHandle handle) override
    {
      size_t index = handle.id - 1;
      return mBuffers[index].isValid() ? &mBuffers[index] : nullptr;
    }

    virtual void destroyBuffer(BufferHandle handle) override
    {
      size_t index = handle.id - 1;
      if (mBuffers[index].isValid())
      {
        mBuffers[index].destroy();
        mFreeBuffersIndices.push(index);
      }
    }

    virtual CommandListHandle createCommandList(const rhi::CommandListDesc& desc) override
    {
      size_t cmdListIndex = mFreeCmdListsIndices.front();
      core::Result res = mCmdLists[cmdListIndex].init(desc);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create command list, error: {}", core::resultToString(res));
        return CommandListHandle::invalid();
      }

      mFreeCmdListsIndices.pop();

      CommandListHandle handle{ cmdListIndex + 1 };
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

    virtual FrameDataHandle createFrameData(const CreateFrameDataDesc& desc) override
    {
      MENTAL_ASSERT_MESSAGE(!mFreeFrameDataIndices.empty(), "Frame data array is full");

      size_t frameDataIndex = mFreeFrameDataIndices.front();
      FrameDataHandle handle{ frameDataIndex + 1 };

      core::Result res = mFrameDataArray[frameDataIndex].cmdList.init(desc.cmdListDesc);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create cmdList for frame data, error: {}", core::resultToString(res));
        return FrameDataHandle::invalid();
      }

      res = mFrameDataArray[frameDataIndex].fence.init(desc.fenceDesc);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create fence for frame data, error: {}", core::resultToString(res));
        destroyFrameData(handle);
        return FrameDataHandle::invalid();
      }

      res = mFrameDataArray[frameDataIndex].imageAvailableSemaphore.init();
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create imageAvailableSemaphore for frame data, error: {}", core::resultToString(res));
        destroyFrameData(handle);
        return FrameDataHandle::invalid();
      }

      res = mFrameDataArray[frameDataIndex].renderFinishedSemaphore.init();
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create renderFinishedSemaphore frame data, error: {}", core::resultToString(res));
        destroyFrameData(handle);
        return FrameDataHandle::invalid();
      }

      mFreeFrameDataIndices.pop();

      return handle;
    }

    virtual FrameData getFrameData(FrameDataHandle handle) override
    {
      size_t index = handle.id - 1;
      VulkanFrameData& vkFrameData = mFrameDataArray[index];

      FrameData frameData{};
      if (vkFrameData.isValid())
      {
        frameData.cmdList = &vkFrameData.cmdList;
        frameData.fence = &vkFrameData.fence;
        frameData.imageAvailableSemaphore = &vkFrameData.imageAvailableSemaphore;
        frameData.renderFinishedSemaphore = &vkFrameData.renderFinishedSemaphore;
      }

      return frameData;
    }

    virtual void destroyFrameData(FrameDataHandle handle) override
    {
      size_t index = handle.id - 1;

      VulkanFrameData& frameData = mFrameDataArray[index];
      bool wasValid = frameData.isValid();

      if (frameData.cmdList.isValid())
      {
        frameData.cmdList.destroy();
      }
      if (frameData.fence.isValid())
      {
        frameData.fence.destroy();
      }
      if (frameData.imageAvailableSemaphore.isValid())
      {
        frameData.imageAvailableSemaphore.destroy();
      }
      if (frameData.renderFinishedSemaphore.isValid())
      {
        frameData.renderFinishedSemaphore.destroy();
      }

      if (wasValid)
      {
        mFreeFrameDataIndices.push(index);
      }
    }

   private:
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

    std::vector<VulkanFrameData> mFrameDataArray;
    std::queue<size_t> mFreeFrameDataIndices;

    bool mIsInit = false;
  };

  static ResourceManagerImpl gResourceManager;
  static bool gIsResourceManagerInit = false;

  void initResourceManager(uint32_t maxFramesInFlight)
  {
    if (gIsResourceManagerInit)
      return;

    gResourceManager.init(maxFramesInFlight);
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

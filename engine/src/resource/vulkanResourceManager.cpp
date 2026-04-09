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
#include <render/rhi/vulkan/texture.hpp>
#include <render/rhi/vulkan/semaphore.hpp>

namespace mental::resource
{
constexpr std::size_t kInitialBuffersVectorSize = 16384;
constexpr std::size_t kInitialTexturesVectorSize = 16384;
constexpr std::size_t kInitialTextureViewsVectorSize = 16384;
constexpr std::size_t kMaxCommandLists = 16;

struct VulkanFrameData
{
  rhi::vk::CommandList cmdList;
  rhi::vk::Fence fence;
  rhi::vk::Semaphore imageAvailableSemaphore;
  rhi::vk::Semaphore renderFinishedSemaphore;

  inline bool isValid() const
  {
    return cmdList.isValid() && fence.isValid() && imageAvailableSemaphore.isValid() &&
           renderFinishedSemaphore.isValid();
  }
};

class ResourceManagerImpl : public IResourceManager
{
 public:
  void init(uint32_t maxFramesInFlight)
  {
    mBuffers.clear();
    mTextures.clear();
    mTextureViews.clear();
    mFrameDataArray.clear();
    mFreeBuffersIndices = {};
    mFreeTextureIndices = {};
    mFreeTextureViewIndices = {};
    mFreeCmdListsIndices = {};
    mFreeFrameDataIndices = {};

    mBuffers.resize(kInitialBuffersVectorSize);
    for (std::size_t i = 0; i < kInitialBuffersVectorSize; i++)
    {
      mFreeBuffersIndices.push(i);
    }

    mTextures.resize(kInitialTexturesVectorSize);
    for (std::size_t i = 0; i < kInitialTexturesVectorSize; i++)
    {
      mFreeTextureIndices.push(i);
    }

    mTextureViews.resize(kInitialTextureViewsVectorSize);
    for (std::size_t i = 0; i < kInitialTextureViewsVectorSize; i++)
    {
      mFreeTextureViewIndices.push(i);
    }

    for (std::size_t i = 0; i < kMaxCommandLists; i++)
    {
      mFreeCmdListsIndices.push(i);
    }

    mFrameDataArray.resize(maxFramesInFlight);
    for (std::size_t i = 0; i < maxFramesInFlight; i++)
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

    for (VulkanFrameData& frameData : mFrameDataArray)
    {
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
    }

    for (rhi::vk::TextureView& textureView : mTextureViews)
    {
      if (textureView.isValid())
      {
        textureView.destroy();
      }
    }

    for (rhi::vk::Texture& texture : mTextures)
    {
      if (texture.isValid())
      {
        texture.destroy();
      }
    }

    for (rhi::vk::Buffer& buffer : mBuffers)
    {
      if (buffer.isValid())
      {
        buffer.destroy();
      }
    }

    for (rhi::vk::CommandList& cmdList : mCmdLists)
    {
      if (cmdList.isValid())
      {
        cmdList.destroy();
      }
    }

    mFreeBuffersIndices = {};
    mFreeTextureIndices = {};
    mFreeTextureViewIndices = {};
    mFreeCmdListsIndices = {};
    mFreeFrameDataIndices = {};
    mIsInit = false;
  }

  virtual BufferHandle createBuffer(const rhi::BufferDesc& desc) override
  {
    if (mFreeBuffersIndices.empty())
    {
      resizeBuffersArray();
    }

    std::size_t bufferIndex = mFreeBuffersIndices.front();
    core::Result res = mBuffers[bufferIndex].init(desc);
    if (res != core::Result::eSuccess)
    {
      MENTAL_ERROR("Failed to create buffer, error: {}", core::resultToString(res));
      return BufferHandle::invalid();
    }

    mFreeBuffersIndices.pop();

    BufferHandle handle {bufferIndex + 1};
    return handle;
  }

  virtual rhi::IBuffer* getBuffer(BufferHandle handle) override
  {
    if (!handle.isValid())
    {
      return nullptr;
    }

    std::size_t index = handle.id - 1;
    if (index >= mBuffers.size() || !mBuffers[index].isValid())
    {
      return nullptr;
    }

    return &mBuffers[index];
  }

  virtual void destroyBuffer(BufferHandle handle) override
  {
    if (!handle.isValid())
    {
      return;
    }

    std::size_t index = handle.id - 1;
    if (index < mBuffers.size() && mBuffers[index].isValid())
    {
      mBuffers[index].destroy();
      mFreeBuffersIndices.push(index);
    }
  }

  virtual TextureHandle createTexture(const rhi::TextureDesc& desc) override
  {
    if (mFreeTextureIndices.empty())
    {
      resizeTexturesArray();
    }

    std::size_t textureIndex = mFreeTextureIndices.front();
    core::Result res = mTextures[textureIndex].init(desc);
    if (res != core::Result::eSuccess)
    {
      MENTAL_ERROR("Failed to create texture, error: {}", core::resultToString(res));
      return TextureHandle::invalid();
    }

    mFreeTextureIndices.pop();

    TextureHandle handle {textureIndex + 1};
    return handle;
  }

  virtual rhi::ITexture* getTexture(TextureHandle handle) override
  {
    if (!handle.isValid())
    {
      return nullptr;
    }

    std::size_t index = handle.id - 1;
    if (index >= mTextures.size() || !mTextures[index].isValid())
    {
      return nullptr;
    }

    return &mTextures[index];
  }

  virtual void destroyTexture(TextureHandle handle) override
  {
    if (!handle.isValid())
    {
      return;
    }

    std::size_t index = handle.id - 1;
    if (index < mTextures.size() && mTextures[index].isValid())
    {
      mTextures[index].destroy();
      mFreeTextureIndices.push(index);
    }
  }

  virtual TextureViewHandle createTextureView(const rhi::TextureViewDesc& desc) override
  {
    if (desc.texture == nullptr || !desc.texture->isValid())
    {
      MENTAL_ERROR("Failed to create texture view, texture is invalid");
      return TextureViewHandle::invalid();
    }

    if (mFreeTextureViewIndices.empty())
    {
      resizeTextureViewsArray();
    }

    std::size_t textureViewIndex = mFreeTextureViewIndices.front();
    core::Result res = mTextureViews[textureViewIndex].init(desc);
    if (res != core::Result::eSuccess)
    {
      MENTAL_ERROR("Failed to create texture view, error: {}", core::resultToString(res));
      return TextureViewHandle::invalid();
    }

    mFreeTextureViewIndices.pop();

    TextureViewHandle handle {textureViewIndex + 1};
    return handle;
  }

  virtual rhi::ITextureView* getTextureView(TextureViewHandle handle) override
  {
    if (!handle.isValid())
    {
      return nullptr;
    }

    std::size_t index = handle.id - 1;
    if (index >= mTextureViews.size() || !mTextureViews[index].isValid())
    {
      return nullptr;
    }

    return &mTextureViews[index];
  }

  virtual void destroyTextureView(TextureViewHandle handle) override
  {
    if (!handle.isValid())
    {
      return;
    }

    std::size_t index = handle.id - 1;
    if (index < mTextureViews.size() && mTextureViews[index].isValid())
    {
      mTextureViews[index].destroy();
      mFreeTextureViewIndices.push(index);
    }
  }

  virtual CommandListHandle createCommandList(const rhi::CommandListDesc& desc) override
  {
    std::size_t cmdListIndex = mFreeCmdListsIndices.front();
    core::Result res = mCmdLists[cmdListIndex].init(desc);
    if (res != core::Result::eSuccess)
    {
      MENTAL_ERROR("Failed to create command list, error: {}", core::resultToString(res));
      return CommandListHandle::invalid();
    }

    mFreeCmdListsIndices.pop();

    CommandListHandle handle {cmdListIndex + 1};
    return handle;
  };

  virtual rhi::ICommandList* getCommandList(CommandListHandle handle) override
  {
    if (!handle.isValid())
    {
      return nullptr;
    }

    std::size_t index = handle.id - 1;
    if (index >= mCmdLists.size() || !mCmdLists[index].isValid())
    {
      return nullptr;
    }

    return &mCmdLists[index];
  };

  virtual void destroyCommandList(CommandListHandle handle) override
  {
    if (!handle.isValid())
    {
      return;
    }

    std::size_t index = handle.id - 1;
    if (index < mCmdLists.size() && mCmdLists[index].isValid())
    {
      mCmdLists[index].destroy();
      mFreeCmdListsIndices.push(index);
    }
  };

  virtual FrameDataHandle createFrameData(const CreateFrameDataDesc& desc) override
  {
    MENTAL_ASSERT_MESSAGE(!mFreeFrameDataIndices.empty(), "Frame data array is full");

    std::size_t frameDataIndex = mFreeFrameDataIndices.front();
    FrameDataHandle handle {frameDataIndex + 1};

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
    if (!handle.isValid())
    {
      return {};
    }

    std::size_t index = handle.id - 1;
    if (index >= mFrameDataArray.size())
    {
      return {};
    }

    VulkanFrameData& vkFrameData = mFrameDataArray[index];

    FrameData frameData {};
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
    if (!handle.isValid())
    {
      return;
    }

    std::size_t index = handle.id - 1;
    if (index >= mFrameDataArray.size())
    {
      return;
    }

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
    std::size_t curSize = mBuffers.size();
    std::size_t newSize = curSize == 0 ? kInitialBuffersVectorSize : curSize * 2;
    mBuffers.resize(newSize);
    for (std::size_t i = curSize; i < newSize; i++)
    {
      mFreeBuffersIndices.push(i);
    }
  }

  void resizeTexturesArray()
  {
    std::size_t curSize = mTextures.size();
    std::size_t newSize = curSize == 0 ? kInitialTexturesVectorSize : curSize * 2;
    mTextures.resize(newSize);
    for (std::size_t i = curSize; i < newSize; i++)
    {
      mFreeTextureIndices.push(i);
    }
  }

  void resizeTextureViewsArray()
  {
    std::size_t curSize = mTextureViews.size();
    std::size_t newSize = curSize == 0 ? kInitialTextureViewsVectorSize : curSize * 2;
    mTextureViews.resize(newSize);
    for (std::size_t i = curSize; i < newSize; i++)
    {
      mFreeTextureViewIndices.push(i);
    }
  }

  std::vector<rhi::vk::Buffer> mBuffers;
  std::queue<std::size_t> mFreeBuffersIndices;

  std::vector<rhi::vk::Texture> mTextures;
  std::queue<std::size_t> mFreeTextureIndices;

  std::vector<rhi::vk::TextureView> mTextureViews;
  std::queue<std::size_t> mFreeTextureViewIndices;

  std::array<rhi::vk::CommandList, kMaxCommandLists> mCmdLists;
  std::queue<std::size_t> mFreeCmdListsIndices;

  std::vector<VulkanFrameData> mFrameDataArray;
  std::queue<std::size_t> mFreeFrameDataIndices;

  bool mIsInit = false;
};

static ResourceManagerImpl gResourceManager;
static bool gIsResourceManagerInit = false;

void initResourceManager(uint32_t maxFramesInFlight)
{
  if (gIsResourceManagerInit)
    return;

  gResourceManager.init(maxFramesInFlight);
  gIsResourceManagerInit = true;
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
  gIsResourceManagerInit = false;
}
} // namespace mental::resource

#endif

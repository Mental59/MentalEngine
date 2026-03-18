#pragma once

#include <render/rhi/rhi.hpp>

namespace mental::resource
{
class BufferHandle;
class CommandListHandle;
class FrameDataHandle;

struct FrameData
{
  rhi::ICommandList* cmdList = nullptr;
  rhi::IFence* fence = nullptr;
  rhi::ISemaphore* imageAvailableSemaphore = nullptr;
  rhi::ISemaphore* renderFinishedSemaphore = nullptr;

  bool isValid() const;
};

struct CreateFrameDataDesc
{
  rhi::CommandListDesc cmdListDesc;
  rhi::FenceDesc fenceDesc;
};

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

  virtual CommandListHandle createCommandList(const rhi::CommandListDesc& desc) = 0;
  virtual rhi::ICommandList* getCommandList(CommandListHandle handle) = 0;
  virtual void destroyCommandList(CommandListHandle handle) = 0;

  virtual FrameDataHandle createFrameData(const CreateFrameDataDesc& desc) = 0;
  virtual FrameData getFrameData(FrameDataHandle handle) = 0;
  virtual void destroyFrameData(FrameDataHandle handle) = 0;
};

void initResourceManager(uint32_t maxFramesInFlight);
void destroyResourceManager();
IResourceManager& getResourceManager();

class BufferHandle : public core::resource::ResourceHandle
{
 public:
  static BufferHandle invalid()
  {
    return {0};
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

class CommandListHandle : public core::resource::ResourceHandle
{
 public:
  static CommandListHandle invalid()
  {
    return {0};
  }

  inline void destroy() const
  {
    getResourceManager().destroyCommandList(*this);
  }

  inline rhi::ICommandList* get() const
  {
    return getResourceManager().getCommandList(*this);
  }
};

class FrameDataHandle : public core::resource::ResourceHandle
{
 public:
  static FrameDataHandle invalid()
  {
    return {0};
  }

  inline void destroy() const
  {
    getResourceManager().destroyFrameData(*this);
  }

  inline FrameData get() const
  {
    return getResourceManager().getFrameData(*this);
  }
};
} // namespace mental::resource

#pragma once

namespace mental::resource
{
  class BufferHandle;
  class CommandListHandle;

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

  class CommandListHandle : public core::resource::ResourceHandle
  {
   public:
    static CommandListHandle invalid()
    {
      return { 0 };
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
}  // namespace mental::resource

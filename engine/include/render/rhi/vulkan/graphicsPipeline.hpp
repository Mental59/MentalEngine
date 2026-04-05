#pragma once

#include <render/rhi/vulkan/resourceSet.hpp>
#include <render/rhi/rhi.hpp>

#include <vector>

#include <volk/volk.h>

namespace mental::rhi::vk
{
class PipelineLayout : public IPipelineLayout
{
 public:
  PipelineLayout() = default;

  virtual core::Result init(const PipelineLayoutDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual const PipelineLayoutDesc& getDesc() const override;
  virtual IResourceLayout* getResourceLayout(uint32_t resourceSetIndex) const override;

 private:
  std::vector<ResourceLayout> mResourceLayouts {};
  std::vector<ResourceLayoutDesc> mResourceLayoutDescs {};
  std::vector<PushConstantRangeDesc> mPushConstantRanges {};
  PipelineLayoutDesc mDesc {};
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
  bool mIsInitialized = false;
};

class GraphicsPipeline : public IGraphicsPipeline
{
 public:
  GraphicsPipeline() = default;

  virtual core::Result init(const GraphicsPipelineDesc& desc) override;
  virtual void destroy() override;
  virtual bool isValid() const override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual const GraphicsPipelineDesc& getDesc() const override;

 private:
  GraphicsPipelineDesc mDesc {};
  VkPipeline mPipeline = VK_NULL_HANDLE;
  bool mIsInitialized = false;
};
} // namespace mental::rhi::vk

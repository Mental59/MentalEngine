#include <render/rhi/vulkan/graphicsPipeline.hpp>

#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/device.hpp>

#include <array>
#include <vector>

namespace
{
std::vector<VkDescriptorSetLayout> collectDescriptorSetLayouts(
  std::vector<mental::rhi::vk::ResourceLayout>& resourceLayouts)
{
  std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts {};
  vkDescriptorSetLayouts.reserve(resourceLayouts.size());
  for (mental::rhi::vk::ResourceLayout& resourceLayout : resourceLayouts)
  {
    vkDescriptorSetLayouts.push_back(
      resourceLayout.getNativeObject(mental::core::resource::ObjectType::eVkDescriptorSetLayout));
  }
  return vkDescriptorSetLayouts;
}

std::vector<VkPushConstantRange> collectPushConstantRanges(
  const std::vector<mental::rhi::PushConstantRangeDesc>& pushConstantRanges)
{
  std::vector<VkPushConstantRange> vkPushConstantRanges {};
  vkPushConstantRanges.reserve(pushConstantRanges.size());
  for (const mental::rhi::PushConstantRangeDesc& pushConstantRange : pushConstantRanges)
  {
    VkPushConstantRange vkPushConstantRange {};
    vkPushConstantRange.stageFlags = mental::rhi::vk::convertShaderStageFlags(pushConstantRange.stageFlags);
    vkPushConstantRange.offset = pushConstantRange.offset;
    vkPushConstantRange.size = pushConstantRange.size;
    vkPushConstantRanges.push_back(vkPushConstantRange);
  }
  return vkPushConstantRanges;
}

mental::core::Result createPipelineLayout(std::vector<mental::rhi::vk::ResourceLayout>& resourceLayouts,
  const std::vector<mental::rhi::PushConstantRangeDesc>& pushConstantRanges,
  VkPipelineLayout& pipelineLayout)
{
  const std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts = collectDescriptorSetLayouts(resourceLayouts);
  const std::vector<VkPushConstantRange> vkPushConstantRanges = collectPushConstantRanges(pushConstantRanges);
  VkPipelineLayoutCreateInfo createInfo {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  createInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
  createInfo.pSetLayouts = vkDescriptorSetLayouts.data();
  createInfo.pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRanges.size());
  createInfo.pPushConstantRanges = vkPushConstantRanges.data();
  const VkResult result =
    vkCreatePipelineLayout(mental::rhi::vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &pipelineLayout);
  if (result != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to create Vulkan pipeline layout, error: {}", mental::rhi::vk::vkResultToString(result));
    return mental::core::Result::eInitializationFailed;
  }
  return mental::core::Result::eSuccess;
}
} // namespace

mental::core::Result mental::rhi::vk::GraphicsPipeline::init(const GraphicsPipelineDesc& desc)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized vk::GraphicsPipeline");
    return core::Result::eInitializationFailed;
  }

  if (desc.vertexShaderModule == nullptr || desc.fragmentShaderModule == nullptr)
  {
    MENTAL_ERROR("Graphics pipeline init requires vertex and fragment shaders");
    return core::Result::eInitializationFailed;
  }
  if ((desc.resourceLayoutDescCount > 0u && desc.resourceLayoutDescs == nullptr) ||
      (desc.pushConstantRangeCount > 0u && desc.pushConstantRanges == nullptr))
  {
    MENTAL_ERROR("Graphics pipeline init requires valid inline layout and push-constant ranges");
    return core::Result::eInitializationFailed;
  }

  mResourceLayouts.resize(desc.resourceLayoutDescCount);
  mResourceLayoutDescs.reserve(desc.resourceLayoutDescCount);
  for (uint32_t resourceLayoutIndex = 0u; resourceLayoutIndex < desc.resourceLayoutDescCount; ++resourceLayoutIndex)
  {
    core::Result result = mResourceLayouts[resourceLayoutIndex].init(desc.resourceLayoutDescs[resourceLayoutIndex]);
    if (result != core::Result::eSuccess)
    {
      for (uint32_t destroyIndex = 0u; destroyIndex < resourceLayoutIndex; ++destroyIndex)
      {
        mResourceLayouts[destroyIndex].destroy();
      }
      mResourceLayouts.clear();
      return result;
    }

    mResourceLayoutDescs.push_back(mResourceLayouts[resourceLayoutIndex].getDesc());
  }
  if (desc.pushConstantRangeCount > 0u)
  {
    mPushConstantRanges.assign(desc.pushConstantRanges, desc.pushConstantRanges + desc.pushConstantRangeCount);
  }

  core::Result result = createPipelineLayout(mResourceLayouts, mPushConstantRanges, mPipelineLayout);
  if (result != core::Result::eSuccess)
  {
    for (ResourceLayout& resourceLayout : mResourceLayouts)
    {
      resourceLayout.destroy();
    }
    mResourceLayouts.clear();
    mResourceLayoutDescs.clear();
    mPushConstantRanges.clear();
    return result;
  }

  const VkShaderModule vertexShaderModule =
    desc.vertexShaderModule->getNativeObject(core::resource::ObjectType::eVkShaderModule);
  const VkShaderModule fragmentShaderModule =
    desc.fragmentShaderModule->getNativeObject(core::resource::ObjectType::eVkShaderModule);
  MENTAL_ASSERT_DEBUG(vertexShaderModule != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(fragmentShaderModule != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(mPipelineLayout != VK_NULL_HANDLE);

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages {};
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = vertexShaderModule;
  shaderStages[0].pName = "main";
  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = fragmentShaderModule;
  shaderStages[1].pName = "main";

  VkPipelineVertexInputStateCreateInfo vertexInputState {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState {
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssemblyState.topology = convertPrimitiveTopology(desc.topology);

  VkPipelineViewportStateCreateInfo viewportState {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizationState {
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizationState.polygonMode = convertPolygonMode(desc.polygonMode);
  rasterizationState.cullMode = convertCullMode(desc.cullMode);
  rasterizationState.frontFace = convertFrontFace(desc.frontFace);
  rasterizationState.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisampleState {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDepthStencilStateCreateInfo depthStencilState {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depthStencilState.depthTestEnable = desc.depthTestEnable ? VK_TRUE : VK_FALSE;
  depthStencilState.depthWriteEnable = desc.depthWriteEnable ? VK_TRUE : VK_FALSE;
  depthStencilState.depthCompareOp = convertCompareOp(desc.depthCompareOp);

  VkPipelineColorBlendAttachmentState colorBlendAttachment {};
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendState {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlendState.attachmentCount = 1;
  colorBlendState.logicOpEnable = VK_FALSE;
  colorBlendState.logicOp = VK_LOGIC_OP_COPY;
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments = &colorBlendAttachment;
  colorBlendState.blendConstants[0] = 0.0f;
  colorBlendState.blendConstants[1] = 0.0f;
  colorBlendState.blendConstants[2] = 0.0f;
  colorBlendState.blendConstants[3] = 0.0f;

  std::array<VkDynamicState, 2> dynamicStates {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineRenderingCreateInfo renderingInfo {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
  const VkFormat colorAttachmentFormat = convertTextureFormat(desc.colorAttachmentFormat);
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachmentFormats = &colorAttachmentFormat;
  if (desc.hasDepthAttachment)
  {
    renderingInfo.depthAttachmentFormat = convertTextureFormat(desc.depthAttachmentFormat);
  }

  VkGraphicsPipelineCreateInfo createInfo {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  createInfo.pNext = &renderingInfo;
  createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  createInfo.pStages = shaderStages.data();
  createInfo.pVertexInputState = &vertexInputState;
  createInfo.pInputAssemblyState = &inputAssemblyState;
  createInfo.pViewportState = &viewportState;
  createInfo.pRasterizationState = &rasterizationState;
  createInfo.pMultisampleState = &multisampleState;
  createInfo.pDepthStencilState = desc.hasDepthAttachment ? &depthStencilState : nullptr;
  createInfo.pColorBlendState = &colorBlendState;
  createInfo.pDynamicState = &dynamicState;
  createInfo.layout = mPipelineLayout;
  createInfo.renderPass = VK_NULL_HANDLE;
  createInfo.subpass = 0;

  const VkResult vkResult =
    vkCreateGraphicsPipelines(vk::getDevice().getVirtualDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline);
  if (vkResult != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to create Vulkan graphics pipeline, error: {}", vkResultToString(vkResult));
    vkDestroyPipelineLayout(vk::getDevice().getVirtualDevice(), mPipelineLayout, nullptr);
    for (ResourceLayout& resourceLayout : mResourceLayouts)
    {
      resourceLayout.destroy();
    }
    mResourceLayouts.clear();
    mResourceLayoutDescs.clear();
    mPushConstantRanges.clear();
    mPipelineLayout = VK_NULL_HANDLE;
    return core::Result::eInitializationFailed;
  }

  mDesc = desc;
  mDesc.resourceLayoutDescs = mResourceLayoutDescs.data();
  mDesc.resourceLayoutDescCount = static_cast<uint32_t>(mResourceLayoutDescs.size());
  mDesc.pushConstantRanges = mPushConstantRanges.data();
  mDesc.pushConstantRangeCount = static_cast<uint32_t>(mPushConstantRanges.size());
  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::GraphicsPipeline::destroy()
{
  if (!mIsInitialized)
  {
    MENTAL_WARN("Trying to destroy an uninitialized vk::GraphicsPipeline");
    return;
  }

  vkDestroyPipelineLayout(vk::getDevice().getVirtualDevice(), mPipelineLayout, nullptr);
  vkDestroyPipeline(vk::getDevice().getVirtualDevice(), mPipeline, nullptr);
  for (ResourceLayout& resourceLayout : mResourceLayouts)
  {
    resourceLayout.destroy();
  }
  mResourceLayouts.clear();
  mResourceLayoutDescs.clear();
  mPushConstantRanges.clear();
  mDesc = {};
  mPipelineLayout = VK_NULL_HANDLE;
  mPipeline = VK_NULL_HANDLE;
  mIsInitialized = false;
}

bool mental::rhi::vk::GraphicsPipeline::isValid() const
{
  return mIsInitialized;
}

mental::core::resource::Object mental::rhi::vk::GraphicsPipeline::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkPipeline:
      return mPipeline;
    default:
      return nullptr;
  }
}

const mental::rhi::GraphicsPipelineDesc& mental::rhi::vk::GraphicsPipeline::getDesc() const
{
  return mDesc;
}

mental::core::resource::Object mental::rhi::vk::GraphicsPipeline::getPipelineLayoutNativeObject()
{
  return mPipelineLayout;
}

mental::rhi::IResourceLayout* mental::rhi::vk::GraphicsPipeline::getResourceLayout(uint32_t resourceSetIndex) const
{
  if (resourceSetIndex >= mResourceLayouts.size())
  {
    return nullptr;
  }

  return const_cast<ResourceLayout*>(&mResourceLayouts[resourceSetIndex]);
}

#include "vulkanPipeline.hpp"
#include "vulkanImage.hpp"
#include "vulkanShader.hpp"
#include "vulkanUtils.hpp"
#include <array>
#include <vector>

bool vkFramework::createPipelineLayout(VkDevice device,
                                       VkDescriptorSetLayout dsLayout,
                                       VkPipelineLayout* pipelineLayout) {
  const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 1,
      .pSetLayouts = &dsLayout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr};

  return (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                 pipelineLayout) == VK_SUCCESS);
}

bool vkFramework::createColorAndDepthRenderPass(VulkanRenderDevice& device,
                                                bool useDepth,
                                                VkRenderPass* renderPass,
                                                const RenderPassCreateInfo& ci,
                                                VkFormat colorFormat) {
  const bool offscreenInternal = ci.flags_ & RenderPassBit_OffscreenInternal;
  const bool offscreen = ci.flags_ & RenderPassBit_Offscreen;
  const bool first = ci.flags_ & RenderPassBit_First;
  const bool last = ci.flags_ & RenderPassBit_Last;

  VkAttachmentDescription colorAttachment = {
      .flags = 0,
      .format = colorFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = offscreenInternal
                    ? VK_ATTACHMENT_LOAD_OP_LOAD
                    : (ci.clearColor_ ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                      : VK_ATTACHMENT_LOAD_OP_LOAD),
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = first ? VK_IMAGE_LAYOUT_UNDEFINED
                             : (offscreenInternal
                                    ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                    : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
      .finalLayout = last ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                          : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  const VkAttachmentReference colorAttachmentRef = {
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
  if (useDepth) {
    depthFormat = findDepthFormat(device.physicalDevice);
  }

  if (depthFormat == VK_FORMAT_UNDEFINED) {
    return false;
  }

  VkAttachmentDescription depthAttachment = {
      .flags = 0,
      .format = depthFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = offscreenInternal
                    ? VK_ATTACHMENT_LOAD_OP_LOAD
                    : (ci.clearDepth_ ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                      : VK_ATTACHMENT_LOAD_OP_LOAD),
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout =
          ci.clearDepth_
              ? VK_IMAGE_LAYOUT_UNDEFINED
              : (offscreenInternal
                     ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                     : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  const VkAttachmentReference depthAttachmentRef = {
      .attachment = 1,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  if (offscreen) {
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }

  std::vector<VkSubpassDependency> dependencies = {VkSubpassDependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = 0}};

  if (offscreen) {
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Use subpass dependencies for layout transitions
    dependencies.resize(2);

    dependencies[0] = {.srcSubpass = VK_SUBPASS_EXTERNAL,
                       .dstSubpass = 0,
                       .srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       .dstStageMask =
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
                       .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                       .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};

    dependencies[1] = {.srcSubpass = 0,
                       .dstSubpass = VK_SUBPASS_EXTERNAL,
                       .srcStageMask =
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                       .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                       .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};
  }

  const VkSubpassDescription subpass = {
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = useDepth ? &depthAttachmentRef : nullptr,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr};

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment,
                                                        depthAttachment};

  const VkRenderPassCreateInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .attachmentCount = static_cast<uint32_t>(useDepth ? 2 : 1),
      .pAttachments = attachments.data(),
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = static_cast<uint32_t>(dependencies.size()),
      .pDependencies = dependencies.data()};

  return (vkCreateRenderPass(device.device, &renderPassInfo, nullptr,
                             renderPass) == VK_SUCCESS);
}

bool vkFramework::createGraphicsPipeline(
    VulkanRenderDevice& vkDev, VkRenderPass renderPass,
    VkPipelineLayout pipelineLayout,
    const std::vector<const char*>& shaderFiles, VkPipeline* pipeline,
    VkPrimitiveTopology topology, bool useDepth, bool useBlending,
    bool dynamicScissorState, bool dynamicViewportState, int32_t customWidth,
    int32_t customHeight, uint32_t numPatchControlPoints) {
  size_t numShaderFiles = shaderFiles.size();
  std::vector<ShaderModule> shaderModules(numShaderFiles);
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages(numShaderFiles);

  for (size_t i = 0; i < numShaderFiles; i++) {
    const char* file = shaderFiles[i];
    MENTAL_VK_CHECK(createShaderModule(vkDev.device, &shaderModules[i], file));
    shaderStages[i] = VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = shaderModules[i].stage,
        .module = shaderModules[i].shaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr};
  }

  const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = topology,
      .primitiveRestartEnable = VK_FALSE};

  const VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(
          customWidth > 0 ? customWidth : vkDev.swapchainExtent.width),
      .height = static_cast<float>(
          customHeight > 0 ? customHeight : vkDev.swapchainExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};

  const VkRect2D scissor = {
      .offset = {0, 0},
      .extent = {customWidth > 0 ? customWidth : vkDev.swapchainExtent.width,
                 customHeight > 0 ? customHeight
                                  : vkDev.swapchainExtent.height}};

  const VkPipelineViewportStateCreateInfo viewportState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor};

  const VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .lineWidth = 1.0f};

  const VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f};

  const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = useBlending ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                                         : VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

  const VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

  const VkPipelineDepthStencilStateCreateInfo depthStencil = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = static_cast<VkBool32>(useDepth ? VK_TRUE : VK_FALSE),
      .depthWriteEnable = static_cast<VkBool32>(useDepth ? VK_TRUE : VK_FALSE),
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .minDepthBounds = 0.0f,
      .maxDepthBounds = 1.0f};

  std::vector<VkDynamicState> dynamicStates;
  dynamicStates.reserve(2);

  if (dynamicScissorState) {
    dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
  }
  if (dynamicViewportState) {
    dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
  }

  bool hasDynamicStates = dynamicStates.size() > 0;
  const VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = hasDynamicStates ? dynamicStates.data() : nullptr};

  const VkPipelineTessellationStateCreateInfo tessellationState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .patchControlPoints = numPatchControlPoints};

  const VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pTessellationState = (topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
                                ? &tessellationState
                                : nullptr,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = useDepth ? &depthStencil : nullptr,
      .pColorBlendState = &colorBlending,
      .pDynamicState = hasDynamicStates ? &dynamicState : nullptr,
      .layout = pipelineLayout,
      .renderPass = renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1};

  MENTAL_VK_CHECK(vkCreateGraphicsPipelines(vkDev.device, VK_NULL_HANDLE, 1,
                                            &pipelineInfo, nullptr, pipeline));

  for (const ShaderModule& m : shaderModules) {
    vkDestroyShaderModule(vkDev.device, m.shaderModule, nullptr);
  }

  return true;
}

#include "vulkanPipeline.hpp"
#include "vulkanImage.hpp"
#include <array>
#include <vector>

bool mental::createPipelineLayout(VkDevice device,
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

bool mental::createColorAndDepthRenderPass(VulkanRenderDevice& device,
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

  VkAttachmentDescription depthAttachment = {
      .flags = 0,
      .format = useDepth ? findDepthFormat(device.physicalDevice)
                         : VK_FORMAT_D32_SFLOAT,
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

#include "gridLayer.hpp"
#include "vkFramework/includes.hpp"
#include <array>
#include <glm/glm.hpp>
#include <volk.h>

void vkFramework::render::GridLayer::init(const VulkanRenderDevice* vkDev,
                                          VulkanImage* depth) {
  CHECK_BOOL(depth != nullptr);
  BaseRenderLayer::init(vkDev, depth);

  CHECK_BOOL(createColorAndDepthRenderPass(*vkDev, true, &mRenderPass,
                                           RenderPassCreateInfo{}));
  CHECK_BOOL(createUniformBuffers(sizeof(UniformBuffer)));
  CHECK_BOOL(createFramebuffers());
  CHECK_BOOL(createDescriptorPool(*vkDev, 1, 0, 0, &mDescriptorPool));
  CHECK_BOOL(createDescriptorSet());
  CHECK_BOOL(createPipelineLayout(vkDev->device, mDescriptorSetLayout,
                                  &mPipelineLayout));
  CHECK_BOOL(createGraphicsPipeline(
      *vkDev, mRenderPass, mPipelineLayout,
      {"data/shaders/infiniteGrid.vert", "data/shaders/infiniteGrid.frag"},
      &mGraphicsPipeline));
}

void vkFramework::render::GridLayer::fillCommandBuffer(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  beginRenderPassDynamic(commandBuffer, currentFrame, currentImage);

  vkCmdDraw(commandBuffer, 6, 1, 0, 0);

  endRenderPass(commandBuffer);
}

void vkFramework::render::GridLayer::updateUniformBuffer(
    const glm::mat4& mvp, const glm::vec3& camPos, uint32_t currentFrame) {
  const UniformBuffer ubo = {.mvp = mvp, .camPos = camPos};

  uploadBufferData(*mRenderDevice, mUniformBuffersMemory[currentFrame], 0, &ubo,
                   sizeof(ubo));
}

bool vkFramework::render::GridLayer::createFramebuffers() {
  return BaseRenderLayer::createFramebuffers(mDepthTexture->imageView);
}

bool vkFramework::render::GridLayer::createDescriptorSet() {
  const std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
      descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT)};

  const VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data()};

  VK_CHECK(vkCreateDescriptorSetLayout(mRenderDevice->device, &layoutInfo,
                                       nullptr, &mDescriptorSetLayout));

  std::vector<VkDescriptorSetLayout> layouts(mRenderDevice->maxFramesInFlight,
                                             mDescriptorSetLayout);

  const VkDescriptorSetAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = mDescriptorPool,
      .descriptorSetCount = mRenderDevice->maxFramesInFlight,
      .pSetLayouts = layouts.data()};

  mDescriptorSets.resize(mRenderDevice->maxFramesInFlight);

  VK_CHECK(vkAllocateDescriptorSets(mRenderDevice->device, &allocInfo,
                                    mDescriptorSets.data()));

  for (size_t i = 0; i < mDescriptorSets.size(); i++) {
    VkDescriptorSet ds = mDescriptorSets[i];

    const VkDescriptorBufferInfo uniformBufferInfo = {mUniformBuffers[i], 0,
                                                      sizeof(UniformBuffer)};

    const std::array<VkWriteDescriptorSet, 1> descriptorWrites = {
        bufferWriteDescriptorSet(ds, &uniformBufferInfo, 0,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)};

    vkUpdateDescriptorSets(mRenderDevice->device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }

  return true;
}

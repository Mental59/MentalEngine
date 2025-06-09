#include "canvasLayer.hpp"
#include "vkFramework/includes.hpp"
#include <array>

vkFramework::render::VulkanCanvas::VulkanCanvas(VulkanRenderDevice& vkDev,
                                                VulkanImage depth)
    : BaseRenderLayer(vkDev, depth) {
  mLines.reserve(mMaxLinesCount / 8);
  mStorageBuffers.resize(vkDev.maxFramesInFlight);
  mStorageBuffersMemory.resize(vkDev.maxFramesInFlight);

  for (uint32_t i = 0; i < vkDev.maxFramesInFlight; i++) {
    CHECK_BOOL(createBuffer(vkDev.device, vkDev.physicalDevice,
                            mMaxLinesDataSize,
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            mStorageBuffers[i], mStorageBuffersMemory[i]));
  }

  CHECK_BOOL(
      createColorAndDepthRenderPass(vkDev, (depth.image != VK_NULL_HANDLE),
                                    &mRenderPass, RenderPassCreateInfo{}));

  CHECK_BOOL(createUniformBuffers(vkDev, sizeof(UniformBuffer)));

  CHECK_BOOL(createColorAndDepthFramebuffers(
      vkDev, mRenderPass, depth.imageView, mSwapchainFramebuffers));

  CHECK_BOOL(createDescriptorPool(vkDev, 1, 1, 0, &mDescriptorPool));

  CHECK_BOOL(createDescriptorSet(vkDev));

  CHECK_BOOL(createPipelineLayout(vkDev.device, mDescriptorSetLayout,
                                  &mPipelineLayout));

  CHECK_BOOL(createGraphicsPipeline(vkDev, mRenderPass, mPipelineLayout,
                                    {"data/shaders/chapter04/Lines.vert",
                                     "data/shaders/chapter04/Lines.frag"},
                                    &mGraphicsPipeline,
                                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                    (depth.image != VK_NULL_HANDLE), true));
}

vkFramework::render::VulkanCanvas::~VulkanCanvas() {
  for (size_t i = 0; i < mStorageBuffers.size(); i++) {
    vkDestroyBuffer(mDevice, mStorageBuffers[i], nullptr);
  }

  for (size_t i = 0; i < mStorageBuffersMemory.size(); i++) {
    vkFreeMemory(mDevice, mStorageBuffersMemory[i], nullptr);
  }
}

void vkFramework::render::VulkanCanvas::fillCommandBuffer(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  if (mLines.empty()) {
    return;
  }

  beginRenderPassDynamic(commandBuffer, currentFrame, currentImage);

  vkCmdDraw(commandBuffer, static_cast<uint32_t>(mLines.size()), 1, 0, 0);

  endRenderPass(commandBuffer);
}

void vkFramework::render::VulkanCanvas::clear() { mLines.clear(); }

void vkFramework::render::VulkanCanvas::line(const glm::vec3& p1,
                                             const glm::vec3& p2,
                                             const glm::vec4& c) {
  mLines.push_back({.position = p1, .color = c});
  mLines.push_back({.position = p2, .color = c});
}

void vkFramework::render::VulkanCanvas::plane3d(const glm::vec3& orig,
                                                const glm::vec3& v1,
                                                const glm::vec3& v2, int n1,
                                                int n2, float s1, float s2,
                                                const glm::vec4& color,
                                                const glm::vec4& outlineColor) {
  line(orig - s1 / 2.0f * v1 - s2 / 2.0f * v2,
       orig - s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
  line(orig + s1 / 2.0f * v1 - s2 / 2.0f * v2,
       orig + s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);

  line(orig - s1 / 2.0f * v1 + s2 / 2.0f * v2,
       orig + s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
  line(orig - s1 / 2.0f * v1 - s2 / 2.0f * v2,
       orig + s1 / 2.0f * v1 - s2 / 2.0f * v2, outlineColor);

  for (int i = 1; i < n1; i++) {
    float t = ((float)i - (float)n1 / 2.0f) * s1 / (float)n1;
    const glm::vec3 o1 = orig + t * v1;
    line(o1 - s2 / 2.0f * v2, o1 + s2 / 2.0f * v2, color);
  }

  for (int i = 1; i < n2; i++) {
    const float t = ((float)i - (float)n2 / 2.0f) * s2 / (float)n2;
    const glm::vec3 o2 = orig + t * v2;
    line(o2 - s1 / 2.0f * v1, o2 + s1 / 2.0f * v1, color);
  }
}

void vkFramework::render::VulkanCanvas::updateBuffer(VulkanRenderDevice& vkDev,
                                                     size_t currentFrame) {
  if (mLines.empty()) {
    return;
  }

  const VkDeviceSize bufferSize = mLines.size() * sizeof(VertexData);

  uploadBufferData(vkDev, mStorageBuffersMemory[currentFrame], 0, mLines.data(),
                   bufferSize);
}

void vkFramework::render::VulkanCanvas::updateUniformBuffer(
    VulkanRenderDevice& vkDev, const glm::mat4& modelViewProj, float time,
    uint32_t currentFrame) {
  const UniformBuffer ubo = {.mvp = modelViewProj, .time = time};

  uploadBufferData(vkDev, mUniformBuffersMemory[currentFrame], 0, &ubo,
                   sizeof(ubo));
}

bool vkFramework::render::VulkanCanvas::createDescriptorSet(
    VulkanRenderDevice& vkDev) {
  const std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
      descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT),
      descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT)};

  const VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data()};

  VK_CHECK(vkCreateDescriptorSetLayout(vkDev.device, &layoutInfo, nullptr,
                                       &mDescriptorSetLayout));

  std::vector<VkDescriptorSetLayout> layouts(vkDev.maxFramesInFlight,
                                             mDescriptorSetLayout);

  const VkDescriptorSetAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = mDescriptorPool,
      .descriptorSetCount = vkDev.maxFramesInFlight,
      .pSetLayouts = layouts.data()};

  mDescriptorSets.resize(vkDev.maxFramesInFlight);

  VK_CHECK(vkAllocateDescriptorSets(vkDev.device, &allocInfo,
                                    mDescriptorSets.data()));

  for (size_t i = 0; i < mDescriptorSets.size(); i++) {
    VkDescriptorSet ds = mDescriptorSets[i];

    const VkDescriptorBufferInfo uniformBufferInfo = {mUniformBuffers[i], 0,
                                                      sizeof(UniformBuffer)};
    const VkDescriptorBufferInfo storageBufferInfo = {mStorageBuffers[i], 0,
                                                      mMaxLinesDataSize};

    const std::array<VkWriteDescriptorSet, 2> descriptorWrites = {
        bufferWriteDescriptorSet(ds, &uniformBufferInfo, 0,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
        bufferWriteDescriptorSet(ds, &storageBufferInfo, 1,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)};

    vkUpdateDescriptorSets(vkDev.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }

  return true;
}

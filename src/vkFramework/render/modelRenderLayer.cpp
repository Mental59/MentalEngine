#include "modelRenderLayer.hpp"
#include "vkFramework/includes.hpp"
#include <array>
#include <volk.h>

vkFramework::render::ModelRenderLayer::ModelRenderLayer(
    VulkanRenderDevice& vkDev, const char* modelFile, const char* textureFile,
    uint32_t uniformDataSize)
    : BaseRenderLayer(vkDev, VulkanImage{}) {

  CHECK_BOOL(createTexturedVertexBuffer(vkDev, modelFile, &mStorageBuffer,
                                        &mStorageBufferMemory,
                                        &mVertexBufferSize, &mIndexBufferSize));

  CHECK_BOOL(createVulkanImage(vkDev, textureFile, mTexture));

  CHECK_BOOL(createDepthResources(vkDev, vkDev.swapchainExtent.width,
                                  vkDev.swapchainExtent.height, mDepthTexture));

  CHECK_BOOL(createColorAndDepthRenderPass(vkDev, true, &mRenderPass,
                                           RenderPassCreateInfo{}));

  CHECK_BOOL(createUniformBuffers(vkDev, uniformDataSize));

  CHECK_BOOL(createColorAndDepthFramebuffers(
      vkDev, mRenderPass, mDepthTexture.imageView, mSwapchainFramebuffers));

  CHECK_BOOL(createDescriptorPool(vkDev, 1, 2, 1, &mDescriptorPool));

  CHECK_BOOL(createDescriptorSet(vkDev, uniformDataSize));

  CHECK_BOOL(createPipelineLayout(vkDev.device, mDescriptorSetLayout,
                                  &mPipelineLayout));

  CHECK_BOOL(createGraphicsPipeline(vkDev, mRenderPass, mPipelineLayout,
                                    {"data/shaders/chapter03/VK02.vert",
                                     "data/shaders/chapter03/VK02.frag",
                                     "data/shaders/chapter03/VK02.geom"},
                                    &mGraphicsPipeline));
}

vkFramework::render::ModelRenderLayer::ModelRenderLayer(
    VulkanRenderDevice& vkDev, bool useDepth, VkBuffer storageBuffer,
    VkDeviceMemory storageBufferMemory, uint32_t vertexBufferSize,
    uint32_t indexBufferSize, VulkanImage texture,
    const std::vector<const char*>& shaderFiles, uint32_t uniformDataSize,
    bool useGeneralTextureLayout, VulkanImage externalDepth,
    bool deleteMeshData)
    : mUseGeneralTextureLayout(useGeneralTextureLayout),
      mVertexBufferSize(vertexBufferSize), mIndexBufferSize(indexBufferSize),
      mStorageBuffer(storageBuffer), mStorageBufferMemory(storageBufferMemory),
      mTexture(texture), mDeleteMeshData(deleteMeshData),
      BaseRenderLayer(vkDev, VulkanImage{}) {
  if (useDepth) {
    mIsExternalDepth = (externalDepth.image != VK_NULL_HANDLE);

    if (mIsExternalDepth) {
      mDepthTexture = externalDepth;
    } else {
      CHECK_BOOL(createDepthResources(vkDev, vkDev.swapchainExtent.width,
                                      vkDev.swapchainExtent.height,
                                      mDepthTexture));
    }
  }

  CHECK_BOOL(createColorAndDepthRenderPass(vkDev, useDepth, &mRenderPass,
                                           RenderPassCreateInfo{}));

  CHECK_BOOL(createUniformBuffers(vkDev, uniformDataSize));

  CHECK_BOOL(!createColorAndDepthFramebuffers(
      vkDev, mRenderPass, mDepthTexture.imageView, mSwapchainFramebuffers));

  CHECK_BOOL(createDescriptorPool(vkDev, 1, 2, 1, &mDescriptorPool));

  CHECK_BOOL(createDescriptorSet(vkDev, uniformDataSize));

  CHECK_BOOL(createPipelineLayout(vkDev.device, mDescriptorSetLayout,
                                  &mPipelineLayout));

  CHECK_BOOL(createGraphicsPipeline(vkDev, mRenderPass, mPipelineLayout,
                                    shaderFiles, &mGraphicsPipeline));
}

vkFramework::render::ModelRenderLayer::~ModelRenderLayer() {
  if (mDeleteMeshData) {
    vkDestroyBuffer(mDevice, mStorageBuffer, nullptr);
    vkFreeMemory(mDevice, mStorageBufferMemory, nullptr);
  }

  if (mTexture.sampler != VK_NULL_HANDLE) {
    destroyVulkanImage(mDevice, mTexture);
  }

  if (!mIsExternalDepth) {
    destroyVulkanImage(mDevice, mDepthTexture);
  }
}

void vkFramework::render::ModelRenderLayer::fillCommandBuffer(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  beginRenderPass(commandBuffer, currentFrame, currentImage);

  uint32_t vertexCount =
      static_cast<uint32_t>(mIndexBufferSize / (sizeof(unsigned int)));
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);

  vkCmdEndRenderPass(commandBuffer);
}

void vkFramework::render::ModelRenderLayer::updateUniformBuffer(
    VulkanRenderDevice& vkDev, uint32_t currentFrame, const void* data,
    const size_t dataSize) {
  uploadBufferData(vkDev, mUniformBuffersMemory[currentFrame], 0, data,
                   dataSize);
}

// HACK to allow sharing textures between multiple ModelRenderers
void vkFramework::render::ModelRenderLayer::freeTextureSampler() {
  mTexture.sampler = VK_NULL_HANDLE;
}

bool vkFramework::render::ModelRenderLayer::createDescriptorSet(
    VulkanRenderDevice& vkDev, uint32_t uniformDataSize) {
  const std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
      descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT),
      descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT),
      descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT),
      descriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 VK_SHADER_STAGE_FRAGMENT_BIT)};

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
                                                      uniformDataSize};
    const VkDescriptorBufferInfo vertexBufferInfo = {mStorageBuffer, 0,
                                                     mVertexBufferSize};
    const VkDescriptorBufferInfo indexBufferInfo = {
        mStorageBuffer, mVertexBufferSize, mIndexBufferSize};
    const VkDescriptorImageInfo imageInfo = {
        mTexture.sampler, mTexture.imageView,
        mUseGeneralTextureLayout ? VK_IMAGE_LAYOUT_GENERAL
                                 : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    const std::array<VkWriteDescriptorSet, 4> descriptorWrites = {
        bufferWriteDescriptorSet(ds, &uniformBufferInfo, 0,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
        bufferWriteDescriptorSet(ds, &vertexBufferInfo, 1,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
        bufferWriteDescriptorSet(ds, &indexBufferInfo, 2,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
        imageWriteDescriptorSet(ds, &imageInfo, 3)};

    vkUpdateDescriptorSets(vkDev.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }

  return true;
}

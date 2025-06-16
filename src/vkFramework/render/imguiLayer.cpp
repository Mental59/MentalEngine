#include "imguiLayer.hpp"
#include "imgui.h"
#include "vkFramework/includes.hpp"
#include <array>
#include <glm/ext.hpp>

namespace {
constexpr uint32_t gImGuiVtxBufferSize = 512 * 1024 * sizeof(ImDrawVert);
constexpr uint32_t gImGuiIdxBufferSize = 512 * 1024 * sizeof(uint32_t);
const char* gDefaultFont = "data/OpenSans-Light.ttf";

bool createFontTexture(ImGuiIO& io, const char* fontFile,
                       vkFramework::VulkanRenderDevice& vkDev,
                       VkImage& textureImage,
                       VkDeviceMemory& textureImageMemory) {
  ImFontConfig cfg = ImFontConfig();
  cfg.FontDataOwnedByAtlas = false;
  cfg.RasterizerMultiply = 1.5f;
  cfg.SizePixels = 24.0f;
  cfg.PixelSnapH = true;
  cfg.OversampleH = 4;
  cfg.OversampleV = 4;

  ImFont* font = io.Fonts->AddFontFromFileTTF(fontFile, cfg.SizePixels, &cfg);

  unsigned char* pixels = nullptr;
  int texWidth = 1;
  int texHeight = 1;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &texWidth, &texHeight);

  if (!pixels || !createTextureImageFromData(
                     vkDev, textureImage, textureImageMemory, pixels, texWidth,
                     texHeight, VK_FORMAT_R8G8B8A8_UNORM)) {
    return false;
  }

  io.Fonts->TexID = (ImTextureID)0;
  io.FontDefault = font;
  io.DisplayFramebufferScale = ImVec2(1, 1);

  return true;
}

void addImGuiItem(uint32_t width, uint32_t height,
                  VkCommandBuffer commandBuffer, const ImDrawCmd* pCmd,
                  ImVec2 clipOff, ImVec2 clipScale, int idxOffset,
                  int vtxOffset,
                  const std::vector<vkFramework::VulkanTexture>& textures,
                  VkPipelineLayout pipelineLayout) {
  if (pCmd->UserCallback)
    return;

  // Project scissor/clipping rectangles into framebuffer space
  ImVec4 clipRect;
  clipRect.x = (pCmd->ClipRect.x - clipOff.x) * clipScale.x;
  clipRect.y = (pCmd->ClipRect.y - clipOff.y) * clipScale.y;
  clipRect.z = (pCmd->ClipRect.z - clipOff.x) * clipScale.x;
  clipRect.w = (pCmd->ClipRect.w - clipOff.y) * clipScale.y;

  if (clipRect.x < width && clipRect.y < height && clipRect.z >= 0.0f &&
      clipRect.w >= 0.0f) {

    if (clipRect.x < 0.0f) {
      clipRect.x = 0.0f;
    }

    if (clipRect.y < 0.0f) {
      clipRect.y = 0.0f;
    }

    // Apply scissor/clipping rectangle
    const VkRect2D scissor = {
        .offset = {.x = (int32_t)(clipRect.x), .y = (int32_t)(clipRect.y)},
        .extent = {.width = (uint32_t)(clipRect.z - clipRect.x),
                   .height = (uint32_t)(clipRect.w - clipRect.y)}};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    if (textures.size() > 0) {
      uint32_t texture = (uint32_t)(intptr_t)pCmd->TextureId;
      vkCmdPushConstants(commandBuffer, pipelineLayout,
                         VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t),
                         (const void*)&texture);
    }

    vkCmdDraw(commandBuffer, pCmd->ElemCount, 1, pCmd->IdxOffset + idxOffset,
              pCmd->VtxOffset + vtxOffset);
  }
}
} // namespace

vkFramework::render::ImGuiLayer::ImGuiLayer(VulkanRenderDevice& vkDev)
    : BaseRenderLayer(vkDev, VulkanImage{}) {
  createFontTexture(ImGui::GetIO(), gDefaultFont, vkDev, mFont.image,
                    mFont.imageMemory);

  createImageView(vkDev.device, mFont.image, VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_ASPECT_COLOR_BIT, &mFont.imageView);
  createTextureSampler(vkDev.device, &mFont.sampler);

  // Buffer allocation
  mStorageBuffer.resize(vkDev.maxFramesInFlight);
  mStorageBufferMemory.resize(vkDev.maxFramesInFlight);

  mBufferSize = gImGuiVtxBufferSize + gImGuiIdxBufferSize;

  for (size_t i = 0; i < vkDev.maxFramesInFlight; i++) {
    VK_CHECK(createBuffer(vkDev.device, vkDev.physicalDevice, mBufferSize,
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          mStorageBuffer[i], mStorageBufferMemory[i]));
  }

  VK_CHECK(createColorAndDepthRenderPass(vkDev, false, &mRenderPass,
                                         RenderPassCreateInfo{}));

  VK_CHECK(createColorAndDepthFramebuffers(vkDev, mRenderPass, VK_NULL_HANDLE,
                                           mSwapchainFramebuffers));

  VK_CHECK(createUniformBuffers(vkDev, sizeof(glm::mat4)));

  VK_CHECK(createDescriptorPool(vkDev, 1, 2, 1, &mDescriptorPool));

  VK_CHECK(createDescriptorSet(vkDev));

  VK_CHECK(createPipelineLayout(vkDev.device, mDescriptorSetLayout,
                                &mPipelineLayout));

  VK_CHECK(createGraphicsPipeline(vkDev, mRenderPass, mPipelineLayout,
                                  {"data/shaders/chapter07/VK02_ImGui.vert",
                                   "data/shaders/chapter07/VK02_ImGui.frag"},
                                  &mGraphicsPipeline));
}

vkFramework::render::ImGuiLayer::ImGuiLayer(
    VulkanRenderDevice& vkDev, const std::vector<VulkanTexture>& textures)
    : BaseRenderLayer(vkDev, VulkanImage{}), mExtTextures(textures) {
  createFontTexture(ImGui::GetIO(), gDefaultFont, vkDev, mFont.image,
                    mFont.imageMemory);

  createImageView(vkDev.device, mFont.image, VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_ASPECT_COLOR_BIT, &mFont.imageView);
  createTextureSampler(vkDev.device, &mFont.sampler);

  mStorageBuffer.resize(vkDev.maxFramesInFlight);
  mStorageBufferMemory.resize(vkDev.maxFramesInFlight);

  mBufferSize = gImGuiVtxBufferSize + gImGuiIdxBufferSize;

  for (size_t i = 0; i < vkDev.maxFramesInFlight; i++) {
    VK_CHECK(createBuffer(vkDev.device, vkDev.physicalDevice, mBufferSize,
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          mStorageBuffer[i], mStorageBufferMemory[i]));
  }

  VK_CHECK(createColorAndDepthRenderPass(vkDev, false, &mRenderPass,
                                         RenderPassCreateInfo{}));

  VK_CHECK(createColorAndDepthFramebuffers(vkDev, mRenderPass, VK_NULL_HANDLE,
                                           mSwapchainFramebuffers));

  VK_CHECK(createUniformBuffers(vkDev, sizeof(glm::mat4)));

  VK_CHECK(
      createDescriptorPool(vkDev, 1, 2, 1 + textures.size(), &mDescriptorPool));

  VK_CHECK(createMultiDescriptorSet(vkDev));

  VK_CHECK(createPipelineLayoutWithConstants(vkDev.device, mDescriptorSetLayout,
                                             0, sizeof(uint32_t),
                                             &mPipelineLayout));

  VK_CHECK(createGraphicsPipeline(vkDev, mRenderPass, mPipelineLayout,
                                  {"data/shaders/chapter04/imgui.vert",
                                   "data/shaders/chapter06/imgui_multi.frag"},
                                  &mGraphicsPipeline));
}

vkFramework::render::ImGuiLayer::~ImGuiLayer() {
  for (VkBuffer buffer : mStorageBuffer) {
    vkDestroyBuffer(mDevice, buffer, nullptr);
  }

  for (VkDeviceMemory memory : mStorageBufferMemory) {
    vkFreeMemory(mDevice, memory, nullptr);
  }

  destroyVulkanImage(mDevice, mFont);
}

void vkFramework::render::ImGuiLayer::fillCommandBuffer(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  beginRenderPass(commandBuffer, currentFrame, currentImage);
  cmdSetViewport(commandBuffer);

  int vtxOffset = 0;
  int idxOffset = 0;

  for (int i = 0; i < mDrawData->CmdLists.Size; i++) {
    const ImDrawList* cmdList = mDrawData->CmdLists[i];

    for (int j = 0; j < cmdList->CmdBuffer.Size; j++) {
      const ImDrawCmd* cmd = &cmdList->CmdBuffer[j];

      addImGuiItem(mFramebufferExtent.width, mFramebufferExtent.height,
                   commandBuffer, cmd, mDrawData->DisplayPos,
                   mDrawData->FramebufferScale, idxOffset, vtxOffset,
                   mExtTextures, mPipelineLayout);
    }

    vtxOffset += cmdList->VtxBuffer.Size;
    idxOffset += cmdList->IdxBuffer.Size;
  }

  endRenderPass(commandBuffer);
}

void vkFramework::render::ImGuiLayer::updateBuffers(
    VulkanRenderDevice& vkDev, uint32_t currentFrame,
    const ImDrawData* imguiDrawData) {
  mDrawData = imguiDrawData;

  const float left = mDrawData->DisplayPos.x;
  const float right = mDrawData->DisplayPos.x + mDrawData->DisplaySize.x;
  const float bottom = mDrawData->DisplayPos.y;
  const float top = mDrawData->DisplayPos.y + mDrawData->DisplaySize.y;

  const glm::mat4 inMtx = glm::ortho(left, right, bottom, top);

  uploadBufferData(vkDev, mUniformBuffersMemory[currentFrame], 0,
                   glm::value_ptr(inMtx), sizeof(glm::mat4));

  void* data = nullptr;
  vkMapMemory(vkDev.device, mStorageBufferMemory[currentFrame], 0, mBufferSize,
              0, &data);

  ImDrawVert* vtx = (ImDrawVert*)data;
  for (int i = 0; i < mDrawData->CmdListsCount; i++) {
    const ImDrawList* cmdList = mDrawData->CmdLists[i];

    memcpy(vtx, cmdList->VtxBuffer.Data,
           cmdList->VtxBuffer.Size * sizeof(ImDrawVert));

    vtx += cmdList->VtxBuffer.Size;
  }

  uint32_t* idx = (uint32_t*)((uint8_t*)data + gImGuiVtxBufferSize);
  for (int i = 0; i < mDrawData->CmdListsCount; i++) {
    const ImDrawList* cmdList = mDrawData->CmdLists[i];

    const uint16_t* src = (const uint16_t*)cmdList->IdxBuffer.Data;
    for (int j = 0; j < cmdList->IdxBuffer.Size; j++) {
      *idx++ = (uint32_t)*src++;
    }
  }

  vkUnmapMemory(vkDev.device, mStorageBufferMemory[currentFrame]);
}

bool vkFramework::render::ImGuiLayer::createDescriptorSet(
    VulkanRenderDevice& vkDev) {
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
    const VkDescriptorBufferInfo uniformBufferInfo = {
        .buffer = mUniformBuffers[i], .offset = 0, .range = sizeof(glm::mat4)};

    const VkDescriptorBufferInfo vertexBufferInfo = {
        .buffer = mStorageBuffer[i], .offset = 0, .range = gImGuiVtxBufferSize};

    const VkDescriptorBufferInfo indexBufferInfo = {
        .buffer = mStorageBuffer[i],
        .offset = gImGuiVtxBufferSize,
        .range = gImGuiIdxBufferSize};

    const VkDescriptorImageInfo imageInfo = {
        .sampler = mFont.sampler,
        .imageView = mFont.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

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

bool vkFramework::render::ImGuiLayer::createMultiDescriptorSet(
    VulkanRenderDevice& vkDev) {
  const std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
      descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT),
      descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT),
      descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT),
      descriptorSetLayoutBinding(
          3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          static_cast<uint32_t>(1 + mExtTextures.size()))};

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

  // use the font texture initialized in the constructor
  std::vector<VkDescriptorImageInfo> textureDescriptors = {
      {mFont.sampler, mFont.imageView,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};

  for (size_t i = 0; i < mExtTextures.size(); i++) {
    textureDescriptors.push_back({
        .sampler = mExtTextures[i].sampler,
        .imageView = mExtTextures[i].image.imageView,
        .imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // TODO: select type from
                                                     // VulkanTexture object
                                                     // (GENERAL or
                                                     // SHADER_READ_ONLY_OPTIMAL)
    });
  }

  for (size_t i = 0; i < mDescriptorSets.size(); i++) {
    VkDescriptorSet ds = mDescriptorSets[i];

    const VkDescriptorBufferInfo uniformBufferInfo = {mUniformBuffers[i], 0,
                                                      sizeof(glm::mat4)};
    const VkDescriptorBufferInfo vertexBufferInfo = {mStorageBuffer[i], 0,
                                                     gImGuiVtxBufferSize};
    const VkDescriptorBufferInfo indexBufferInfo = {
        mStorageBuffer[i], gImGuiVtxBufferSize, gImGuiIdxBufferSize};

    const std::array<VkWriteDescriptorSet, 4> descriptorWrites = {
        bufferWriteDescriptorSet(ds, &uniformBufferInfo, 0,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
        bufferWriteDescriptorSet(ds, &vertexBufferInfo, 1,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
        bufferWriteDescriptorSet(ds, &indexBufferInfo, 2,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
        VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = ds,
            .dstBinding = 3,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<uint32_t>(1 + mExtTextures.size()),
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = textureDescriptors.data()},
    };

    vkUpdateDescriptorSets(vkDev.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }

  return true;
}

#include "vulkanMemory.hpp"
#include "vulkanCommand.hpp"
#include "vulkanUtils.hpp"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <vector>

uint32_t vkFramework::findMemoryType(VkPhysicalDevice device,
                                     uint32_t typeFilter,
                                     VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  return 0xFFFFFFFF;
}

bool vkFramework::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                               VkDeviceSize size, VkBufferUsageFlags usage,
                               VkMemoryPropertyFlags properties,
                               VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
  const VkBufferCreateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr};

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    return false;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  const VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = findMemoryType(
          physicalDevice, memRequirements.memoryTypeBits, properties)};

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS) {
    vkDestroyBuffer(device, buffer, nullptr);
    return false;
  }

  vkBindBufferMemory(device, buffer, bufferMemory, 0);

  return true;
}

void vkFramework::copyBuffer(VulkanRenderDevice& vkDev, VkBuffer srcBuffer,
                             VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(vkDev);

  const VkBufferCopy copyRegion = {
      .srcOffset = 0, .dstOffset = 0, .size = size};

  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(vkDev, commandBuffer);
}

bool vkFramework::createTexturedVertexBuffer(
    VulkanRenderDevice& vkDev, const char* filename, VkBuffer* storageBuffer,
    VkDeviceMemory* storageBufferMemory, size_t* vertexBufferSize,
    size_t* indexBufferSize) {
  const aiScene* scene = aiImportFile(filename, aiProcess_Triangulate);

  if (!scene || !scene->HasMeshes()) {
    return false;
  }

  const aiMesh* mesh = scene->mMeshes[0];

  std::vector<VertexData> vertices;
  vertices.reserve(mesh->mNumVertices);
  for (unsigned i = 0; i != mesh->mNumVertices; i++) {
    const aiVector3D v = mesh->mVertices[i];
    const aiVector3D t = mesh->mTextureCoords[0][i];
    vertices.push_back(
        {.pos = glm::vec3(v.x, v.z, v.y), .tc = glm::vec2(t.x, 1.0f - t.y)});
  }

  std::vector<unsigned int> indices;
  indices.reserve(mesh->mNumFaces * 3);
  for (unsigned i = 0; i != mesh->mNumFaces; i++) {
    for (unsigned j = 0; j != 3; j++) {
      indices.push_back(mesh->mFaces[i].mIndices[j]);
    }
  }
  aiReleaseImport(scene);

  *vertexBufferSize = sizeof(VertexData) * vertices.size();
  *indexBufferSize = sizeof(unsigned int) * indices.size();

  size_t allocationSize = allocateVertexBuffer(
      vkDev, storageBuffer, storageBufferMemory, *vertexBufferSize,
      vertices.data(), *indexBufferSize, indices.data());

  return allocationSize != 0;
}

size_t vkFramework::allocateVertexBuffer(
    VulkanRenderDevice& vkDev, VkBuffer* storageBuffer,
    VkDeviceMemory* storageBufferMemory, size_t vertexDataSize,
    const void* vertexData, size_t indexDataSize, const void* indexData) {
  VkDeviceSize bufferSize = vertexDataSize + indexDataSize;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  bool isStagingBufferCreated =
      createBuffer(vkDev.device, vkDev.physicalDevice, bufferSize,
                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   stagingBuffer, stagingBufferMemory);
  if (!isStagingBufferCreated) {
    return 0;
  }

  void* data;
  vkMapMemory(vkDev.device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertexData, vertexDataSize);
  memcpy((unsigned char*)data + vertexDataSize, indexData, indexDataSize);
  vkUnmapMemory(vkDev.device, stagingBufferMemory);

  bool isFinalBufferCreated = createBuffer(
      vkDev.device, vkDev.physicalDevice, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *storageBuffer,
      *storageBufferMemory);
  if (!isFinalBufferCreated) {
    vkDestroyBuffer(vkDev.device, stagingBuffer, nullptr);
    vkFreeMemory(vkDev.device, stagingBufferMemory, nullptr);
    return 0;
  }

  copyBuffer(vkDev, stagingBuffer, *storageBuffer, bufferSize);

  vkDestroyBuffer(vkDev.device, stagingBuffer, nullptr);
  vkFreeMemory(vkDev.device, stagingBufferMemory, nullptr);

  return bufferSize;
}

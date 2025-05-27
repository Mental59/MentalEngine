#pragma once

#include "baseRenderLayer.hpp"

#include <glm/glm.hpp>

namespace vkFramework::render {

class VulkanCanvas : public BaseRenderLayer {
public:
  explicit VulkanCanvas(VulkanRenderDevice& vkDev, VulkanImage depth);
  virtual ~VulkanCanvas();

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) override;

  void clear();
  void line(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& c);
  void plane3d(const glm::vec3& orig, const glm::vec3& v1, const glm::vec3& v2,
               int n1, int n2, float s1, float s2, const glm::vec4& color,
               const glm::vec4& outlineColor);
  void updateBuffer(VulkanRenderDevice& vkDev, size_t currentFrame);
  void updateUniformBuffer(VulkanRenderDevice& vkDev,
                           const glm::mat4& modelViewProj, float time,
                           uint32_t currentFrame);

private:
  struct VertexData {
    glm::vec3 position;
    glm::vec4 color;
  };

  struct UniformBuffer {
    glm::mat4 mvp;
    float time;
  };

  bool createDescriptorSet(VulkanRenderDevice& vkDev);

  std::vector<VertexData> mLines;

  std::vector<VkBuffer> mStorageBuffers;
  std::vector<VkDeviceMemory> mStorageBuffersMemory;

  static constexpr unsigned int mMaxLinesCount = 65536;
  static constexpr unsigned int mMaxLinesDataSize =
      mMaxLinesCount * sizeof(VertexData) * 2;
};
} // namespace vkFramework::render

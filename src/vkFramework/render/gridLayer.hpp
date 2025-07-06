#pragma once

#include "baseRenderLayer.hpp"
#include <glm/glm.hpp>

namespace vkFramework {
struct VulkanRenderDevice;
struct VulkanImage;
} // namespace vkFramework

namespace vkFramework::render {
class GridLayer : public BaseRenderLayer {
public:
  explicit GridLayer() = default;

  void init(const VulkanRenderDevice* vkDev, VulkanImage* depth);

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) override;

  void updateUniformBuffer(const glm::mat4& mvp, const glm::vec3& camPos,
                           uint32_t currentFrame);

  virtual bool createFramebuffers() override;

private:
  struct UniformBuffer {
    alignas(16) glm::mat4 mvp;
    alignas(16) glm::vec3 camPos;
  };

  bool createDescriptorSet();
};
} // namespace vkFramework::render

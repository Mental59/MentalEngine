#pragma once

#include <render/primitiveMeshLibrary.hpp>
#include <render/rhi/rhi.hpp>

#include <glm/glm.hpp>

#include <array>
#include <filesystem>
#include <memory>
#include <vector>

namespace mental::render
{
struct alignas(16) PrimitiveDrawPushConstants
{
  glm::mat4 worldTransform {1.0f};
  glm::mat4 normalMatrix {1.0f};
  std::uint32_t vertexOffsetBytes = 0u;
  std::uint32_t indexOffsetBytes = 0u;
  std::uint32_t padding0 = 0u;
  std::uint32_t padding1 = 0u;
};

struct alignas(16) GridDrawPushConstants
{
  float gridSize = 100.0f;
  float gridCellSize = 0.025f;
  float gridMinPixelsBetweenCells = 2.0f;
  float padding0 = 0.0f;
  glm::vec4 gridColorThin {0.4f, 0.4f, 0.4f, 1.0f};
  glm::vec4 gridColorThick {0.05f, 0.05f, 0.05f, 1.0f};
};

struct ScenePipelineLibraryConfig
{
  std::filesystem::path shaderRootPath {};
  std::uint32_t framesInFlight = 0u;
  rhi::TextureFormat colorAttachmentFormat = rhi::TextureFormat::eBGRA32_SRGB;
  rhi::TextureFormat depthAttachmentFormat = rhi::TextureFormat::eD32_SFLOAT;
  rhi::IBuffer* geometryStorageBuffer = nullptr;
};

[[nodiscard]] std::array<rhi::ResourceBindingDesc, 2> buildSceneResourceBindings();
[[nodiscard]] rhi::PushConstantRangeDesc buildScenePushConstantRange();

class ScenePipelineLibrary
{
 public:
  [[nodiscard]] core::Result init(const ScenePipelineLibraryConfig& config);
  void destroy();
  [[nodiscard]] bool isValid() const noexcept;

  [[nodiscard]] core::Result updateFrameResourceSet(std::uint32_t frameIndex, rhi::IBuffer* cameraBuffer);
  [[nodiscard]] core::Result recordGridDraw(rhi::ICommandList* cmdList, std::uint32_t frameIndex) const;
  [[nodiscard]] core::Result recordPrimitiveDraw(rhi::ICommandList* cmdList,
    std::uint32_t frameIndex,
    const PrimitiveMeshView& primitiveMeshView,
    const SceneRenderObject& renderObject) const;

 private:
  [[nodiscard]] core::Result createShaders(const std::filesystem::path& shaderRootPath);
  [[nodiscard]] core::Result createSceneResourceSets();
  [[nodiscard]] core::Result createPipelines();
  [[nodiscard]] bool isFrameIndexValid(std::uint32_t frameIndex) const noexcept;

  ScenePipelineLibraryConfig mConfig {};
  std::vector<std::unique_ptr<rhi::IResourceSet>> mSceneResourceSets {};
  std::unique_ptr<rhi::IShaderModule> mPrimitiveVertexShader {};
  std::unique_ptr<rhi::IShaderModule> mPrimitiveFragmentShader {};
  std::unique_ptr<rhi::IShaderModule> mGridVertexShader {};
  std::unique_ptr<rhi::IShaderModule> mGridFragmentShader {};
  std::unique_ptr<rhi::IGraphicsPipeline> mPrimitivePipeline {};
  std::unique_ptr<rhi::IGraphicsPipeline> mGridPipeline {};
  bool mIsInitialized = false;
};
} // namespace mental::render

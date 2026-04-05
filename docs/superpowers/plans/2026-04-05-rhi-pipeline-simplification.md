# RHI Pipeline Simplification Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Simplify renderer-facing graphics-pipeline creation so render code no longer manually creates pipeline layouts or layout-derived resource sets, while keeping Vulkan descriptor sets and pipeline layouts internal to the backend.

**Architecture:** Move the remaining layout boilerplate behind `IGraphicsPipeline` and pipeline-owned resource binding contracts. `GraphicsPipelineDesc` should describe shaders, fixed-function state, inline resource-layout descriptions, and push-constant ranges in one place; the Vulkan graphics pipeline implementation should then create and own its internal `VkPipelineLayout`. Resource sets should be allocated against a graphics pipeline plus set index instead of a separate public `IPipelineLayout`, and `ScenePipelineLibrary` should become a thin consumer that creates two pipelines and per-frame resource sets without stitching layout objects together manually.

**Tech Stack:** C++20, Vulkan 1.3 dynamic rendering, existing RHI/device layer, Slang shader modules, GLM, CTest

---

## Current Constraints From The Codebase

- [`engine/include/render/rhi/rhi.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/rhi.hpp) still exposes `IPipelineLayout` publicly even after resource layouts were renamed and inline resource layout descriptions were added.
- [`engine/src/render/rhi/vulkan/graphicsPipeline.cpp`](D:/Proj/MentalEngine/engine/src/render/rhi/vulkan/graphicsPipeline.cpp) already owns the mechanics for building `VkPipelineLayout`, but renderer code must still create a pipeline layout object separately and pass it into each graphics pipeline.
- [`engine/src/render/rhi/vulkan/resourceSet.cpp`](D:/Proj/MentalEngine/engine/src/render/rhi/vulkan/resourceSet.cpp) allocates Vulkan descriptor sets from a pipeline layout plus set index, which proves the backend already has enough information to hide layout objects from renderer-facing code.
- [`engine/src/render/scenePipelineLibrary.cpp`](D:/Proj/MentalEngine/engine/src/render/scenePipelineLibrary.cpp) still has explicit pipeline-setup staging:
  - compile shaders
  - create a scene pipeline layout
  - create primitive and grid pipelines against that layout
  - create per-frame resource sets against that layout
- The current scene shaders already define the shared scene binding contract in one place:
  - camera uniform buffer at binding `0`
  - primitive geometry storage buffer at binding `1`
  That makes them a good fit for a pipeline-owned resource layout contract.
- [`engine/tests/shaderCompilerTests.cpp`](D:/Proj/MentalEngine/engine/tests/shaderCompilerTests.cpp) is already the lightweight contract-test location for this pipeline layer and should continue proving the public API shape without needing a live Vulkan device.

## File Structure

**Modify**
- [`engine/include/render/rhi/rhi.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/rhi.hpp)
- [`engine/include/render/rhi/vulkan/device.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/vulkan/device.hpp)
- [`engine/src/render/rhi/vulkan/device.cpp`](D:/Proj/MentalEngine/engine/src/render/rhi/vulkan/device.cpp)
- [`engine/include/render/rhi/vulkan/graphicsPipeline.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/vulkan/graphicsPipeline.hpp)
- [`engine/src/render/rhi/vulkan/graphicsPipeline.cpp`](D:/Proj/MentalEngine/engine/src/render/rhi/vulkan/graphicsPipeline.cpp)
- [`engine/include/render/rhi/vulkan/resourceSet.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/vulkan/resourceSet.hpp)
- [`engine/src/render/rhi/vulkan/resourceSet.cpp`](D:/Proj/MentalEngine/engine/src/render/rhi/vulkan/resourceSet.cpp)
- [`engine/include/render/scenePipelineLibrary.hpp`](D:/Proj/MentalEngine/engine/include/render/scenePipelineLibrary.hpp)
- [`engine/src/render/scenePipelineLibrary.cpp`](D:/Proj/MentalEngine/engine/src/render/scenePipelineLibrary.cpp)
- [`engine/tests/shaderCompilerTests.cpp`](D:/Proj/MentalEngine/engine/tests/shaderCompilerTests.cpp)

**Responsibility Split**
- `rhi.hpp`: renderer-facing API contract. After this plan, pipeline creation should be one public object creation step instead of `createPipelineLayout()` plus `createGraphicsPipeline()`.
- `graphicsPipeline.*`: owns backend pipeline layout lifetime internally and exposes only the information render code still legitimately needs.
- `resourceSet.*`: allocates backend resource sets from a graphics pipeline plus set index, not from a public pipeline-layout object.
- `device.*`: stops exposing `createPipelineLayout()` publicly once the graphics-pipeline object owns layout creation.
- `scenePipelineLibrary.*`: consumes only shader modules, graphics pipelines, and per-frame resource sets.
- `shaderCompilerTests.cpp`: compile-only contract checks for the simplified public API shape.

### Task 1: Internalize Pipeline Layout Ownership Into Graphics Pipelines

**Files:**
- Modify: [`engine/include/render/rhi/rhi.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/rhi.hpp)
- Modify: [`engine/include/render/rhi/vulkan/graphicsPipeline.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/vulkan/graphicsPipeline.hpp)
- Modify: [`engine/src/render/rhi/vulkan/graphicsPipeline.cpp`](D:/Proj/MentalEngine/engine/src/render/rhi/vulkan/graphicsPipeline.cpp)
- Test: [`engine/tests/shaderCompilerTests.cpp`](D:/Proj/MentalEngine/engine/tests/shaderCompilerTests.cpp)

- [ ] **Step 1: Write the failing contract test**

```cpp
void testGraphicsPipelineContractsOwnInlineLayoutsAndPushConstants()
{
  const auto bindings = mental::render::buildSceneResourceBindings();
  const mental::rhi::ResourceLayoutDesc resourceLayoutDesc {
    .bindings = bindings.data(),
    .bindingCount = static_cast<std::uint32_t>(bindings.size()),
  };
  const mental::rhi::PushConstantRangeDesc pushConstantRange = mental::render::buildScenePushConstantRange();

  const mental::rhi::GraphicsPipelineDesc pipelineDesc {
    .vertexShaderModule = nullptr,
    .fragmentShaderModule = nullptr,
    .resourceLayoutDescs = &resourceLayoutDesc,
    .resourceLayoutDescCount = 1u,
    .pushConstantRanges = &pushConstantRange,
    .pushConstantRangeCount = 1u,
  };

  require(pipelineDesc.resourceLayoutDescCount == 1u,
    "Graphics pipeline contracts should accept inline resource layout descriptions");
  require(pipelineDesc.pushConstantRangeCount == 1u,
    "Graphics pipeline contracts should carry inline push constant ranges");
}
```

- [ ] **Step 2: Run the contract test to verify it fails**

Run: `cmake --build build --target MentalEngineShaderCompilerTests`
Expected: FAIL in `shaderCompilerTests.cpp` because `GraphicsPipelineDesc` still requires `pipelineLayout` instead of inline resource-layout and push-constant arrays.

- [ ] **Step 3: Move layout and push-constant descriptions onto `GraphicsPipelineDesc`**

```cpp
struct GraphicsPipelineDesc
{
  IShaderModule* vertexShaderModule = nullptr;
  IShaderModule* fragmentShaderModule = nullptr;
  const ResourceLayoutDesc* resourceLayoutDescs = nullptr;
  uint32_t resourceLayoutDescCount = 0;
  const PushConstantRangeDesc* pushConstantRanges = nullptr;
  uint32_t pushConstantRangeCount = 0;
  PrimitiveTopology topology = PrimitiveTopology::eTriangleList;
  PolygonMode polygonMode = PolygonMode::eFill;
  CullMode cullMode = CullMode::eBack;
  FrontFace frontFace = FrontFace::eCounterClockwise;
  bool depthTestEnable = false;
  bool depthWriteEnable = false;
  CompareOp depthCompareOp = CompareOp::eLess;
  TextureFormat colorAttachmentFormat = TextureFormat::eBGRA32_SRGB;
  TextureFormat depthAttachmentFormat = TextureFormat::eD32_SFLOAT;
  bool hasDepthAttachment = true;
};

class IGraphicsPipeline : public core::resource::IResource
{
 public:
  virtual core::Result init(const GraphicsPipelineDesc& desc) = 0;
  virtual const GraphicsPipelineDesc& getDesc() const = 0;
  virtual core::resource::Object getPipelineLayoutNativeObject() = 0;
  virtual IResourceLayout* getResourceLayout(uint32_t resourceSetIndex) const = 0;
};
```

- [ ] **Step 4: Make the Vulkan graphics pipeline own its internal `VkPipelineLayout`**

```cpp
core::Result GraphicsPipeline::init(const GraphicsPipelineDesc& desc)
{
  mResourceLayouts.clear();
  mPushConstantRanges.clear();

  for (uint32_t index = 0u; index < desc.resourceLayoutDescCount; ++index)
  {
    ResourceLayout resourceLayout {};
    core::Result result = resourceLayout.init(desc.resourceLayoutDescs[index]);
    if (result != core::Result::eSuccess)
    {
      return result;
    }
    mResourceLayouts.push_back(std::move(resourceLayout));
  }

  mPushConstantRanges.assign(desc.pushConstantRanges, desc.pushConstantRanges + desc.pushConstantRangeCount);

  VkPipelineLayout pipelineLayout = createVkPipelineLayout(mResourceLayouts, mPushConstantRanges);
  mPipelineLayout = pipelineLayout;
  mPipeline = createVkGraphicsPipeline(desc, mPipelineLayout);
  mDesc = desc;
  return core::Result::eSuccess;
}
```

- [ ] **Step 5: Run the contract test to verify it passes**

Run: `cmake --build build --target MentalEngineShaderCompilerTests`
Expected: PASS and `MentalEngineShaderCompilerTests.exe` links successfully.

- [ ] **Step 6: Commit**

```bash
git add engine/include/render/rhi/rhi.hpp engine/include/render/rhi/vulkan/graphicsPipeline.hpp engine/src/render/rhi/vulkan/graphicsPipeline.cpp engine/tests/shaderCompilerTests.cpp
git commit -m "refactor: make graphics pipelines own pipeline layouts"
```

### Task 2: Allocate Resource Sets From Graphics Pipelines Instead Of Public Pipeline Layouts

**Files:**
- Modify: [`engine/include/render/rhi/rhi.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/rhi.hpp)
- Modify: [`engine/include/render/rhi/vulkan/resourceSet.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/vulkan/resourceSet.hpp)
- Modify: [`engine/src/render/rhi/vulkan/resourceSet.cpp`](D:/Proj/MentalEngine/engine/src/render/rhi/vulkan/resourceSet.cpp)
- Modify: [`engine/include/render/rhi/vulkan/device.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/vulkan/device.hpp)
- Modify: [`engine/src/render/rhi/vulkan/device.cpp`](D:/Proj/MentalEngine/engine/src/render/rhi/vulkan/device.cpp)
- Test: [`engine/tests/shaderCompilerTests.cpp`](D:/Proj/MentalEngine/engine/tests/shaderCompilerTests.cpp)

- [ ] **Step 1: Write the failing contract test**

```cpp
void testResourceSetContractsUseGraphicsPipelinesAndSetIndices()
{
  const mental::rhi::ResourceSetDesc resourceSetDesc {
    .graphicsPipeline = nullptr,
    .resourceSetIndex = 0u,
  };

  require(resourceSetDesc.resourceSetIndex == 0u,
    "Resource sets should still identify which pipeline set they belong to");
}
```

- [ ] **Step 2: Run the contract test to verify it fails**

Run: `cmake --build build --target MentalEngineShaderCompilerTests`
Expected: FAIL because `ResourceSetDesc` still refers to `IPipelineLayout*`.

- [ ] **Step 3: Replace the public `IPipelineLayout*` dependency with `IGraphicsPipeline*`**

```cpp
struct ResourceSetDesc
{
  IGraphicsPipeline* graphicsPipeline = nullptr;
  uint32_t resourceSetIndex = 0;
};

class IDevice : public core::resource::IResource
{
 public:
  virtual std::unique_ptr<IResourceSet> createResourceSet() = 0;
  virtual std::unique_ptr<IGraphicsPipeline> createGraphicsPipeline() = 0;
};
```

- [ ] **Step 4: Resolve the backend allocation through the pipeline-owned layout**

```cpp
core::Result ResourceSet::init(const ResourceSetDesc& desc)
{
  if (desc.graphicsPipeline == nullptr)
  {
    return core::Result::eInitializationFailed;
  }

  IResourceLayout* resourceLayout = desc.graphicsPipeline->getResourceLayout(desc.resourceSetIndex);
  if (resourceLayout == nullptr)
  {
    return core::Result::eInitializationFailed;
  }

  const VkDescriptorSetLayout vkDescriptorSetLayout =
    resourceLayout->getNativeObject(core::resource::ObjectType::eVkDescriptorSetLayout);

  return allocateVkDescriptorSet(vkDescriptorSetLayout);
}
```

- [ ] **Step 5: Run the contract test to verify it passes**

Run: `cmake --build build --target MentalEngineShaderCompilerTests`
Expected: PASS and no compile errors referencing `IPipelineLayout*` in `ResourceSetDesc`.

- [ ] **Step 6: Commit**

```bash
git add engine/include/render/rhi/rhi.hpp engine/include/render/rhi/vulkan/resourceSet.hpp engine/src/render/rhi/vulkan/resourceSet.cpp engine/include/render/rhi/vulkan/device.hpp engine/src/render/rhi/vulkan/device.cpp engine/tests/shaderCompilerTests.cpp
git commit -m "refactor: allocate resource sets from graphics pipelines"
```

### Task 3: Remove Public Pipeline Layout Creation From The Renderer Path

**Files:**
- Modify: [`engine/include/render/scenePipelineLibrary.hpp`](D:/Proj/MentalEngine/engine/include/render/scenePipelineLibrary.hpp)
- Modify: [`engine/src/render/scenePipelineLibrary.cpp`](D:/Proj/MentalEngine/engine/src/render/scenePipelineLibrary.cpp)
- Modify: [`engine/include/render/rhi/rhi.hpp`](D:/Proj/MentalEngine/engine/include/render/rhi/rhi.hpp)
- Test: [`engine/tests/shaderCompilerTests.cpp`](D:/Proj/MentalEngine/engine/tests/shaderCompilerTests.cpp)

- [ ] **Step 1: Write the failing contract test**

```cpp
void testScenePipelinesCanBeDescribedWithoutPublicPipelineLayouts()
{
  const auto bindings = mental::render::buildSceneResourceBindings();
  const mental::rhi::ResourceLayoutDesc resourceLayoutDesc {
    .bindings = bindings.data(),
    .bindingCount = static_cast<std::uint32_t>(bindings.size()),
  };
  const mental::rhi::PushConstantRangeDesc pushConstantRange = mental::render::buildScenePushConstantRange();

  const mental::rhi::GraphicsPipelineDesc pipelineDesc {
    .vertexShaderModule = nullptr,
    .fragmentShaderModule = nullptr,
    .resourceLayoutDescs = &resourceLayoutDesc,
    .resourceLayoutDescCount = 1u,
    .pushConstantRanges = &pushConstantRange,
    .pushConstantRangeCount = 1u,
  };

  require(pipelineDesc.resourceLayoutDescCount == 1u,
    "Scene pipelines should be fully described without a separate public pipeline-layout object");
}
```

- [ ] **Step 2: Run the contract test to verify it fails for the renderer path**

Run: `cmake --build build --target MentalEngineShaderCompilerTests MentalEditor`
Expected: FAIL in `scenePipelineLibrary.cpp` because it still calls `createPipelineLayout()` and stores `mScenePipelineLayout`.

- [ ] **Step 3: Remove `createPipelineLayout()` from public renderer-facing flow**

```cpp
// scenePipelineLibrary.hpp
std::vector<std::unique_ptr<rhi::IResourceSet>> mSceneResourceSets {};
std::unique_ptr<rhi::IGraphicsPipeline> mPrimitivePipeline {};
std::unique_ptr<rhi::IGraphicsPipeline> mGridPipeline {};

// scenePipelineLibrary.cpp
const rhi::ResourceLayoutDesc sceneResourceLayoutDesc {
  .bindings = sceneBindings.data(),
  .bindingCount = static_cast<std::uint32_t>(sceneBindings.size()),
};
const rhi::PushConstantRangeDesc scenePushConstantRange = buildScenePushConstantRange();

result = mPrimitivePipeline->init({
  .vertexShaderModule = mPrimitiveVertexShader.get(),
  .fragmentShaderModule = mPrimitiveFragmentShader.get(),
  .resourceLayoutDescs = &sceneResourceLayoutDesc,
  .resourceLayoutDescCount = 1u,
  .pushConstantRanges = &scenePushConstantRange,
  .pushConstantRangeCount = 1u,
  .topology = rhi::PrimitiveTopology::eTriangleList,
  .polygonMode = rhi::PolygonMode::eFill,
  .cullMode = rhi::CullMode::eBack,
  .frontFace = rhi::FrontFace::eCounterClockwise,
  .depthTestEnable = true,
  .depthWriteEnable = true,
  .depthCompareOp = rhi::CompareOp::eLessOrEqual,
  .colorAttachmentFormat = mConfig.colorAttachmentFormat,
  .depthAttachmentFormat = mConfig.depthAttachmentFormat,
  .hasDepthAttachment = true,
});
```

- [ ] **Step 4: Allocate frame resource sets from one graphics pipeline and bind with each pipeline’s native layout**

```cpp
result = resourceSet->init({
  .graphicsPipeline = mPrimitivePipeline.get(),
  .resourceSetIndex = kSceneResourceSetIndex,
});

cmdList->bindGraphicsPipeline(mPrimitivePipeline.get());
cmdList->bindResourceSets(
  mPrimitivePipeline->getPipelineLayoutNativeObject(),
  kSceneResourceSetIndex,
  resourceSets,
  1u);
```

- [ ] **Step 5: Run renderer-facing verification**

Run: `cmake --build build --target MentalEngineShaderCompilerTests MentalEditor`
Expected: PASS

Run: `ctest --test-dir build --output-on-failure -R "MentalEngine(ShaderCompiler|SceneCamera|EditorApplication|PrimitiveMeshData|RenderSceneData)Tests"`
Expected: `100% tests passed, 0 tests failed out of 5`

- [ ] **Step 6: Commit**

```bash
git add engine/include/render/scenePipelineLibrary.hpp engine/src/render/scenePipelineLibrary.cpp engine/include/render/rhi/rhi.hpp engine/tests/shaderCompilerTests.cpp
git commit -m "refactor: remove public pipeline layout creation from scene pipelines"
```

### Task 4: Remove Dead Compatibility Helpers And Tighten Naming

**Files:**
- Modify: [`engine/include/render/scenePipelineLibrary.hpp`](D:/Proj/MentalEngine/engine/include/render/scenePipelineLibrary.hpp)
- Modify: [`engine/src/render/scenePipelineLibrary.cpp`](D:/Proj/MentalEngine/engine/src/render/scenePipelineLibrary.cpp)
- Modify: [`engine/tests/shaderCompilerTests.cpp`](D:/Proj/MentalEngine/engine/tests/shaderCompilerTests.cpp)

- [ ] **Step 1: Write the failing cleanup test**

```cpp
void testSharedScenePushConstantContractIsTheOnlyPublicPushConstantHelper()
{
  const mental::rhi::PushConstantRangeDesc sceneRange = mental::render::buildScenePushConstantRange();
  require(sceneRange.size >= sizeof(mental::render::PrimitiveDrawPushConstants),
    "Shared scene push constants should cover primitive payloads");
  require(sceneRange.size >= sizeof(mental::render::GridDrawPushConstants),
    "Shared scene push constants should cover grid payloads");
}
```

- [ ] **Step 2: Run the test target and identify dead helpers**

Run: `rg -n "buildPrimitivePushConstantRange|buildGridPushConstantRange|createPipelineLayout" engine/include engine/src engine/tests`
Expected: any remaining occurrences are cleanup targets for this task.

- [ ] **Step 3: Remove dead helpers and stale compatibility code**

```cpp
// Keep only:
[[nodiscard]] rhi::PushConstantRangeDesc buildScenePushConstantRange();

// Remove:
// - buildPrimitivePushConstantRange()
// - buildGridPushConstantRange()
// - any scene-side wrapper whose only behavior is returning buildScenePushConstantRange()
```

- [ ] **Step 4: Run focused verification**

Run: `cmake --build build --target MentalEngineShaderCompilerTests MentalEditor`
Expected: PASS

Run: `ctest --test-dir build --output-on-failure -R "MentalEngine(ShaderCompiler|SceneCamera|EditorApplication|PrimitiveMeshData|RenderSceneData)Tests"`
Expected: `100% tests passed, 0 tests failed out of 5`

- [ ] **Step 5: Commit**

```bash
git add engine/include/render/scenePipelineLibrary.hpp engine/src/render/scenePipelineLibrary.cpp engine/tests/shaderCompilerTests.cpp
git commit -m "refactor: remove obsolete scene pipeline compatibility helpers"
```

## Final Verification

- [ ] Run: `cmake --build build --target MentalEngineShaderCompilerTests MentalEditor`
- [ ] Expected: both targets build successfully with no compile errors from removed public pipeline-layout usage.
- [ ] Run: `ctest --test-dir build --output-on-failure -R "MentalEngine(ShaderCompiler|SceneCamera|EditorApplication|PrimitiveMeshData|RenderSceneData)Tests"`
- [ ] Expected: `100% tests passed, 0 tests failed out of 5`
- [ ] Run: `git status --short`
- [ ] Expected: only intended simplification changes are present before the final review/commit.


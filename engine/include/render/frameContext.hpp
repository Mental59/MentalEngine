#pragma once

#include <cstdint>
#include <optional>

#include <platform/window.hpp>
#include <render/sceneRenderData.hpp>

namespace mental::render
{
struct FrameContext
{
  double absoluteTimeSeconds = 0.0;
  double deltaTimeSeconds = 0.0;
  platform::WindowSize framebufferSize {};
  bool framebufferResized = false;
  SceneRenderFrame sceneRenderFrame {};
  std::optional<std::uint64_t> frameIndex = std::nullopt;
};
} // namespace mental::render

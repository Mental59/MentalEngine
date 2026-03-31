#pragma once

#include <cstdint>
#include <optional>

#include <platform/window.hpp>

namespace mental::editor
{
struct FrameContext
{
  double absoluteTimeSeconds = 0.0;
  double deltaTimeSeconds = 0.0;
  platform::WindowSize framebufferSize {};
  bool framebufferResized = false;
  std::optional<std::uint64_t> frameIndex = std::nullopt;
};
} // namespace mental::editor

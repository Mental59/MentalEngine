#pragma once

#include <core/types.hpp>
#include <platform/window.hpp>
#include <render/rhi/rhi.hpp>

namespace mental::render
{
struct FramebufferExtent
{
  uint32_t width = 0;
  uint32_t height = 0;
};

enum class FramebufferExtentRecoveryStatus : uint8_t
{
  eUsableExtent = 0,
  eClosing,
};

struct FramebufferExtentRecoveryResult
{
  FramebufferExtentRecoveryStatus status = FramebufferExtentRecoveryStatus::eClosing;
  FramebufferExtent extent {};
};

class IRenderHostAdapter
{
 public:
  virtual ~IRenderHostAdapter() = default;

  [[nodiscard]] virtual rhi::DeviceInitInput createDeviceInitInput(rhi::GraphicsApi api) const = 0;
  [[nodiscard]] virtual FramebufferExtentRecoveryResult recoverNextFramebufferExtent() const = 0;
};

class WindowRenderHostAdapter final : public IRenderHostAdapter
{
 public:
  explicit WindowRenderHostAdapter(platform::IWindow* window)
    : mWindow(window)
  {
  }

  [[nodiscard]] virtual rhi::DeviceInitInput createDeviceInitInput(rhi::GraphicsApi api) const override;
  [[nodiscard]] virtual FramebufferExtentRecoveryResult recoverNextFramebufferExtent() const override;

 private:
  platform::IWindow* mWindow = nullptr;
};
} // namespace mental::render

#pragma once
#include <platform/window.hpp>

class GLFWwindow;

namespace mental::platform
{
class GLFWwindow : public IWindow
{
public:
    GLFWwindow(const WindowDesc& desc);
    virtual ~GLFWwindow();

    virtual void pollEvents() const override;
    virtual double getTime() const override;
    virtual bool shouldClose() const override;

#if defined(MENTAL_WITH_VULKAN)
    virtual ::vk::SurfaceKHR createSurface(const ::vk::Instance& instance) const override;
#endif

private:
    ::GLFWwindow* mWindow;
};
}  // namespace mental::platform

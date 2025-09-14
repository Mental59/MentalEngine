#pragma once
#include <vector>

namespace mental::rhi::vk
{
class ExtensionManager
{
public:
    static std::vector<const char*> getRequiredInstanceExtensions();
    static std::vector<const char*> getValidationLayers();
};
}  // namespace mental::rhi::vk

#pragma once

#include <array>
#include <cstdint>

namespace mental::platform
{
enum class KeyCode : std::uint8_t
{
  eW = 0,
  eA,
  eS,
  eD,
  eQ,
  eE,
  eR,
  eEscape,
};

enum class MouseButton : std::uint8_t
{
  eLeft = 0,
  eRight,
  eMiddle,
};

inline constexpr std::array<KeyCode, 8> kKeyCodes = {
  KeyCode::eW,
  KeyCode::eA,
  KeyCode::eS,
  KeyCode::eD,
  KeyCode::eQ,
  KeyCode::eE,
  KeyCode::eR,
  KeyCode::eEscape,
};

inline constexpr std::array<MouseButton, 3> kMouseButtons = {
  MouseButton::eLeft,
  MouseButton::eRight,
  MouseButton::eMiddle,
};
} // namespace mental::platform

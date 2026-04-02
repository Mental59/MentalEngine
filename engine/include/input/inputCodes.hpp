#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace mental::input
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
  eLeftShift,
  eEscape,
};

enum class MouseButton : std::uint8_t
{
  eLeft = 0,
  eRight,
  eMiddle,
};

inline constexpr std::array<KeyCode, 9> kKeyCodes = {
  KeyCode::eW,
  KeyCode::eA,
  KeyCode::eS,
  KeyCode::eD,
  KeyCode::eQ,
  KeyCode::eE,
  KeyCode::eR,
  KeyCode::eLeftShift,
  KeyCode::eEscape,
};

inline constexpr std::array<MouseButton, 3> kMouseButtons = {
  MouseButton::eLeft,
  MouseButton::eRight,
  MouseButton::eMiddle,
};

[[nodiscard]] constexpr std::size_t toIndex(KeyCode keyCode) noexcept
{
  return static_cast<std::size_t>(keyCode);
}

[[nodiscard]] constexpr std::size_t toIndex(MouseButton mouseButton) noexcept
{
  return static_cast<std::size_t>(mouseButton);
}
} // namespace mental::input

/** @file press_classifier.hpp
 * @brief Declares a timestamp-driven press gesture classifier.
 */
/**
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#pragma once
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
namespace eurorack::controls {
enum class PressEvent : std::uint8_t { None, ShortPress, DoublePress, LongPress, Repeat };
struct PressClassifierConfig final { std::uint32_t longPressMs{600U}; std::uint32_t doublePressMs{250U}; std::uint32_t repeatDelayMs{750U}; std::uint32_t repeatIntervalMs{120U}; };
class PressClassifier final {
 public:
  explicit PressClassifier(PressClassifierConfig config={}) noexcept : config_(config) {}
  /** @brief Updates the debounced physical state and returns at most one gesture. */
  PressEvent update(bool pressed, std::uint32_t nowMs) noexcept;
  /** @brief Flushes a pending single click after the double-click window. */
  /** @brief Emits a delayed short press when its double-click window expires. @param nowMs Current monotonic time. @return ShortPress or None. */
  PressEvent poll(std::uint32_t nowMs) noexcept;
  /** @brief Resets all gesture state. @param pressed Current debounced state. @param nowMs Current monotonic time. */
  void reset(bool pressed, std::uint32_t nowMs) noexcept;
 private:
  static bool elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t duration) noexcept { return static_cast<std::uint32_t>(now-since)>=duration; }
  static bool reached(std::uint32_t now, std::uint32_t deadline) noexcept { return static_cast<std::int32_t>(now-deadline)>=0; }
  PressClassifierConfig config_{}; bool pressed_{false}; bool longEmitted_{false}; bool pendingShort_{false}; std::uint32_t pressedAt_{0U}; std::uint32_t releasedAt_{0U}; std::uint32_t nextRepeatAt_{0U};
};
}

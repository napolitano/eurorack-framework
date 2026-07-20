/** @file soft_takeover.hpp
 * @brief Declares generic pickup/soft-takeover behavior.
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
class SoftTakeover final {
 public:
  explicit SoftTakeover(std::uint16_t tolerance=8U) noexcept : tolerance_(tolerance) {}
  void arm(std::uint16_t target, std::uint16_t physical) noexcept { target_=target; previous_=physical; active_=false; }
  /** @brief Updates physical position and reports whether control now owns the target. */
  bool update(std::uint16_t physical) noexcept { if (!active_) { const bool near = physical>target_ ? physical-target_<=tolerance_ : target_-physical<=tolerance_; const bool crossed=(previous_<target_ && physical>=target_) || (previous_>target_ && physical<=target_); active_=near||crossed; } previous_=physical; return active_; }
  [[nodiscard]] bool active() const noexcept { return active_; }
  [[nodiscard]] std::uint16_t target() const noexcept { return target_; }
 private: std::uint16_t tolerance_{8U}, target_{0U}, previous_{0U}; bool active_{false};
};
}

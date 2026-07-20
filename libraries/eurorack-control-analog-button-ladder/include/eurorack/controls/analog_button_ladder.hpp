/** @file analog_button_ladder.hpp @brief Deterministic analog resistor-ladder button model.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */
#pragma once
#if defined(__AVR__)
#include <eurorack/compat/avr/cstddef.hpp>
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstddef>
#include <cstdint>
#endif
namespace eurorack::controls {
struct AnalogButtonLadderConfig final { const std::uint16_t* expectedCodes{nullptr}; std::size_t buttonCount{0U}; std::uint16_t openThreshold{1023U}; std::uint16_t acceptanceWindow{16U}; std::uint32_t stableTimeMs{64U}; };
struct AnalogButtonLadderSnapshot final { std::int16_t pressedIndex{-1}; std::int16_t justPressedIndex{-1}; std::int16_t justReleasedIndex{-1}; bool validSample{true}; };
class AnalogButtonLadder final {
 public:
  explicit AnalogButtonLadder(AnalogButtonLadderConfig config) noexcept:config_(config){}
  /** @brief Performs the documented deterministic operation. */
  void reset(std::uint16_t raw,std::uint32_t nowMs) noexcept;
  /** @brief Processes one ADC sample. @param raw ADC code. @param nowMs Monotonic time in milliseconds. */
  void update(std::uint16_t raw,std::uint32_t nowMs) noexcept;
  [[nodiscard]] const AnalogButtonLadderSnapshot& snapshot() const noexcept{return snapshot_;}
 private:
  /** @brief Performs the documented deterministic operation. */
  /** @brief Decodes a raw sample. @param raw ADC code. @param valid Receives whether the code is plausible. @return Button index or -1 for open. */
  [[nodiscard]] std::int16_t decode(std::uint16_t raw,bool& valid) const noexcept;
  AnalogButtonLadderConfig config_{}; AnalogButtonLadderSnapshot snapshot_{};
  std::int16_t candidate_{-1}; std::uint32_t candidateSinceMs_{0U}; bool initialized_{false};
};
} // namespace eurorack::controls

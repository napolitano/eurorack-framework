/** @file encoder_acceleration.hpp
 * @brief Declares a platform-neutral encoder acceleration policy.
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
struct EncoderAccelerationConfig final { std::uint32_t fastThresholdMs{20U}; std::uint32_t mediumThresholdMs{60U}; std::int16_t fastMultiplier{8}; std::int16_t mediumMultiplier{3}; };
class EncoderAcceleration final {
 public: explicit EncoderAcceleration(EncoderAccelerationConfig config={}) noexcept : config_(config) {}
 std::int16_t apply(std::int8_t detents, std::uint32_t nowMs) noexcept { if(detents==0) return 0; std::int16_t m=1; if(initialized_) { const auto dt=static_cast<std::uint32_t>(nowMs-lastMs_); if(dt<=config_.fastThresholdMs) m=config_.fastMultiplier; else if(dt<=config_.mediumThresholdMs) m=config_.mediumMultiplier; } lastMs_=nowMs; initialized_=true; return static_cast<std::int16_t>(detents)*m; }
 void reset() noexcept { initialized_=false; lastMs_=0U; }
 private: EncoderAccelerationConfig config_{}; std::uint32_t lastMs_{0U}; bool initialized_{false};
};
}

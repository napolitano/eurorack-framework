/**
 * @file src/controls/rotary_encoder.cpp
 * @brief Quadrature rotary encoder implementation.
 * *
 * @details
 * Legal Gray-code transitions contribute signed substeps to an accumulator.
 * Once the configured transitions-per-detent threshold is reached, public
 * position and delta are updated. Impossible two-bit jumps are rejected and
 * counted as diagnostics for missed samples, bounce, or wiring faults.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * @see LICENSE and ADDITIONAL_PERMISSION.md in the repository root.
 */
#include <eurorack/controls/rotary_encoder.hpp>
namespace eurorack::controls {
namespace {
constexpr std::int8_t T[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
}
/** @brief Constructs an encoder decoder. @param config Resolution and direction. */
RotaryEncoder::RotaryEncoder(const RotaryEncoderConfig config) noexcept : config_(config) {}
/** @brief Resets decoder state. @param levelA Channel A level. @param levelB Channel B level.
 * @param position Initial position. */
void RotaryEncoder::reset(bool levelA, bool levelB, std::int32_t position) noexcept {
    previousState_ = static_cast<std::uint8_t>((levelA ? 2U : 0U) | (levelB ? 1U : 0U));
    accumulator_ = 0;
    initialized_ = true;
    snapshot_ = {};
    snapshot_.position = position;
}
/** @brief Processes sampled A/B levels. @param levelA Channel A level. @param levelB Channel B
 * level. */
void RotaryEncoder::update(bool levelA, bool levelB) noexcept {
    snapshot_.delta = 0;
    snapshot_.direction = RotationDirection::None;
    const auto current = static_cast<std::uint8_t>((levelA ? 2U : 0U) | (levelB ? 1U : 0U));
    if (!initialized_) {
        reset(levelA, levelB, snapshot_.position);
        return;
    }
    const auto idx = static_cast<std::uint8_t>((previousState_ << 2U) | current);
    const auto tr = T[idx];
    if (tr == 0 && current != previousState_ && (current ^ previousState_) == 3U) {
        ++snapshot_.invalidTransitionCount;
        accumulator_ = 0;
    } else {
        accumulator_ = static_cast<std::int8_t>(accumulator_ + tr);
    }
    previousState_ = current;
    const auto th = static_cast<std::int8_t>(
        config_.transitionsPerDetent == 0U ? 1U : config_.transitionsPerDetent);
    if (accumulator_ >= th || accumulator_ <= -th) {
        std::int32_t d = accumulator_ > 0 ? 1 : -1;
        if (config_.inverted)
            d = -d;
        snapshot_.delta = d;
        snapshot_.position += d;
        snapshot_.direction =
            d > 0 ? RotationDirection::Clockwise : RotationDirection::CounterClockwise;
        accumulator_ = 0;
    }
}
/** @brief Returns current encoder state. @return Constant state reference. */
const RotaryEncoderSnapshot& RotaryEncoder::snapshot() const noexcept {
    return snapshot_;
}
} // namespace eurorack::controls

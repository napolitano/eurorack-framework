/**
 * @file src/controls/encoder_value.cpp
 * @brief Implements bounded encoder-driven values.
 *
 * @details
 * Applies signed detents using clamped or wrapped boundary handling.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#if defined(__AVR__)
#include <eurorack/compat/avr/algorithm.hpp>
#else
#include <algorithm>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/controls/encoder_value.hpp>

namespace eurorack::controls {

/*
 * Purpose: Constructs a bounded encoder value.
 *
 * config: Bounds, step size, and boundary behavior.
 * initialValue: Initial value before range normalization.
 */
EncoderValue::EncoderValue(const EncoderValueConfig config,
                           const std::int32_t initialValue) noexcept
    : config_(config) {
    if (config_.maximum < config_.minimum) {
        std::swap(config_.minimum, config_.maximum);
    }

    if (config_.step == 0) {
        config_.step = 1;
    }

    reset(initialValue);
}

/*
 * Purpose: Resets the current value.
 *
 * value: New value before range normalization.
 */
void EncoderValue::reset(const std::int32_t value) noexcept {
    snapshot_ = {};
    snapshot_.value = normalize(value);
    snapshot_.changed = false;
    snapshot_.wrapped = false;
    snapshot_.clamped = false;
}

/*
 * Purpose: Applies relative encoder detents.
 *
 * detents: Signed detent count.
 */
void EncoderValue::applyDetents(const std::int32_t detents) noexcept {
    snapshot_.delta = 0;
    snapshot_.changed = false;
    snapshot_.wrapped = false;
    snapshot_.clamped = false;

    if (detents == 0) {
        return;
    }

    const std::int32_t previous = snapshot_.value;
    const std::int64_t candidate =
        static_cast<std::int64_t>(previous) +
        static_cast<std::int64_t>(detents) * static_cast<std::int64_t>(config_.step);

    snapshot_.value = normalize(candidate);
    snapshot_.delta = snapshot_.value - previous;
    snapshot_.changed = snapshot_.value != previous;
}

/*
 * Purpose: Returns the current immutable value state.
 *
 * Returns: Constant reference to current value and event flags.
 */
const EncoderValueSnapshot& EncoderValue::snapshot() const noexcept {
    return snapshot_;
}

/*
 * Purpose: Normalizes one candidate value according to configured bounds.
 *
 * candidate: Unbounded candidate value.
 * Returns: Bounded value.
 */
std::int32_t EncoderValue::normalize(const std::int64_t candidate) noexcept {
    const std::int64_t minimum = config_.minimum;
    const std::int64_t maximum = config_.maximum;

    if (config_.boundaryMode == EncoderBoundaryMode::Clamp) {
        if (candidate < minimum) {
            snapshot_.clamped = true;
            return config_.minimum;
        }

        if (candidate > maximum) {
            snapshot_.clamped = true;
            return config_.maximum;
        }

        return static_cast<std::int32_t>(candidate);
    }

    const std::int64_t span = maximum - minimum + 1;
    if (span <= 0) {
        return config_.minimum;
    }

    if (candidate < minimum || candidate > maximum) {
        snapshot_.wrapped = true;
    }

    const std::int64_t offset = ((candidate - minimum) % span + span) % span;

    return static_cast<std::int32_t>(minimum + offset);
}

} // namespace eurorack::controls

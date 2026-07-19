/**
 * @file src/controls/gate.cpp
 * @brief Implements hysteretic gate input and timed trigger output models.
 *
 * @details
 * Implements polarity conversion and edge detection for gate signals.
 *
 * Each sampled electrical level is converted into logical active state. Rising and falling flags
 describe only the most recent update, while transition count and stable state persist.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * @see LICENSE and ADDITIONAL_PERMISSION.md in the repository root.
 */

#include <eurorack/controls/gate.hpp>

namespace eurorack::controls {

/**
 * @brief Constructs a gate-input model.
 *
 * @param lowMaximum Highest voltage guaranteed to be interpreted as low.
 * @param highMinimum Lowest voltage guaranteed to be interpreted as high.
 */
GateInput::GateInput(const float lowMaximum, const float highMinimum) noexcept
    : lowMaximum_(lowMaximum), highMinimum_(highMinimum) {}

/**
 * @brief Processes one measured gate voltage.
 *
 * @param volts Measured gate voltage.
 */
void GateInput::update(const float volts) noexcept {
    snapshot_.rising = false;
    snapshot_.falling = false;
    snapshot_.volts = volts;

    if (!snapshot_.high && volts >= highMinimum_) {
        snapshot_.high = true;
        snapshot_.rising = true;
    } else if (snapshot_.high && volts <= lowMaximum_) {
        snapshot_.high = false;
        snapshot_.falling = true;
    }
}

/**
 * @brief Returns the current immutable gate-input state.
 *
 * @return Constant reference to gate state and edge flags.
 */
const GateInputSnapshot& GateInput::snapshot() const noexcept {
    return snapshot_;
}

/**
 * @brief Constructs a trigger-output model.
 *
 * @param pulseMs Pulse duration in milliseconds.
 */
TriggerOutput::TriggerOutput(const std::uint32_t pulseMs) noexcept : pulseMs_(pulseMs) {}

/**
 * @brief Starts or restarts a trigger pulse.
 *
 * @param nowMs Current monotonic time in milliseconds.
 */
void TriggerOutput::trigger(const std::uint32_t nowMs) noexcept {
    snapshot_.high = true;
    snapshot_.started = true;
    snapshot_.ended = false;
    snapshot_.startedAtMs = nowMs;
}

/**
 * @brief Advances trigger timing.
 *
 * @param nowMs Current monotonic time in milliseconds.
 */
void TriggerOutput::update(const std::uint32_t nowMs) noexcept {
    snapshot_.started = false;
    snapshot_.ended = false;

    if (snapshot_.high && nowMs - snapshot_.startedAtMs >= pulseMs_) {
        snapshot_.high = false;
        snapshot_.ended = true;
    }
}

/**
 * @brief Returns the current immutable trigger-output state.
 *
 * @return Constant reference to trigger timing and edge flags.
 */
const TriggerOutputSnapshot& TriggerOutput::snapshot() const noexcept {
    return snapshot_;
}

} // namespace eurorack::controls

/**
 * @file src/controls/momentary_button.cpp
 * @brief Implements the platform-independent momentary pushbutton state machine.
 *
 * @details
 * Implements a timestamp-based debounce state machine for a momentary button.
 * The first observed sample initializes state without fabricating an edge. A changed electrical
 * candidate must remain stable for the configured interval before it becomes the accepted logical
 * state. Press duration uses unsigned subtraction, preserving correct behavior across a 32-bit
 * millisecond timer wraparound.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 *
 * This file is part of Eurorack Framework. Noncommercial use, modification,
 * and redistribution are permitted under the terms in the repository-root
 * LICENSE file. The separate ADDITIONAL_PERMISSION.md file permits limited
 * five-unit, cost-recovery distribution under its stated conditions.
 * Commercial use requires prior written permission.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/controls/momentary_button.hpp>

namespace eurorack::controls {

/*
 * Purpose: Resets the button state machine to a known sampled level.
 *
 * rawLevelHigh: True when the sampled electrical input is high.
 * nowMs: Current monotonic timestamp in milliseconds.
 *
 * The method converts the electrical level using the configured polarity,
 * initializes all debounce candidates, clears event history, and produces no
 * synthetic transition.
 */
void MomentaryButton::reset(const bool rawLevelHigh, const std::uint32_t nowMs) noexcept {
    const bool logicalPressed = toLogicalPressed(rawLevelHigh);

    initialized_ = true;
    candidatePressed_ = logicalPressed;
    candidateSinceMs_ = nowMs;
    pressedSinceMs_ = nowMs;

    snapshot_ = {};
    snapshot_.pressed = logicalPressed;
}

/*
 * Purpose: Processes one raw button sample and advances debounce state.
 *
 * rawLevelHigh: True when the sampled electrical input is high.
 * nowMs: Current monotonic timestamp in milliseconds.
 *
 * The method clears one-update event flags, tracks candidate transitions,
 * accepts changes only after the configured stable interval, and updates hold
 * durations using wrap-safe unsigned timestamp arithmetic.
 */
void MomentaryButton::update(const bool rawLevelHigh, const std::uint32_t nowMs) noexcept {
    const bool logicalPressed = toLogicalPressed(rawLevelHigh);

    snapshot_.justPressed = false;
    snapshot_.justReleased = false;

    if (!initialized_) {
        reset(rawLevelHigh, nowMs);
        return;
    }

    if (logicalPressed != candidatePressed_) {
        candidatePressed_ = logicalPressed;
        candidateSinceMs_ = nowMs;
    }

    const std::uint32_t candidateAgeMs = nowMs - candidateSinceMs_;
    const bool transitionIsStable =
        candidatePressed_ != snapshot_.pressed && candidateAgeMs >= config_.debounceTimeMs;

    if (transitionIsStable) {
        snapshot_.pressed = candidatePressed_;
        ++snapshot_.transitionCount;

        if (snapshot_.pressed) {
            snapshot_.justPressed = true;
            pressedSinceMs_ = nowMs;
        } else {
            snapshot_.justReleased = true;
            snapshot_.lastPressDurationMs = nowMs - pressedSinceMs_;
            snapshot_.heldForMs = 0U;
        }
    }

    if (snapshot_.pressed) {
        snapshot_.heldForMs = nowMs - pressedSinceMs_;
    } else {
        snapshot_.heldForMs = 0U;
    }
}

} // namespace eurorack::controls

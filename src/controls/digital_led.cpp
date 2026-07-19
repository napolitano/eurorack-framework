/**
 * @file src/controls/digital_led.cpp
 * @brief Implements the platform-independent binary LED state model.
 *
 * @details
 * The implementation handles logical transitions, edge flags, transition counting, reset behavior,
 * and active-level conversion. Physical GPIO writes remain the responsibility of a platform adapter
 * or example.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @license PolyForm Noncommercial License 1.0.0
 *
 * This file is part of Eurorack Framework. Noncommercial use, modification,
 * and redistribution are permitted under the terms in the repository-root
 * LICENSE file. The separate ADDITIONAL_PERMISSION.md file permits limited
 * five-unit, cost-recovery distribution under its stated conditions.
 * Commercial use requires prior written permission.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/controls/digital_led.hpp>

namespace eurorack::controls {

/**
 * @brief Restores the configured initial LED state.
 *
 * The method clears transition flags and history, then recalculates the raw
 * output level from the configured active polarity. It performs no GPIO access.
 */
void DigitalLed::reset() noexcept {
    snapshot_ = {
        config_.initiallyOn,
        false,
        false,
        toRawOutputHigh(config_.initiallyOn, config_.activeLevel),
        0U,
    };
}

/**
 * @brief Applies a requested logical LED state.
 *
 * @param on True to request the logical on state; false to request off.
 *
 * A changed state updates polarity-aware output level, transition flags, and
 * transition count. An idempotent request clears event flags without counting
 * a transition.
 */
void DigitalLed::setOn(const bool on) noexcept {
    snapshot_.justTurnedOn = false;
    snapshot_.justTurnedOff = false;

    if (snapshot_.on == on) {
        return;
    }

    snapshot_.on = on;
    snapshot_.rawOutputHigh = toRawOutputHigh(on, config_.activeLevel);
    ++snapshot_.transitionCount;

    snapshot_.justTurnedOn = on;
    snapshot_.justTurnedOff = !on;
}

/**
 * @brief Requests the logical LED on state.
 *
 * The method delegates to setOn(true) and performs no direct hardware access.
 */
void DigitalLed::turnOn() noexcept {
    setOn(true);
}

/**
 * @brief Requests the logical LED off state.
 *
 * The method delegates to setOn(false) and performs no direct hardware access.
 */
void DigitalLed::turnOff() noexcept {
    setOn(false);
}

/**
 * @brief Inverts the current logical LED state.
 *
 * The method delegates to setOn() with the inverse current state, producing one
 * logical transition and recalculating the required electrical output level.
 */
void DigitalLed::toggle() noexcept {
    setOn(!snapshot_.on);
}

} // namespace eurorack::controls

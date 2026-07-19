/**
 * @file src/controls/bicolor_led.cpp
 * @brief Two-color LED implementation.
 * *
 * @details
 * Common-anode and common-cathode devices can energize both dies at once. A
 * two-terminal bipolar device selects color by current direction and therefore
 * requires external time multiplexing for a mixed color. This model calculates
 * drive intent but does not schedule GPIO transitions.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * @see LICENSE and ADDITIONAL_PERMISSION.md in the repository root.
 */
#include <eurorack/controls/bicolor_led.hpp>
namespace eurorack::controls {
/* Purpose: Constructs the LED model. config: Wiring topology and initial color. */
BiColorLed::BiColorLed(const BiColorLedConfig config) noexcept : config_(config) {
    reset();
}
/* Purpose: Restores initial color and clears history. */
void BiColorLed::reset() noexcept {
    snapshot_ = {};
    snapshot_.requestedColor = config_.initialColor;
    recalculate();
    snapshot_.changed = false;
    snapshot_.transitionCount = 0U;
}
/* Purpose: Requests a logical color. color: Requested color. */
void BiColorLed::setColor(const BiColorLedColor color) noexcept {
    snapshot_.changed = false;
    if (snapshot_.requestedColor == color)
        return;
    snapshot_.requestedColor = color;
    ++snapshot_.transitionCount;
    snapshot_.changed = true;
    recalculate();
}
/* Purpose: Returns current state. Returns: Constant state reference. */
const BiColorLedSnapshot& BiColorLed::snapshot() const noexcept {
    return snapshot_;
}
/* Purpose: Returns wiring topology. Returns: LED topology. */
BiColorLedTopology BiColorLed::topology() const noexcept {
    return config_.topology;
}
/* Purpose: Recalculates effective color and required pin drives. */
void BiColorLed::recalculate() noexcept {
    snapshot_.effectiveColor = snapshot_.requestedColor;
    snapshot_.multiplexingRequired = false;
    if (config_.topology == BiColorLedTopology::CommonAnode) {
        switch (snapshot_.requestedColor) {
        case BiColorLedColor::Off:
            snapshot_.pinA = PinDrive::High;
            snapshot_.pinB = PinDrive::High;
            break;
        case BiColorLedColor::ColorA:
            snapshot_.pinA = PinDrive::Low;
            snapshot_.pinB = PinDrive::High;
            break;
        case BiColorLedColor::ColorB:
            snapshot_.pinA = PinDrive::High;
            snapshot_.pinB = PinDrive::Low;
            break;
        case BiColorLedColor::Mixed:
            snapshot_.pinA = PinDrive::Low;
            snapshot_.pinB = PinDrive::Low;
            break;
        }
        return;
    }
    if (config_.topology == BiColorLedTopology::CommonCathode) {
        switch (snapshot_.requestedColor) {
        case BiColorLedColor::Off:
            snapshot_.pinA = PinDrive::Low;
            snapshot_.pinB = PinDrive::Low;
            break;
        case BiColorLedColor::ColorA:
            snapshot_.pinA = PinDrive::High;
            snapshot_.pinB = PinDrive::Low;
            break;
        case BiColorLedColor::ColorB:
            snapshot_.pinA = PinDrive::Low;
            snapshot_.pinB = PinDrive::High;
            break;
        case BiColorLedColor::Mixed:
            snapshot_.pinA = PinDrive::High;
            snapshot_.pinB = PinDrive::High;
            break;
        }
        return;
    }
    switch (snapshot_.requestedColor) {
    case BiColorLedColor::Off:
        snapshot_.pinA = PinDrive::Low;
        snapshot_.pinB = PinDrive::Low;
        break;
    case BiColorLedColor::ColorA:
        snapshot_.pinA = PinDrive::High;
        snapshot_.pinB = PinDrive::Low;
        break;
    case BiColorLedColor::ColorB:
        snapshot_.pinA = PinDrive::Low;
        snapshot_.pinB = PinDrive::High;
        break;
    case BiColorLedColor::Mixed:
        snapshot_.effectiveColor = BiColorLedColor::Off;
        snapshot_.pinA = PinDrive::Low;
        snapshot_.pinB = PinDrive::Low;
        snapshot_.multiplexingRequired = true;
        break;
    }
}
} // namespace eurorack::controls

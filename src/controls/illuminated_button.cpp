/**
 * @file src/controls/illuminated_button.cpp
 * @brief Implements the illuminated momentary button.
 *
 * @details
 * Updates the debounced button and optionally maps its stable state to an RGB LED color.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#include <eurorack/controls/illuminated_button.hpp>

namespace eurorack::controls {

IlluminatedButton::IlluminatedButton(const IlluminatedButtonConfig config) noexcept
    : config_(config), button_(config.button), led_(config.inactiveColor) {}

void IlluminatedButton::reset(const bool rawLevelHigh, const std::uint32_t nowMs) noexcept {
    button_.reset(rawLevelHigh, nowMs);
    led_.reset();
    updateAutomaticLed();
}

void IlluminatedButton::update(const bool rawLevelHigh, const std::uint32_t nowMs) noexcept {
    button_.update(rawLevelHigh, nowMs);
    updateAutomaticLed();
}

void IlluminatedButton::setLedColor(const RgbColor color) noexcept {
    led_.setColor(color);
}
void IlluminatedButton::setLedBrightness(const std::uint16_t brightness) noexcept {
    led_.setMasterBrightness(brightness);
}
const MomentaryButton& IlluminatedButton::button() const noexcept {
    return button_;
}
const MulticolorLed& IlluminatedButton::led() const noexcept {
    return led_;
}

void IlluminatedButton::updateAutomaticLed() noexcept {
    if (config_.mode == IlluminatedButtonMode::Manual) {
        return;
    }
    const bool active = config_.mode == IlluminatedButtonMode::LitWhilePressed
                            ? button_.isPressed()
                            : !button_.isPressed();
    led_.setColor(active ? config_.activeColor : config_.inactiveColor);
}

} // namespace eurorack::controls

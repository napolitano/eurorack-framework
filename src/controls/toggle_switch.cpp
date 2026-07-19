/**
 * @file src/controls/toggle_switch.cpp
 * @brief Implements the debounced two-position toggle switch.
 *
 * @details
 * Uses the existing momentary-button debounce state machine while exposing maintained-switch terminology.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#include <eurorack/controls/toggle_switch.hpp>

namespace eurorack::controls {

ToggleSwitch::ToggleSwitch(const ToggleSwitchConfig config) noexcept
    : config_(config),
      debouncer_({config.activeLevel, config.debounceTimeMs}) {
}

void ToggleSwitch::reset(const bool rawLevelHigh, const std::uint32_t nowMs) noexcept {
    debouncer_.reset(rawLevelHigh, nowMs);
    snapshot_ = {};
    snapshot_.position = debouncer_.isPressed()
        ? ToggleSwitchPosition::On
        : ToggleSwitchPosition::Off;
}

void ToggleSwitch::update(const bool rawLevelHigh, const std::uint32_t nowMs) noexcept {
    debouncer_.update(rawLevelHigh, nowMs);
    snapshot_.justTurnedOn = debouncer_.wasPressed();
    snapshot_.justTurnedOff = debouncer_.wasReleased();
    snapshot_.position = debouncer_.isPressed()
        ? ToggleSwitchPosition::On
        : ToggleSwitchPosition::Off;
    snapshot_.transitionCount = debouncer_.snapshot().transitionCount;
}

const ToggleSwitchSnapshot& ToggleSwitch::snapshot() const noexcept { return snapshot_; }
bool ToggleSwitch::isOn() const noexcept { return snapshot_.position == ToggleSwitchPosition::On; }
const ToggleSwitchConfig& ToggleSwitch::config() const noexcept { return config_; }

} // namespace eurorack::controls

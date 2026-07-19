/**
 * @file src/controls/dip_switch.cpp
 * @brief Implements an individual DIP-switch pole.
 *
 * @details
 * Delegates electrical debounce and maintained-state handling to ToggleSwitch.
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

#include <eurorack/controls/dip_switch.hpp>

namespace eurorack::controls {

DipSwitch::DipSwitch(const ToggleSwitchConfig config) noexcept : switch_(config) {}
void DipSwitch::reset(const bool rawLevelHigh, const std::uint32_t nowMs) noexcept {
    switch_.reset(rawLevelHigh, nowMs);
}
void DipSwitch::update(const bool rawLevelHigh, const std::uint32_t nowMs) noexcept {
    switch_.update(rawLevelHigh, nowMs);
}
bool DipSwitch::isOn() const noexcept {
    return switch_.isOn();
}
const ToggleSwitchSnapshot& DipSwitch::snapshot() const noexcept {
    return switch_.snapshot();
}

} // namespace eurorack::controls

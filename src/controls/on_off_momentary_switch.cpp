/**
 * @file src/controls/on_off_momentary_switch.cpp
 * @brief Implements the debounced On-Off-(On) switch.
 *
 * @details
 * Combines two independently debounced contacts into one On-Off-(On) position.
 * Neither contact asserted represents center Off. The maintained contact selects On, and the spring-return contact selects MomentaryOn. Simultaneous assertion is preserved as Invalid rather than resolved by priority, making wiring or sampling faults visible to application logic.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#include <eurorack/controls/on_off_momentary_switch.hpp>

namespace eurorack::controls {

OnOffMomentarySwitch::OnOffMomentarySwitch(
    const OnOffMomentarySwitchConfig config) noexcept
    : config_(config),
      maintained_({config.maintainedActiveLevel, config.debounceTimeMs}),
      momentary_({config.momentaryActiveLevel, config.debounceTimeMs}) {
}

void OnOffMomentarySwitch::reset(
    const bool maintainedRawHigh,
    const bool momentaryRawHigh,
    const std::uint32_t nowMs) noexcept {
    maintained_.reset(maintainedRawHigh, nowMs);
    momentary_.reset(momentaryRawHigh, nowMs);
    snapshot_ = {};
    snapshot_.position = derivePosition();
    snapshot_.previousPosition = snapshot_.position;
    snapshot_.invalidCombination = snapshot_.position == OnOffMomentaryPosition::Invalid;
}

void OnOffMomentarySwitch::update(
    const bool maintainedRawHigh,
    const bool momentaryRawHigh,
    const std::uint32_t nowMs) noexcept {
    maintained_.update(maintainedRawHigh, nowMs);
    momentary_.update(momentaryRawHigh, nowMs);

    const auto next = derivePosition();
    snapshot_.changed = next != snapshot_.position;
    if (snapshot_.changed) {
        snapshot_.previousPosition = snapshot_.position;
        snapshot_.position = next;
        ++snapshot_.transitionCount;
    }
    snapshot_.invalidCombination = next == OnOffMomentaryPosition::Invalid;
}

const OnOffMomentarySwitchSnapshot& OnOffMomentarySwitch::snapshot() const noexcept {
    return snapshot_;
}
OnOffMomentaryPosition OnOffMomentarySwitch::position() const noexcept {
    return snapshot_.position;
}
OnOffMomentaryPosition OnOffMomentarySwitch::derivePosition() const noexcept {
    if (maintained_.isPressed() && momentary_.isPressed()) {
        return OnOffMomentaryPosition::Invalid;
    }
    if (maintained_.isPressed()) {
        return OnOffMomentaryPosition::On;
    }
    if (momentary_.isPressed()) {
        return OnOffMomentaryPosition::MomentaryOn;
    }
    return OnOffMomentaryPosition::Off;
}

} // namespace eurorack::controls

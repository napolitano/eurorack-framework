/**
 * @file src/controls/analog_button_ladder.cpp
 * @brief Implements deterministic resistor-ladder button decoding.
 *
 * @details
 * `decode()` first checks the open-circuit case (a sample at or above `openThreshold`, meaning
 * no button is pressed), then performs a linear nearest-neighbor search over `expectedCodes`,
 * tracking the smallest absolute distance seen so far; a distance beyond `acceptanceWindow`
 * marks the sample invalid rather than guessing the closest button under noise. `update()` layers
 * a stable-transition timer on top of that per-sample decode: a newly observed code becomes the
 * debounce candidate immediately, but is only promoted to `pressedIndex` (with the corresponding
 * `justPressedIndex`/`justReleasedIndex` edge) once it has read identically for at least
 * `stableTimeMs`, so a button in mechanical or electrical transition between two resistances does
 * not produce spurious presses. The whole implementation is allocation-free and touches no
 * hardware directly; it operates purely on the raw code and timestamp it is given.

 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */

#include <eurorack/controls/analog_button_ladder.hpp>

namespace eurorack::controls {

std::int16_t AnalogButtonLadder::decode(std::uint16_t raw, bool& valid) const noexcept {
    valid = true;

    if (raw >= config_.openThreshold) {
        return -1;
    }

    if (config_.expectedCodes == nullptr || config_.buttonCount == 0U) {
        valid = false;
        return -1;
    }

    std::size_t best = 0U;
    std::uint16_t bestDistance = 0xFFFFU;
    for (std::size_t i = 0; i < config_.buttonCount; ++i) {
        const std::uint16_t expected = config_.expectedCodes[i];
        const std::uint16_t distance = raw > expected ? static_cast<std::uint16_t>(raw - expected)
                                                      : static_cast<std::uint16_t>(expected - raw);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = i;
        }
    }

    if (bestDistance > config_.acceptanceWindow) {
        valid = false;
        return -1;
    }

    return static_cast<std::int16_t>(best);
}

void AnalogButtonLadder::reset(std::uint16_t raw, std::uint32_t nowMs) noexcept {
    bool valid = false;
    const std::int16_t value = decode(raw, valid);
    snapshot_ = {value, -1, -1, valid};
    candidate_ = value;
    candidateSinceMs_ = nowMs;
    initialized_ = true;
}

void AnalogButtonLadder::update(std::uint16_t raw, std::uint32_t nowMs) noexcept {
    snapshot_.justPressedIndex = -1;
    snapshot_.justReleasedIndex = -1;

    bool valid = false;
    const std::int16_t decoded = decode(raw, valid);
    snapshot_.validSample = valid;

    if (!initialized_) {
        reset(raw, nowMs);
        return;
    }

    if (!valid) {
        return;
    }

    if (decoded != candidate_) {
        candidate_ = decoded;
        candidateSinceMs_ = nowMs;
        return;
    }

    if (decoded == snapshot_.pressedIndex) {
        return;
    }

    if (static_cast<std::uint32_t>(nowMs - candidateSinceMs_) < config_.stableTimeMs) {
        return;
    }

    const std::int16_t previous = snapshot_.pressedIndex;
    snapshot_.pressedIndex = decoded;
    if (previous >= 0) {
        snapshot_.justReleasedIndex = previous;
    }
    if (decoded >= 0) {
        snapshot_.justPressedIndex = decoded;
    }
}

} // namespace eurorack::controls

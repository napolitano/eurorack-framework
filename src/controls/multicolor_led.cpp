/**
 * @file src/controls/multicolor_led.cpp
 * @brief Implements the RGB multicolor LED model.
 *
 * @details
 * Implements logical RGB color and master-brightness scaling.
 * 
 * Each effective channel is calculated with a 32-bit intermediate and rounded back to 16 bits. Transition metadata changes only when effective optical intent changes, not merely when a setter repeats the same value.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#include <eurorack/controls/multicolor_led.hpp>

namespace eurorack::controls {
namespace {
[[nodiscard]] std::uint16_t scaleChannel(
    const std::uint16_t channel,
    const std::uint16_t master) noexcept {
    const std::uint32_t product =
        static_cast<std::uint32_t>(channel)
        * static_cast<std::uint32_t>(master);
    return static_cast<std::uint16_t>(
        (product + 32767U) / 65535U);
}
[[nodiscard]] bool sameColor(const RgbColor a, const RgbColor b) noexcept {
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}
} // namespace

MulticolorLed::MulticolorLed(
    const RgbColor initialColor,
    const std::uint16_t masterBrightness) noexcept
    : initialColor_(initialColor),
      initialMasterBrightness_(masterBrightness) {
    reset();
}

void MulticolorLed::reset() noexcept {
    snapshot_ = {};
    snapshot_.requested = initialColor_;
    snapshot_.masterBrightness = initialMasterBrightness_;
    snapshot_.effective = {
        scaleChannel(initialColor_.red, initialMasterBrightness_),
        scaleChannel(initialColor_.green, initialMasterBrightness_),
        scaleChannel(initialColor_.blue, initialMasterBrightness_)};
}

void MulticolorLed::setColor(const RgbColor color) noexcept {
    snapshot_.requested = color;
    recalculate();
}

void MulticolorLed::setMasterBrightness(const std::uint16_t brightness) noexcept {
    snapshot_.masterBrightness = brightness;
    recalculate();
}

void MulticolorLed::turnOff() noexcept {
    setColor({});
}

void MulticolorLed::applyTo(
    eurorack::drivers::led::LedChannel& red,
    eurorack::drivers::led::LedChannel& green,
    eurorack::drivers::led::LedChannel& blue) const noexcept {
    red.setBrightness(snapshot_.effective.red);
    green.setBrightness(snapshot_.effective.green);
    blue.setBrightness(snapshot_.effective.blue);
}

const MulticolorLedSnapshot& MulticolorLed::snapshot() const noexcept { return snapshot_; }

void MulticolorLed::recalculate() noexcept {
    const RgbColor previous = snapshot_.effective;
    snapshot_.effective = {
        scaleChannel(snapshot_.requested.red, snapshot_.masterBrightness),
        scaleChannel(snapshot_.requested.green, snapshot_.masterBrightness),
        scaleChannel(snapshot_.requested.blue, snapshot_.masterBrightness)};
    snapshot_.changed = !sameColor(previous, snapshot_.effective);
    if (snapshot_.changed) {
        ++snapshot_.transitionCount;
    }
}

} // namespace eurorack::controls

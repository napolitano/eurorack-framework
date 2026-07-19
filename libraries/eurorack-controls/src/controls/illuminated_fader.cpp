/**
 * @file src/controls/illuminated_fader.cpp
 * @brief Implements the illuminated fader.
 *
 * @details
 * Maps normalized fader travel to a bounded 16-bit LED master brightness in automatic modes.
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

#if defined(__AVR__)
#include <eurorack/compat/avr/algorithm.hpp>
#else
#include <algorithm>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cmath.hpp>
#else
#include <cmath>
#endif
#include <eurorack/controls/illuminated_fader.hpp>

namespace eurorack::controls {

IlluminatedFader::IlluminatedFader(const IlluminatedFaderConfig config) noexcept
    : config_(config), fader_(config.fader), led_(config.color, config.minimumBrightness) {}

void IlluminatedFader::reset(const std::uint32_t raw) noexcept {
    fader_.reset(raw);
    led_.reset();
    updateAutomaticLed();
}
void IlluminatedFader::update(const std::uint32_t raw) noexcept {
    fader_.update(raw);
    updateAutomaticLed();
}
void IlluminatedFader::setLedColor(const RgbColor color) noexcept {
    led_.setColor(color);
}
void IlluminatedFader::setLedBrightness(const std::uint16_t brightness) noexcept {
    led_.setMasterBrightness(brightness);
}
const Fader& IlluminatedFader::fader() const noexcept {
    return fader_;
}
const MulticolorLed& IlluminatedFader::led() const noexcept {
    return led_;
}

void IlluminatedFader::updateAutomaticLed() noexcept {
    if (config_.mode == IlluminatedFaderMode::Manual) {
        return;
    }

    float position = fader_.snapshot().normalized;
    if (config_.mode == IlluminatedFaderMode::InversePosition) {
        position = 1.0F - position;
    }
    position = std::clamp(position, 0.0F, 1.0F);

    const std::uint16_t low = std::min(config_.minimumBrightness, config_.maximumBrightness);
    const std::uint16_t high = std::max(config_.minimumBrightness, config_.maximumBrightness);
    const float value = static_cast<float>(low) +
                        position * static_cast<float>(static_cast<std::uint32_t>(high) - low);
    led_.setMasterBrightness(static_cast<std::uint16_t>(std::lround(value)));
}

} // namespace eurorack::controls

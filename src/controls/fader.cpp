/**
 * @file src/controls/fader.cpp
 * @brief Implements the calibrated linear fader.
 *
 * @details
 * Maps fader direction into potentiometer inversion and delegates filtering and range handling.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#include <eurorack/controls/fader.hpp>

namespace eurorack::controls {

Fader::Fader(const FaderConfig config) noexcept
    : config_(config),
      potentiometer_({config.rawMinimum,
                      config.rawMaximum,
                      config.direction == FaderDirection::TopToBottom,
                      config.deadbandNormalized,
                      config.smoothingFactor}) {
}
void Fader::reset(const std::uint32_t raw) noexcept { potentiometer_.reset(raw); }
void Fader::update(const std::uint32_t raw) noexcept { potentiometer_.update(raw); }
const FaderSnapshot& Fader::snapshot() const noexcept { return potentiometer_.snapshot(); }
const FaderConfig& Fader::config() const noexcept { return config_; }

} // namespace eurorack::controls

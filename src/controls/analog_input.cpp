/**
 * @file src/controls/analog_input.cpp
 * @brief Implements calibrated normalization for raw ADC samples.
 *
 * @details
 * Implements one normalized analog-control state model.
 * 
 * Raw samples and acquisition results are copied into a snapshot. Successful codes are clamped and normalized against configured endpoints; failed samples preserve diagnostics without inventing a new valid value.

 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * @see LICENSE and ADDITIONAL_PERMISSION.md in the repository root.
 */

#include <eurorack/controls/analog_input.hpp>

namespace eurorack::controls {

/**
 * @brief Constructs an analog-input normalizer.
 *
 * @param config ADC calibration and optional direction inversion.
 */
AnalogInput::AnalogInput(const AnalogInputConfig config) noexcept : config_(config) {}

/**
 * @brief Processes one raw ADC code.
 *
 * @param raw Raw ADC sample.
 */
void AnalogInput::update(const std::uint32_t raw) noexcept {
    snapshot_.raw = raw;
    snapshot_.belowRange = raw < config_.rawMinimum;
    snapshot_.aboveRange = raw > config_.rawMaximum;

    const std::uint32_t clamped = raw < config_.rawMinimum
                                      ? config_.rawMinimum
                                      : (raw > config_.rawMaximum ? config_.rawMaximum : raw);

    const std::uint32_t span =
        config_.rawMaximum > config_.rawMinimum ? config_.rawMaximum - config_.rawMinimum : 1U;

    float normalized = static_cast<float>(clamped - config_.rawMinimum) / static_cast<float>(span);

    if (config_.inverted) {
        normalized = 1.0F - normalized;
    }

    snapshot_.normalized = normalized;
    snapshot_.bipolar = normalized * 2.0F - 1.0F;
}

/**
 * @brief Returns the current immutable analog-input state.
 *
 * @return Constant reference to the latest normalized sample.
 */
const AnalogInputSnapshot& AnalogInput::snapshot() const noexcept {
    return snapshot_;
}

} // namespace eurorack::controls

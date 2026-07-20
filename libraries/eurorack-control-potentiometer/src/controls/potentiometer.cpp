/**
 * @file src/controls/potentiometer.cpp
 * @brief Implements calibrated potentiometer processing.
 *
 * @details
 * Normalizes raw ADC samples into unipolar and bipolar application values.
 * Input codes are clamped to calibrated endpoints before optional inversion. Exponential smoothing
 * moves toward the new target, while deadband suppresses insignificant public changes. Range flags
 * always reflect the latest raw sample even when deadband prevents a normalized-value update.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
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
#include <eurorack/controls/potentiometer.hpp>

namespace eurorack::controls {

/*
 * Purpose: Constructs an uninitialized potentiometer model.
 *
 * config: Calibration, inversion, deadband, and smoothing settings.
 */
Potentiometer::Potentiometer(const PotentiometerConfig config) noexcept : config_(config) {
    config_.deadbandNormalized = std::clamp(config_.deadbandNormalized, 0.0F, 1.0F);
    config_.smoothingFactor = std::clamp(config_.smoothingFactor, 0.0F, 1.0F);
}

/*
 * Purpose: Resets the model from one raw ADC sample.
 *
 * raw: Raw ADC code.
 */
void Potentiometer::reset(const std::uint32_t raw) noexcept {
    snapshot_ = {};
    snapshot_.raw = raw;
    snapshot_.belowRange = raw < config_.rawMinimum;
    snapshot_.aboveRange = raw > config_.rawMaximum;
    snapshot_.normalized = normalize(raw);
    snapshot_.bipolar = snapshot_.normalized * 2.0F - 1.0F;
    initialized_ = true;
}

/*
 * Purpose: Processes one raw ADC sample.
 *
 * raw: Raw ADC code.
 */
void Potentiometer::update(const std::uint32_t raw) noexcept {
    if (!initialized_) {
        reset(raw);
        return;
    }

    snapshot_.raw = raw;
    snapshot_.belowRange = raw < config_.rawMinimum;
    snapshot_.aboveRange = raw > config_.rawMaximum;

    const float target = normalize(raw);
    const float filtered =
        snapshot_.normalized + (target - snapshot_.normalized) * config_.smoothingFactor;

    snapshot_.changed = std::fabs(filtered - snapshot_.normalized) >= config_.deadbandNormalized;

    if (snapshot_.changed) {
        snapshot_.normalized = filtered;
        snapshot_.bipolar = filtered * 2.0F - 1.0F;
    }
}

/*
 * Purpose: Returns the current immutable potentiometer state.
 *
 * Returns: Constant reference to the latest state.
 */
const PotentiometerSnapshot& Potentiometer::snapshot() const noexcept {
    return snapshot_;
}

/*
 * Purpose: Returns the immutable potentiometer configuration.
 *
 * Returns: Constant reference to the configuration.
 */
const PotentiometerConfig& Potentiometer::config() const noexcept {
    return config_;
}

/*
 * Purpose: Converts one raw ADC code into normalized position.
 *
 * raw: Raw ADC code.
 * Returns: Clamped and direction-corrected value from 0 to 1.
 */
float Potentiometer::normalize(const std::uint32_t raw) const noexcept {
    const std::uint32_t minimum = config_.rawMinimum;
    const std::uint32_t maximum = config_.rawMaximum > minimum ? config_.rawMaximum : minimum + 1U;
    const std::uint32_t clamped = raw < minimum ? minimum : (raw > maximum ? maximum : raw);

    float normalized =
        static_cast<float>(clamped - minimum) / static_cast<float>(maximum - minimum);

    if (config_.inverted) {
        normalized = 1.0F - normalized;
    }

    return normalized;
}

} // namespace eurorack::controls

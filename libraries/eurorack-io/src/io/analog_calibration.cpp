/**
 * @file src/io/analog_calibration.cpp
 * @brief Implements raw-code and voltage calibration.
 *
 * @details
 * Implements affine conversion between raw converter codes and engineering voltage.
 * Two-point calibration derives slope and offset from measured endpoints. Voltage-to-code
 * conversion reports clipping separately from the returned bounded code, allowing application code
 * to distinguish a valid endpoint request from an out-of-range command. Negative slopes are
 * supported for electrically inverted signal paths.
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
#include <eurorack/io/analog_calibration.hpp>

namespace eurorack::io {

LinearCodeCalibration::LinearCodeCalibration(const std::uint32_t maximumCode,
                                             const float voltsPerCode,
                                             const float offsetVolts) noexcept
    : maximumCode_(maximumCode), voltsPerCode_(voltsPerCode == 0.0F ? 1.0F : voltsPerCode),
      offsetVolts_(offsetVolts) {}

LinearCodeCalibration LinearCodeCalibration::fromTwoPoints(const std::uint32_t maximumCode,
                                                           const std::uint32_t firstCode,
                                                           const float firstVolts,
                                                           const std::uint32_t secondCode,
                                                           const float secondVolts) noexcept {
    if (firstCode == secondCode) {
        return {maximumCode, 1.0F, firstVolts - static_cast<float>(firstCode)};
    }

    const auto codeDifference =
        static_cast<std::int64_t>(secondCode) - static_cast<std::int64_t>(firstCode);
    const float slope = (secondVolts - firstVolts) / static_cast<float>(codeDifference);
    const float offset = firstVolts - slope * static_cast<float>(firstCode);

    return {maximumCode, slope, offset};
}

float LinearCodeCalibration::codeToVolts(const std::uint32_t code) const noexcept {
    const std::uint32_t clamped = std::min(code, maximumCode_);

    return static_cast<float>(clamped) * voltsPerCode_ + offsetVolts_;
}

CodeConversion LinearCodeCalibration::voltsToCode(const float volts) const noexcept {
    const float lower = minimumVolts();
    const float upper = maximumVolts();

    CodeConversion result{};
    result.belowRange = volts < lower;
    result.aboveRange = volts > upper;

    const float clamped = std::clamp(volts, lower, upper);
    const float raw = (clamped - offsetVolts_) / voltsPerCode_;
    const float rounded = std::round(raw);
    const float bounded = std::clamp(rounded, 0.0F, static_cast<float>(maximumCode_));

    result.code = static_cast<std::uint32_t>(bounded);
    return result;
}

std::uint32_t LinearCodeCalibration::maximumCode() const noexcept {
    return maximumCode_;
}

float LinearCodeCalibration::voltsPerCode() const noexcept {
    return voltsPerCode_;
}

float LinearCodeCalibration::offsetVolts() const noexcept {
    return offsetVolts_;
}

float LinearCodeCalibration::minimumVolts() const noexcept {
    return std::min(codeToVolts(0U), codeToVolts(maximumCode_));
}

float LinearCodeCalibration::maximumVolts() const noexcept {
    return std::max(codeToVolts(0U), codeToVolts(maximumCode_));
}

} // namespace eurorack::io

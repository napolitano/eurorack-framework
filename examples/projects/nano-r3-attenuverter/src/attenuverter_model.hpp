/**
 * @file attenuverter_model.hpp
 * @brief Pure signal-processing helpers for the attenuverter example.
 *
 * @details
 * This header contains no Arduino or framework dependencies. Keeping the
 * arithmetic separate from sampling, DAC transport, and LED output makes the
 * intended behavior easy to review and test.
 */

#pragma once

#include <cmath>

namespace attenuverter_example {

/**
 * @brief Clamps a floating-point value without relying on the full STL.
 *
 * @details
 * The classic Arduino AVR toolchain does not provide every C++17 standard
 * library header even though the compiler accepts C++17 language syntax.
 * Keeping this tiny helper local to the example avoids a dependency on
 * `<algorithm>` and `std::clamp`.
 *
 * @param value Value to constrain.
 * @param minimum Inclusive lower bound.
 * @param maximum Inclusive upper bound.
 * @return `minimum` when value is lower, `maximum` when value is higher, or the
 * original value when it already lies inside the range.
 */
[[nodiscard]] constexpr float
clampValue(const float value, const float minimum, const float maximum) noexcept {
    return value < minimum ? minimum : (value > maximum ? maximum : value);
}

/**
 * @brief Result of one attenuverter calculation.
 */
struct AttenuverterResult final {
    float inputVolts{0.0F};      ///< Calibrated input voltage.
    float attenuation{0.0F};     ///< Centered gain from -1.0 to +1.0.
    float balance{0.0F};         ///< Side balance from -1.0 to +1.0.
    float positiveScale{1.0F};   ///< Scale applied to positive intermediate values.
    float negativeScale{1.0F};   ///< Scale applied to negative intermediate values.
    float attenuatedVolts{0.0F}; ///< Input after signed attenuation.
    float outputVolts{0.0F};     ///< Output after asymmetric side scaling.
};

/**
 * @brief Applies a symmetric dead zone around zero.
 *
 * @details
 * Values inside the dead zone become exactly zero. Values outside it are
 * rescaled so that the remaining travel still reaches -1.0 and +1.0.
 *
 * @param value Input value, nominally in the inclusive range -1.0 to +1.0.
 * @param halfWidth Dead-zone half width in the range 0.0 to less than 1.0.
 * @return Dead-zone compensated value in the range -1.0 to +1.0.
 */
[[nodiscard]] inline float applyCenteredDeadZone(const float value,
                                                 const float halfWidth) noexcept {
    const float clampedValue = clampValue(value, -1.0F, 1.0F);
    const float clampedWidth = clampValue(halfWidth, 0.0F, 0.999F);
    const float magnitude = std::fabs(clampedValue);

    if (magnitude <= clampedWidth) {
        return 0.0F;
    }

    const float rescaled = (magnitude - clampedWidth) / (1.0F - clampedWidth);

    return std::copysign(rescaled, clampedValue);
}

/**
 * @brief Converts balance into independent positive and negative scales.
 *
 * @details
 * Center balance keeps both sides at full scale. Moving balance positive keeps
 * the positive side at full scale while reducing the negative side. Moving
 * balance negative keeps the negative side at full scale while reducing the
 * positive side.
 *
 * Examples:
 *
 * - balance  0.00 -> positive 1.00, negative 1.00
 * - balance +0.75 -> positive 1.00, negative 0.25
 * - balance -0.75 -> positive 0.25, negative 1.00
 *
 * @param balance Centered balance value from -1.0 to +1.0.
 * @param positiveScale Receives the positive-side scale.
 * @param negativeScale Receives the negative-side scale.
 */
inline void
calculateSideScales(const float balance, float& positiveScale, float& negativeScale) noexcept {
    const float clampedBalance = clampValue(balance, -1.0F, 1.0F);

    positiveScale = 1.0F;
    negativeScale = 1.0F;

    if (clampedBalance > 0.0F) {
        negativeScale = 1.0F - clampedBalance;
    } else if (clampedBalance < 0.0F) {
        positiveScale = 1.0F + clampedBalance;
    }
}

/**
 * @brief Calculates one attenuverter output sample.
 *
 * @param inputVolts Calibrated bipolar input voltage.
 * @param attenuation Signed attenuation from -1.0 to +1.0.
 * @param balance Side balance from -1.0 to +1.0.
 * @return Complete intermediate and final calculation state.
 */
[[nodiscard]] inline AttenuverterResult
process(const float inputVolts, const float attenuation, const float balance) noexcept {
    AttenuverterResult result{};
    result.inputVolts = inputVolts;
    result.attenuation = clampValue(attenuation, -1.0F, 1.0F);
    result.balance = clampValue(balance, -1.0F, 1.0F);

    calculateSideScales(result.balance, result.positiveScale, result.negativeScale);

    result.attenuatedVolts = result.inputVolts * result.attenuation;

    result.outputVolts = result.attenuatedVolts >= 0.0F
                             ? result.attenuatedVolts * result.positiveScale
                             : result.attenuatedVolts * result.negativeScale;

    return result;
}

} // namespace attenuverter_example

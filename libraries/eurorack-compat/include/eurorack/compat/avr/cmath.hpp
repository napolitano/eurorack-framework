/**
 * @file include/eurorack/compat/avr/cmath.hpp
 * @brief Provides the `std::` float math functions the framework uses, on AVR.
 *
 * @details
 * Covers exactly the functions used across the framework's implementation
 * files: `fabs`, `copysign`, `pow`, `round`, `lround`, `sin`, and `cos`. Extend this list if a new
 * framework source starts using another `<cmath>` function; do not widen it
 * speculatively. See `eurorack/compat/avr/cstdint.hpp` for background. Do not
 * include this header directly; include `<cmath>` as usual.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup compat_avr
 */

#pragma once

#include <math.h>

namespace std {

/**
 * @brief Returns the absolute value of a float.
 *
 * @param value Input value.
 * @return `value` with its sign removed.
 */
inline float fabs(const float value) noexcept {
    return ::fabsf(value);
}

/**
 * @brief Returns a value with the magnitude of one argument and the sign of another.
 *
 * @param magnitude Value supplying the magnitude.
 * @param sign Value whose sign is applied to the result.
 * @return `magnitude` with the sign of `sign`.
 */
inline float copysign(const float magnitude, const float sign) noexcept {
    return ::copysignf(magnitude, sign);
}

/**
 * @brief Raises a base to an exponent.
 *
 * @param base Base value.
 * @param exponent Exponent value.
 * @return `base` raised to the power of `exponent`.
 */
inline float pow(const float base, const float exponent) noexcept {
    return ::powf(base, exponent);
}

/**
 * @brief Rounds a float to the nearest integer value, rounding halfway cases away from zero.
 *
 * @param value Value to round.
 * @return Rounded value, still represented as `float`.
 */
inline float round(const float value) noexcept {
    return ::roundf(value);
}

/**
 * @brief Rounds a float to the nearest integer, rounding halfway cases away from zero.
 *
 * @param value Value to round.
 * @return Rounded value as a `long`.
 */
inline long lround(const float value) noexcept {
    return ::lroundf(value);
}

/**
 * @brief Computes the sine of an angle in radians.
 *
 * @param radians Angle in radians.
 * @return Sine of `radians`.
 */
inline float sin(const float radians) noexcept {
    return ::sinf(radians);
}

/**
 * @brief Computes the cosine of an angle in radians.
 *
 * @param radians Angle in radians.
 * @return Cosine of `radians`.
 */
inline float cos(const float radians) noexcept {
    return ::cosf(radians);
}

} // namespace std

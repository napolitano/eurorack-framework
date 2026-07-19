/**
 * @file include/eurorack/io/analog_calibration.hpp
 * @brief Declares reusable raw-code and voltage calibration models.
 *
 * @details
 * Provides affine and two-point conversion between converter codes and calibrated voltages.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup io
 */

#pragma once

#include <cstdint>

namespace eurorack::io {

/**
 * @brief Result of converting a voltage into a converter code.
 */
struct CodeConversion final {
    std::uint32_t code{0U};
    bool belowRange{false};
    bool aboveRange{false};
};

/**
 * @brief Affine conversion between converter code and voltage.
 */
class LinearCodeCalibration final {
  public:
    /**
     * @brief Constructs an affine calibration.
     *
     * @param maximumCode Inclusive maximum converter code.
     * @param voltsPerCode Voltage represented by one code step.
     * @param offsetVolts Voltage represented by code zero.
     */
    LinearCodeCalibration(std::uint32_t maximumCode = 4095U,
                          float voltsPerCode = 1.0F / 4095.0F,
                          float offsetVolts = 0.0F) noexcept;

    /**
     * @brief Creates a calibration from two measured points.
     *
     * @param maximumCode Inclusive maximum converter code.
     * @param firstCode First measured code.
     * @param firstVolts Voltage measured at firstCode.
     * @param secondCode Second measured code.
     * @param secondVolts Voltage measured at secondCode.
     * @return Derived affine calibration.
     */
    [[nodiscard]] static LinearCodeCalibration fromTwoPoints(std::uint32_t maximumCode,
                                                             std::uint32_t firstCode,
                                                             float firstVolts,
                                                             std::uint32_t secondCode,
                                                             float secondVolts) noexcept;

    /**
     * @brief Converts one converter code to voltage.
     *
     * @param code Raw converter code.
     * @return Calibrated voltage after code clamping.
     */
    [[nodiscard]] float codeToVolts(std::uint32_t code) const noexcept;

    /**
     * @brief Converts one voltage to the nearest converter code.
     *
     * @param volts Requested calibrated voltage.
     * @return Converter code and range diagnostics.
     */
    [[nodiscard]] CodeConversion voltsToCode(float volts) const noexcept;

    /** @brief Returns the inclusive maximum code. @return Maximum code. */
    [[nodiscard]] std::uint32_t maximumCode() const noexcept;

    /** @brief Returns voltage per code step. @return Volts per code. */
    [[nodiscard]] float voltsPerCode() const noexcept;

    /** @brief Returns voltage represented by code zero. @return Offset volts. */
    [[nodiscard]] float offsetVolts() const noexcept;

    /** @brief Returns the lowest calibrated voltage. @return Minimum volts. */
    [[nodiscard]] float minimumVolts() const noexcept;

    /** @brief Returns the highest calibrated voltage. @return Maximum volts. */
    [[nodiscard]] float maximumVolts() const noexcept;

  private:
    std::uint32_t maximumCode_{4095U};
    float voltsPerCode_{1.0F / 4095.0F};
    float offsetVolts_{0.0F};
};

} // namespace eurorack::io

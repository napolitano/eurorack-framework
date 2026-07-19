/**
 * @file include/eurorack/controls/potentiometer.hpp
 * @brief Declares a calibrated and optionally smoothed potentiometer model.
 *
 * @details
 * Converts raw ADC codes into normalized and bipolar values with inversion, deadband, and
 * smoothing.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#pragma once

#include <cstdint>

namespace eurorack::controls {

/**
 * @brief Configuration for a potentiometer.
 */
struct PotentiometerConfig final {
    std::uint32_t rawMinimum{0U};     ///< Raw ADC code representing minimum position.
    std::uint32_t rawMaximum{4095U};  ///< Raw ADC code representing maximum position.
    bool inverted{false};             ///< True to reverse logical direction.
    float deadbandNormalized{0.002F}; ///< Minimum normalized change to report.
    float smoothingFactor{1.0F};      ///< Exponential smoothing factor from 0 to 1.
};

/**
 * @brief Immutable potentiometer state.
 */
struct PotentiometerSnapshot final {
    std::uint32_t raw{0U};  ///< Latest raw ADC code.
    float normalized{0.0F}; ///< Smoothed position from 0 to 1.
    float bipolar{0.0F};    ///< Smoothed position from -1 to 1.
    bool changed{false};    ///< True when latest update exceeded deadband.
    bool belowRange{false}; ///< Raw value below configured minimum.
    bool aboveRange{false}; ///< Raw value above configured maximum.
};

/**
 * @brief Models a physical potentiometer independently from an ADC implementation.
 */
class Potentiometer final {
  public:
    /**
     * @brief Constructs an uninitialized potentiometer model.
     *
     * @param config Calibration, inversion, deadband, and smoothing settings.
     */
    explicit Potentiometer(PotentiometerConfig config = {}) noexcept;

    /**
     * @brief Resets the model from one raw ADC sample.
     *
     * @param raw Raw ADC code.
     */
    void reset(std::uint32_t raw) noexcept;

    /**
     * @brief Processes one raw ADC sample.
     *
     * @param raw Raw ADC code.
     */
    void update(std::uint32_t raw) noexcept;

    /**
     * @brief Returns the current immutable potentiometer state.
     *
     * @return Constant reference to the latest state.
     */
    [[nodiscard]] const PotentiometerSnapshot& snapshot() const noexcept;

    /**
     * @brief Returns the immutable potentiometer configuration.
     *
     * @return Constant reference to the configuration.
     */
    [[nodiscard]] const PotentiometerConfig& config() const noexcept;

  private:
    /**
     * @brief Converts one raw ADC code into normalized position.
     *
     * @param raw Raw ADC code.
     * @return Clamped and direction-corrected value from 0 to 1.
     */
    [[nodiscard]] float normalize(std::uint32_t raw) const noexcept;

    PotentiometerConfig config_{};
    PotentiometerSnapshot snapshot_{};
    bool initialized_{false};
};

} // namespace eurorack::controls

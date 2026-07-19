/**
 * @file include/eurorack/controls/cv.hpp
 * @brief Declares voltage-domain CV input and output models.
 *
 * @details
 * The models keep application logic in volts. Hardware adapters remain responsible for ADC and DAC
 * conversion, analog scaling, protection, and calibration.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * @see LICENSE and ADDITIONAL_PERMISSION.md in the repository root.
 *
 * @ingroup controls
 */

#pragma once

#include <eurorack/core/framework_config.hpp>

namespace eurorack::controls {

/**
 * @brief Immutable control-voltage input state.
 */
struct CvInputSnapshot final {
    float rawVolts{0.0F};   ///< Unclamped calibrated input voltage.
    float volts{0.0F};      ///< Voltage clamped to the configured operational range.
    float normalized{0.0F}; ///< Position within the configured range from 0.0 to 1.0.
    bool belowRange{false}; ///< True when rawVolts is below the configured minimum.
    bool aboveRange{false}; ///< True when rawVolts is above the configured maximum.
};

/**
 * @brief Models a calibrated CV input in volts.
 */
class CvInput final {
  public:
    /**
     * @brief Constructs a CV input model.
     *
     * @param range Accepted operational voltage range.
     */
    explicit CvInput(eurorack::core::VoltageRange range =
                         eurorack::core::eurorackDefaults.cv.inputVolts) noexcept;

    /**
     * @brief Processes one calibrated voltage sample.
     *
     * @param volts Measured voltage after hardware calibration.
     */
    void update(float volts) noexcept;

    /**
     * @brief Returns the current immutable CV-input state.
     *
     * @return Constant reference to the latest voltage-domain state.
     */
    [[nodiscard]] const CvInputSnapshot& snapshot() const noexcept;

  private:
    eurorack::core::VoltageRange range_{}; ///< Accepted operational range.
    CvInputSnapshot snapshot_{};           ///< Most recently calculated state.
};

/**
 * @brief Immutable control-voltage output state.
 */
struct CvOutputSnapshot final {
    float requestedVolts{0.0F}; ///< Voltage requested by application logic.
    float effectiveVolts{0.0F}; ///< Voltage after range limiting.
    bool clampedLow{false};     ///< True when requestedVolts is below the configured minimum.
    bool clampedHigh{false};    ///< True when requestedVolts is above the configured maximum.
};

/**
 * @brief Models a CV output request in volts.
 */
class CvOutput final {
  public:
    /**
     * @brief Constructs a CV output model.
     *
     * @param range Permitted output-voltage range.
     */
    explicit CvOutput(eurorack::core::VoltageRange range =
                          eurorack::core::eurorackDefaults.cv.outputVolts) noexcept;

    /**
     * @brief Requests an output voltage.
     *
     * @param volts Requested voltage.
     */
    void setVolts(float volts) noexcept;

    /**
     * @brief Returns the current immutable CV-output state.
     *
     * @return Constant reference to requested and effective output voltage.
     */
    [[nodiscard]] const CvOutputSnapshot& snapshot() const noexcept;

  private:
    eurorack::core::VoltageRange range_{}; ///< Permitted output range.
    CvOutputSnapshot snapshot_{};          ///< Most recent output request.
};

} // namespace eurorack::controls

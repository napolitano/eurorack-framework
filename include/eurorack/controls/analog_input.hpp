/**
 * @file include/eurorack/controls/analog_input.hpp
 * @brief Declares calibrated normalization for raw ADC samples.
 *
 * @details
 * The component converts integer ADC samples into normalized unipolar and bipolar values. It is
 * independent from any MCU ADC implementation and therefore usable by firmware, tests, and a
 * simulator.
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

#include <cstdint>

namespace eurorack::controls {

/**
 * @brief Calibration and direction settings for an analog input.
 */
struct AnalogInputConfig final {
    std::uint32_t rawMinimum{0U};    ///< ADC code representing the minimum value.
    std::uint32_t rawMaximum{4095U}; ///< ADC code representing the maximum value.
    bool inverted{false};            ///< True to reverse the normalized direction.
};

/**
 * @brief Immutable normalized analog-input state.
 */
struct AnalogInputSnapshot final {
    std::uint32_t raw{0U};  ///< Most recently supplied raw ADC code.
    float normalized{0.0F}; ///< Clamped value in the inclusive range 0.0 to 1.0.
    float bipolar{0.0F};    ///< Clamped value in the inclusive range -1.0 to 1.0.
    bool belowRange{false}; ///< True when raw is below rawMinimum.
    bool aboveRange{false}; ///< True when raw is above rawMaximum.
};

/**
 * @brief Converts raw ADC samples into normalized values.
 */
class AnalogInput final {
  public:
    /**
     * @brief Constructs an analog-input normalizer.
     *
     * @param config ADC calibration and optional direction inversion.
     */
    explicit AnalogInput(AnalogInputConfig config = {}) noexcept;

    /**
     * @brief Processes one raw ADC code.
     *
     * @param raw Raw ADC sample.
     *
     * Values outside the calibrated range are clamped while the corresponding diagnostic flag is
     * retained in the snapshot.
     */
    void update(std::uint32_t raw) noexcept;

    /**
     * @brief Returns the current immutable analog-input state.
     *
     * @return Constant reference to the latest normalized sample.
     */
    [[nodiscard]] const AnalogInputSnapshot& snapshot() const noexcept;

  private:
    AnalogInputConfig config_{};     ///< Immutable ADC calibration.
    AnalogInputSnapshot snapshot_{}; ///< Most recently calculated state.
};

} // namespace eurorack::controls

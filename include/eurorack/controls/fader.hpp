/**
 * @file include/eurorack/controls/fader.hpp
 * @brief Declares a calibrated linear fader model.
 *
 * @details
 * Wraps the potentiometer processor with fader-specific direction and endpoint terminology while
 * preserving normalized and bipolar output.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#pragma once

#include <cstdint>
#include <eurorack/controls/potentiometer.hpp>

namespace eurorack::controls {

/** @brief Physical direction interpreted as increasing fader value. */
enum class FaderDirection : std::uint8_t {
    BottomToTop, ///< Minimum at bottom and maximum at top.
    TopToBottom  ///< Maximum at bottom and minimum at top.
};

/** @brief Calibration and filtering settings for a fader. */
struct FaderConfig final {
    std::uint32_t rawMinimum{0U};                          ///< ADC code at physical minimum.
    std::uint32_t rawMaximum{4095U};                       ///< ADC code at physical maximum.
    FaderDirection direction{FaderDirection::BottomToTop}; ///< Logical direction.
    float deadbandNormalized{0.002F}; ///< Minimum accepted normalized movement.
    float smoothingFactor{1.0F};      ///< Exponential smoothing factor from zero to one.
};

/** @brief Immutable fader state. */
using FaderSnapshot = PotentiometerSnapshot;

/**
 * @brief Models a linear panel fader independently from the ADC backend.
 *
 * @details
 * Mechanically, a fader is a potentiometer. This wrapper supplies domain
 * terminology and leaves room for fader-specific behavior without duplicating
 * normalization logic. Call update() with raw ADC codes.
 */
class Fader final {
  public:
    /** @brief Constructs an uninitialized fader. @param config Calibration and filtering. */
    explicit Fader(FaderConfig config = {}) noexcept;

    /** @brief Resets from one raw ADC code. @param raw Raw ADC code. */
    void reset(std::uint32_t raw) noexcept;

    /** @brief Processes one raw ADC code. @param raw Raw ADC code. */
    void update(std::uint32_t raw) noexcept;

    /** @brief Returns latest fader state. @return Constant snapshot reference. */
    [[nodiscard]] const FaderSnapshot& snapshot() const noexcept;

    /** @brief Returns immutable fader settings. @return Constant configuration reference. */
    [[nodiscard]] const FaderConfig& config() const noexcept;

  private:
    FaderConfig config_{};
    Potentiometer potentiometer_{};
};

} // namespace eurorack::controls

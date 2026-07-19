/**
 * @file include/eurorack/controls/illuminated_fader.hpp
 * @brief Declares a linear fader with integrated RGB illumination.
 *
 * @details
 * Composes Fader and MulticolorLed and optionally scales LED brightness from the normalized fader
 * position.
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
#include <eurorack/controls/fader.hpp>
#include <eurorack/controls/multicolor_led.hpp>

namespace eurorack::controls {

/** @brief Automatic illumination mode for an LED fader. */
enum class IlluminatedFaderMode : std::uint8_t {
    Manual,         ///< Color and brightness are controlled explicitly.
    FollowPosition, ///< Master brightness follows normalized fader position.
    InversePosition ///< Master brightness follows one minus normalized position.
};

/** @brief Configuration of an illuminated fader. */
struct IlluminatedFaderConfig final {
    FaderConfig fader{};                                     ///< Fader calibration and filtering.
    IlluminatedFaderMode mode{IlluminatedFaderMode::Manual}; ///< Automatic brightness behavior.
    RgbColor color{0U, 0U, 65535U};                          ///< Color used by automatic modes.
    std::uint16_t minimumBrightness{0U};     ///< Brightness at the dark end of travel.
    std::uint16_t maximumBrightness{65535U}; ///< Brightness at the bright end of travel.
};

/**
 * @brief Composite fader and RGB illumination model.
 *
 * @details
 * Automatic modes update LED master brightness only when update() or reset()
 * processes a fader sample. Color remains configurable independently. The
 * model owns both subobjects and performs no dynamic allocation.
 */
class IlluminatedFader final {
  public:
    /** @brief Constructs the composite control. @param config Fader and illumination settings. */
    explicit IlluminatedFader(IlluminatedFaderConfig config = {}) noexcept;

    /** @brief Resets from one ADC sample. @param raw Raw ADC code. */
    void reset(std::uint32_t raw) noexcept;

    /** @brief Processes one ADC sample. @param raw Raw ADC code. */
    void update(std::uint32_t raw) noexcept;

    /** @brief Sets LED color. @param color Requested RGB color. */
    void setLedColor(RgbColor color) noexcept;

    /** @brief Sets LED brightness directly. @param brightness Inclusive 16-bit brightness. */
    void setLedBrightness(std::uint16_t brightness) noexcept;

    /** @brief Returns fader submodel. @return Constant fader reference. */
    [[nodiscard]] const Fader& fader() const noexcept;

    /** @brief Returns LED submodel. @return Constant LED reference. */
    [[nodiscard]] const MulticolorLed& led() const noexcept;

  private:
    /** @brief Maps current normalized position to configured LED brightness. */
    void updateAutomaticLed() noexcept;

    IlluminatedFaderConfig config_{}; ///< Mode, color, and brightness range used by
                                      ///< @ref updateAutomaticLed.
    Fader fader_{};                   ///< Underlying calibrated linear-fader submodel.
    MulticolorLed led_{};             ///< RGB illumination submodel.
};

} // namespace eurorack::controls

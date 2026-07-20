/**
 * @file include/eurorack/controls/multicolor_led.hpp
 * @brief Declares an allocation-free RGB multicolor LED model.
 *
 * @details
 * Stores independent 16-bit red, green, and blue intensities plus a master brightness, producing
 * effective channel values suitable for generic LED drivers.
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

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/drivers/led/led_driver.hpp>

namespace eurorack::controls {

/** @brief Logical 16-bit RGB color. */
struct RgbColor final {
    std::uint16_t red{0U};   ///< Red intensity.
    std::uint16_t green{0U}; ///< Green intensity.
    std::uint16_t blue{0U};  ///< Blue intensity.
};

/** @brief Immutable state of a multicolor LED. */
struct MulticolorLedSnapshot final {
    RgbColor requested{}; ///< Requested unscaled RGB color.
    RgbColor effective{}; ///< RGB color after master-brightness scaling.
    std::uint16_t masterBrightness{
        eurorack::drivers::led::MAX_BRIGHTNESS}; ///< Global intensity scale.
    bool changed{false}; ///< True when the most recent setter changed effective output.
    std::uint32_t transitionCount{0U}; ///< Number of accepted effective-output changes.
};

/**
 * @brief Platform-independent RGB LED state model.
 *
 * @details
 * This model expresses logical brightness only. Current limiting, common-anode
 * or common-cathode wiring, PWM frequency, gamma correction, and optical color
 * calibration belong to hardware adapters or LED-driver implementations.
 */
class MulticolorLed final {
  public:
    /** @brief Constructs an RGB LED. @param initialColor Initial logical color. @param
     * masterBrightness Initial global intensity. */
    explicit MulticolorLed(
        RgbColor initialColor = {},
        std::uint16_t masterBrightness = eurorack::drivers::led::MAX_BRIGHTNESS) noexcept;

    /** @brief Restores the configured initial state and clears history. */
    void reset() noexcept;

    /** @brief Sets the requested RGB color. @param color New unscaled color. */
    void setColor(RgbColor color) noexcept;

    /** @brief Sets global brightness. @param brightness Inclusive 16-bit brightness. */
    void setMasterBrightness(std::uint16_t brightness) noexcept;

    /** @brief Turns all effective channels off while retaining no requested color. */
    void turnOff() noexcept;

    /**
     * @brief Writes effective channels to three generic LED channels.
     * @param red Destination red channel.
     * @param green Destination green channel.
     * @param blue Destination blue channel.
     */
    void applyTo(eurorack::drivers::led::LedChannel& red,
                 eurorack::drivers::led::LedChannel& green,
                 eurorack::drivers::led::LedChannel& blue) const noexcept;

    /** @brief Returns latest state. @return Constant snapshot reference. */
    [[nodiscard]] const MulticolorLedSnapshot& snapshot() const noexcept;

  private:
    /** @brief Recalculates effective RGB output and transition metadata. */
    void recalculate() noexcept;

    RgbColor initialColor_{}; ///< Color restored by @ref reset; set once at
                              ///< construction.
    std::uint16_t initialMasterBrightness_{
        eurorack::drivers::led::MAX_BRIGHTNESS}; ///< Master
                                                 ///< brightness restored by @ref reset; set once at
                                                 ///< construction.
    MulticolorLedSnapshot snapshot_{}; ///< Most recently calculated requested/effective color and
                                       ///< transition metadata.
};

} // namespace eurorack::controls

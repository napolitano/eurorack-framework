/**
 * @file include/eurorack/controls/illuminated_button.hpp
 * @brief Declares a momentary pushbutton with an integrated RGB LED.
 *
 * @details
 * Composes the existing debounced button model with the multicolor LED model while keeping input
 * and illumination state independently controllable.
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
#include <eurorack/controls/momentary_button.hpp>
#include <eurorack/controls/multicolor_led.hpp>

namespace eurorack::controls {

/** @brief Automatic illumination behavior of an LED pushbutton. */
enum class IlluminatedButtonMode : std::uint8_t {
    Manual,          ///< LED state is controlled only by explicit setters.
    LitWhilePressed, ///< LED uses activeColor while pressed and inactiveColor otherwise.
    LitWhileReleased ///< LED uses inactiveColor while pressed and activeColor otherwise.
};

/** @brief Configuration for an illuminated momentary button. */
struct IlluminatedButtonConfig final {
    MomentaryButtonConfig button{};                            ///< Electrical button settings.
    IlluminatedButtonMode mode{IlluminatedButtonMode::Manual}; ///< Automatic LED behavior.
    RgbColor activeColor{0U, 65535U, 0U}; ///< Color used by the active automatic state.
    RgbColor inactiveColor{};             ///< Color used by the inactive automatic state.
};

/**
 * @brief Composite LED pushbutton model.
 *
 * @details
 * Input sampling and illumination are separate concerns. Manual mode never
 * changes LED state during update(). Automatic modes recalculate the LED after
 * every button sample. The object owns both submodels and allocates no memory.
 */
class IlluminatedButton final {
  public:
    /** @brief Constructs the composite control. @param config Button and illumination settings. */
    explicit IlluminatedButton(IlluminatedButtonConfig config = {}) noexcept;

    /** @brief Resets button and automatic illumination. @param rawLevelHigh Electrical button
     * level. @param nowMs Monotonic milliseconds. */
    void reset(bool rawLevelHigh, std::uint32_t nowMs) noexcept;

    /** @brief Processes one button sample and updates automatic illumination. @param rawLevelHigh
     * Electrical button level. @param nowMs Monotonic milliseconds. */
    void update(bool rawLevelHigh, std::uint32_t nowMs) noexcept;

    /** @brief Sets LED color, including in Manual mode. @param color Requested RGB color. */
    void setLedColor(RgbColor color) noexcept;

    /** @brief Sets LED master brightness. @param brightness Inclusive 16-bit intensity. */
    void setLedBrightness(std::uint16_t brightness) noexcept;

    /** @brief Returns button submodel. @return Constant button reference. */
    [[nodiscard]] const MomentaryButton& button() const noexcept;

    /** @brief Returns LED submodel. @return Constant LED reference. */
    [[nodiscard]] const MulticolorLed& led() const noexcept;

  private:
    /** @brief Applies configured automatic LED behavior. */
    void updateAutomaticLed() noexcept;

    IlluminatedButtonConfig config_{}; ///< Mode and colors used by @ref updateAutomaticLed.
    MomentaryButton button_{};         ///< Debounced electrical button submodel.
    MulticolorLed led_{};              ///< RGB illumination submodel.
};

} // namespace eurorack::controls

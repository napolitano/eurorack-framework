/**
 * @file include/eurorack/controls/toggle_switch.hpp
 * @brief Declares a debounced two-position latching toggle switch.
 *
 * @details
 * Converts a sampled electrical level into stable Off and On positions with edge flags and
 * transition counting. The model is allocation-free and contains no hardware dependency.
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

namespace eurorack::controls {

/** @brief Stable position of a two-position latching switch. */
enum class ToggleSwitchPosition : std::uint8_t {
    Off, ///< Logical off position.
    On   ///< Logical on position.
};

/** @brief Configuration of a two-position toggle switch. */
struct ToggleSwitchConfig final {
    ActiveLevel activeLevel{ActiveLevel::High}; ///< Electrical level representing On.
    std::uint32_t debounceTimeMs{10U};          ///< Required stable time in milliseconds.
};

/** @brief Immutable state produced by ToggleSwitch. */
struct ToggleSwitchSnapshot final {
    ToggleSwitchPosition position{ToggleSwitchPosition::Off}; ///< Current stable position.
    bool justTurnedOn{false};          ///< True only for the update accepting Off to On.
    bool justTurnedOff{false};         ///< True only for the update accepting On to Off.
    std::uint32_t transitionCount{0U}; ///< Accepted position transitions since reset.
};

/**
 * @brief Debounces a maintained two-position switch.
 *
 * @details
 * The switch model does not sample hardware itself. Call update() from the
 * cooperative firmware loop with a raw electrical level and monotonic time.
 * Event flags remain valid until the next update(). The object allocates no
 * memory and is not internally synchronized.
 */
class ToggleSwitch final {
  public:
    /** @brief Constructs a switch model. @param config Polarity and debounce settings. */
    explicit ToggleSwitch(ToggleSwitchConfig config = {}) noexcept;

    /**
     * @brief Resets to a known electrical level without producing an event.
     * @param rawLevelHigh True when the sampled input is electrically high.
     * @param nowMs Current monotonic time in milliseconds.
     */
    void reset(bool rawLevelHigh, std::uint32_t nowMs) noexcept;

    /**
     * @brief Processes one electrical sample.
     * @param rawLevelHigh True when the sampled input is electrically high.
     * @param nowMs Current monotonic time in milliseconds.
     */
    void update(bool rawLevelHigh, std::uint32_t nowMs) noexcept;

    /** @brief Returns the latest stable state. @return Constant snapshot reference. */
    [[nodiscard]] const ToggleSwitchSnapshot& snapshot() const noexcept;

    /** @brief Reports whether the switch is in On. @return True in On. */
    [[nodiscard]] bool isOn() const noexcept;

    /** @brief Returns immutable configuration. @return Constant configuration reference. */
    [[nodiscard]] const ToggleSwitchConfig& config() const noexcept;

  private:
    ToggleSwitchConfig config_{};       ///< Polarity and debounce settings.
    MomentaryButton debouncer_{}; ///< Underlying debounce state machine; On maps to the
                                    ///< debouncer's pressed state and its edge/transition
                                    ///< reporting is reused directly.
    ToggleSwitchSnapshot snapshot_{}; ///< Most recently derived stable position and edge flags.
};

} // namespace eurorack::controls

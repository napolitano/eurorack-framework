/**
 * @file include/eurorack/controls/digital_led.hpp
 * @brief Declares a platform-independent binary LED state model.
 *
 * @details
 * The component stores the logical on/off state of a simple LED, emits one-update transition flags,
 * and translates the logical state into the electrical output level required by active-high or
 * active-low wiring. It contains no GPIO, Arduino, display, or operating-system dependency.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @license PolyForm Noncommercial License 1.0.0
 *
 * This file is part of Eurorack Framework. Noncommercial use, modification,
 * and redistribution are permitted under the terms in the repository-root
 * LICENSE file. The separate ADDITIONAL_PERMISSION.md file permits limited
 * five-unit, cost-recovery distribution under its stated conditions.
 * Commercial use requires prior written permission.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#pragma once

#include <cstdint>

namespace eurorack::controls {

/**
 * @brief Electrical level that turns a digital LED on.
 */
enum class LedActiveLevel : std::uint8_t {
    Low, ///< A low output level turns the LED on.
    High ///< A high output level turns the LED on.
};

/**
 * @brief Configuration for a binary LED.
 */
struct DigitalLedConfig final {
    /**
     * @brief Electrical output level that turns the LED on.
     *
     * Active-high is typical when the GPIO sources current through a resistor
     * and LED to ground. Active-low is common when the GPIO sinks current.
     */
    LedActiveLevel activeLevel{LedActiveLevel::High};

    /**
     * @brief Logical state established by construction and reset().
     */
    bool initiallyOn{false};
};

/**
 * @brief Immutable state exposed to application logic, renderers, and simulators.
 */
struct DigitalLedSnapshot final {
    bool on{false};            ///< Current logical LED state.
    bool justTurnedOn{false};  ///< True only after an accepted off-to-on change.
    bool justTurnedOff{false}; ///< True only after an accepted on-to-off change.

    /**
     * @brief Electrical level that a hardware adapter must write to the pin.
     *
     * True means a high output level. False means a low output level.
     */
    bool rawOutputHigh{false};

    /**
     * @brief Number of accepted logical state transitions since reset().
     *
     * Repeatedly setting the already active state does not increment the count.
     * The unsigned counter wraps naturally at UINT32_MAX.
     */
    std::uint32_t transitionCount{0U};
};

/**
 * @brief Models a simple, non-PWM, single-color LED.
 *
 * The class owns logical LED state only. A platform adapter writes
 * snapshot().rawOutputHigh to the physical GPIO. A display or simulator reads
 * snapshot().on and the transition flags. This separation keeps all consumers
 * synchronized without binding the control to a particular platform.
 */
class DigitalLed final {
  public:
    /**
     * @brief Constructs an LED using the configured electrical polarity and state.
     *
     * @param config Electrical active level and initial logical state.
     *
     * The constructor performs no GPIO access. The initial raw output level is
     * calculated immediately and can be read through rawOutputHigh().
     */
    explicit constexpr DigitalLed(DigitalLedConfig config = {}) noexcept
        : config_(config), snapshot_{config.initiallyOn,
                                     false,
                                     false,
                                     toRawOutputHigh(config.initiallyOn, config.activeLevel),
                                     0U} {}

    /**
     * @brief Restores the configured initial logical state.
     *
     * Transition flags and the transition counter are cleared.
     */
    void reset() noexcept;

    /**
     * @brief Sets the logical LED state.
     *
     * @param on True to turn the LED on, false to turn it off.
     *
     * Calling setOn() with the current state clears the one-call transition
     * flags but does not increment transitionCount.
     */
    void setOn(bool on) noexcept;

    /**
     * @brief Changes the logical LED state to on.
     *
     * This is equivalent to setOn(true). A transition event and counter update
     * occur only when the LED was previously off.
     */
    void turnOn() noexcept;

    /**
     * @brief Changes the logical LED state to off.
     *
     * This is equivalent to setOn(false). A transition event and counter update
     * occur only when the LED was previously on.
     */
    void turnOff() noexcept;

    /**
     * @brief Inverts the current logical LED state.
     *
     * The method always produces exactly one accepted logical transition and
     * updates the electrical output level accordingly.
     */
    void toggle() noexcept;

    /**
     * @brief Returns the current immutable LED snapshot.
     *
     * @return Constant reference to the logical state, transition flags,
     *         electrical output level, and transition count.
     *
     * The returned reference remains valid for the lifetime of this object.
     */
    [[nodiscard]] constexpr const DigitalLedSnapshot& snapshot() const noexcept {
        return snapshot_;
    }

    /**
     * @brief Reports the current logical LED state.
     *
     * @return True when the LED is logically on; otherwise false.
     *
     * This value is independent of whether the wiring is active-high or
     * active-low.
     */
    [[nodiscard]] constexpr bool isOn() const noexcept {
        return snapshot_.on;
    }

    /**
     * @brief Returns the electrical output level required by the configured wiring.
     *
     * @return True when a hardware adapter must drive the GPIO high; false when
     *         it must drive the GPIO low.
     *
     * The value already accounts for active-high or active-low configuration.
     */
    [[nodiscard]] constexpr bool rawOutputHigh() const noexcept {
        return snapshot_.rawOutputHigh;
    }

    /**
     * @brief Returns the immutable configuration supplied at construction.
     *
     * @return Constant reference to the LED's electrical active level and
     *         configured initial logical state.
     *
     * The returned reference remains valid for the lifetime of this object.
     */
    [[nodiscard]] constexpr const DigitalLedConfig& config() const noexcept {
        return config_;
    }

  private:
    /**
     * @brief Converts a logical LED state into the required electrical level.
     *
     * @param on Logical LED state.
     * @param activeLevel Electrical level that turns the LED on.
     * @return True when the output pin must be high; otherwise false.
     *
     * This helper is pure, deterministic, and performs no GPIO access.
     */
    [[nodiscard]] static constexpr bool toRawOutputHigh(bool on,
                                                        LedActiveLevel activeLevel) noexcept {
        return activeLevel == LedActiveLevel::High ? on : !on;
    }

    DigitalLedConfig config_{};     ///< Immutable polarity and initial-state settings.
    DigitalLedSnapshot snapshot_{}; ///< Most recently calculated public LED state.
};

} // namespace eurorack::controls

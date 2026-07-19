/**
 * @file include/eurorack/controls/momentary_button.hpp
 * @brief Declares a platform-independent debounced momentary pushbutton.
 *
 * @details
 * The component converts a sampled electrical level and a monotonic millisecond timestamp into a
 * stable pressed state, edge events, and a read-only snapshot suitable for application logic,
 * displays, tests, and future user-interface simulators. It deliberately contains no GPIO, Arduino,
 * display, or operating-system dependencies.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
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
#include <eurorack/core/framework_config.hpp>

namespace eurorack::controls {

/**
 * @brief Electrical level that represents an asserted input.
 */
enum class ActiveLevel : std::uint8_t {
    Low, ///< A low electrical level means that the button is pressed.
    High ///< A high electrical level means that the button is pressed.
};

/**
 * @brief Configuration for a momentary pushbutton.
 */
struct MomentaryButtonConfig final {
    /**
     * @brief Electrical level that represents the pressed state.
     *
     * Active-low is common when a tactile switch connects the input to ground
     * and the microcontroller input uses a pull-up resistor.
     */
    ActiveLevel activeLevel{ActiveLevel::Low};

    /**
     * @brief Required stable time before an electrical transition is accepted.
     *
     * Unit: milliseconds.
     *
     * Valid range: 0 to UINT32_MAX. A value of zero accepts a transition on the
     * first update call that observes it. Typical mechanical switches work well
     * with values between 5 and 30 milliseconds.
     */
    std::uint32_t debounceTimeMs{eurorack::core::eurorackDefaults.interaction.buttonDebounceMs};

    /**
     * @brief Creates button settings from a complete framework configuration.
     *
     * @param frameworkConfig Central project configuration providing the default
     *        debounce interval.
     * @param level Electrical level that represents the pressed state.
     * @return Button-specific configuration containing the supplied polarity and
     *         the project's central debounce interval.
     *
     * The method copies values and does not retain a reference to the framework
     * configuration.
     */
    [[nodiscard]] static constexpr MomentaryButtonConfig
    fromFramework(const eurorack::core::FrameworkConfig& frameworkConfig,
                  const ActiveLevel level = ActiveLevel::Low) noexcept {
        return {level, frameworkConfig.interaction.buttonDebounceMs};
    }
};

/**
 * @brief Immutable application-facing state of a momentary pushbutton.
 *
 * A snapshot contains both the current stable state and edge events generated
 * by the most recent call to MomentaryButton::update().
 */
struct MomentaryButtonSnapshot final {
    bool pressed{false};      ///< True while the debounced button is pressed.
    bool justPressed{false};  ///< True only for the update that accepts a press.
    bool justReleased{false}; ///< True only for the update that accepts a release.

    /**
     * @brief Duration of the current accepted press.
     *
     * Unit: milliseconds. The value is zero while released and on the update
     * that accepts the press. Unsigned arithmetic keeps the duration valid
     * across a 32-bit timer wraparound.
     */
    std::uint32_t heldForMs{0U};

    /**
     * @brief Duration of the most recently completed accepted press.
     *
     * Unit: milliseconds. The value updates when a release is accepted and
     * remains available until the next accepted release or reset. This is
     * useful for application logic, display rendering, and simulation traces.
     */
    std::uint32_t lastPressDurationMs{0U};

    /**
     * @brief Number of accepted stable transitions since reset.
     *
     * The counter increments once for every accepted press and once for every
     * accepted release. It wraps naturally at UINT32_MAX.
     */
    std::uint32_t transitionCount{0U};
};

/**
 * @brief Debounces a non-latching, momentary pushbutton.
 *
 * The class is deterministic, allocation-free, and platform-independent.
 * Hardware adapters, application code, tests, and simulators provide the raw
 * electrical level and the current monotonic time through update().
 *
 * Event flags are valid until the next update() call. Code that needs queued
 * events should copy snapshots into an application-owned event queue.
 */
class MomentaryButton final {
  public:
    /**
     * @brief Constructs an uninitialized button state machine.
     *
     * @param config Electrical polarity and debounce configuration.
     *
     * The first update() call initializes the stable state without reporting a
     * synthetic press or release event. Call reset() explicitly when the
     * initial electrical level is already known.
     */
    explicit constexpr MomentaryButton(MomentaryButtonConfig config = {}) noexcept
        : config_(config) {}

    /**
     * @brief Resets the state machine to a known electrical level.
     *
     * @param rawLevelHigh True when the sampled electrical input is high.
     * @param nowMs Current monotonic time in milliseconds.
     *
     * No edge event is generated. The transition counter and held duration are
     * reset. This function is suitable for initialization and test setup.
     */
    void reset(bool rawLevelHigh, std::uint32_t nowMs) noexcept;

    /**
     * @brief Processes one electrical sample.
     *
     * @param rawLevelHigh True when the sampled electrical input is high.
     * @param nowMs Current monotonic time in milliseconds.
     * Call this function regularly. The sampling interval should be shorter
     * than debounceTimeMs for responsive operation, but correctness does not
     * depend on a fixed update frequency.
     *
     * The function mutates the internal state machine and intentionally returns
     * no value. Read the resulting state through snapshot() or the focused
     * accessors. This avoids forcing callers to consume a value when they only
     * need to advance the control state.
     */
    void update(bool rawLevelHigh, std::uint32_t nowMs) noexcept;

    /**
     * @brief Returns the most recently calculated snapshot.
     *
     * @return Current stable state and most recent edge flags.
     */
    [[nodiscard]] constexpr const MomentaryButtonSnapshot& snapshot() const noexcept {
        return snapshot_;
    }

    /**
     * @brief Reports the current stable logical button state.
     *
     * @return True while the debounced button is pressed; otherwise false.
     *
     * This accessor does not sample hardware and does not modify event flags.
     */
    [[nodiscard]] constexpr bool isPressed() const noexcept {
        return snapshot_.pressed;
    }

    /**
     * @brief Reports whether the most recent update accepted a press transition.
     *
     * @return True only between the update() call that accepted the press and
     *         the next update() call; otherwise false.
     *
     * Reading this value does not consume or clear the event.
     */
    [[nodiscard]] constexpr bool wasPressed() const noexcept {
        return snapshot_.justPressed;
    }

    /**
     * @brief Reports whether the most recent update accepted a release transition.
     *
     * @return True only between the update() call that accepted the release and
     *         the next update() call; otherwise false.
     *
     * Reading this value does not consume or clear the event.
     */
    [[nodiscard]] constexpr bool wasReleased() const noexcept {
        return snapshot_.justReleased;
    }

    /**
     * @brief Returns the duration of the current accepted press.
     *
     * @return Duration in milliseconds, or zero while released.
     */
    [[nodiscard]] constexpr std::uint32_t heldForMs() const noexcept {
        return snapshot_.heldForMs;
    }

    /**
     * @brief Returns the immutable configuration supplied at construction.
     *
     * @return Constant reference to the button's polarity and debounce settings.
     *
     * The returned reference remains valid for the lifetime of this object.
     */
    [[nodiscard]] constexpr const MomentaryButtonConfig& config() const noexcept {
        return config_;
    }

  private:
    /**
     * @brief Converts a sampled electrical level into the logical pressed state.
     *
     * @param rawLevelHigh True when the sampled electrical input is high.
     * @return True when the configured active level considers the button
     *         pressed; otherwise false.
     *
     * This helper performs no debounce and has no side effects.
     */
    [[nodiscard]] constexpr bool toLogicalPressed(bool rawLevelHigh) const noexcept {
        return config_.activeLevel == ActiveLevel::High ? rawLevelHigh : !rawLevelHigh;
    }

    MomentaryButtonConfig config_{};     ///< Immutable polarity and debounce settings.
    MomentaryButtonSnapshot snapshot_{}; ///< Most recently calculated public state.

    bool initialized_{false};            ///< True after reset() or the first update() call.
    bool candidatePressed_{false};       ///< Most recently sampled logical candidate state.
    std::uint32_t candidateSinceMs_{0U}; ///< Time at which the candidate state began.
    std::uint32_t pressedSinceMs_{0U};   ///< Time at which the current press was accepted.
};

} // namespace eurorack::controls

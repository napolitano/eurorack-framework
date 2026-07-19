/**
 * @file include/eurorack/core/framework_config.hpp
 * @brief Defines central, project-overridable electrical and interaction limits.
 *
 * @details
 * The configuration types provide conservative Eurorack defaults for CV, pitch CV, gates, triggers,
 * button timing, and simulator timing. They are plain constexpr-capable value types so consuming
 * firmware can copy the default configuration, override selected fields, validate the result, and
 * pass the same configuration to controls, drivers, displays, tests, and future UI simulators.
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
 * @ingroup core
 */

#pragma once

#include <cstdint>

/**
 * @def EURORACK_FRAMEWORK_CONFIG_FILE
 * @brief Selects the project-specific compile-time configuration header.
 *
 * @details
 * Consuming firmware may define this macro to an angle-bracket header token
 * before including framework headers. When it is not defined, the framework
 * loads the repository default `eurorack_config.hpp`.
 */
#ifndef EURORACK_FRAMEWORK_CONFIG_FILE
#define EURORACK_FRAMEWORK_CONFIG_FILE <eurorack_config.hpp>
#endif

#include EURORACK_FRAMEWORK_CONFIG_FILE

namespace eurorack::core {

/**
 * @brief Closed floating-point voltage interval.
 */
struct VoltageRange final {
    float minimumVolts{-10.0F}; ///< Inclusive lower operational limit in volts.
    float maximumVolts{10.0F};  ///< Inclusive upper operational limit in volts.

    /**
     * @brief Reports whether the interval has an ordered pair of bounds.
     *
     * @return True when minimumVolts is less than or equal to maximumVolts;
     *         otherwise false.
     *
     * The method performs no normalization and does not reject equal bounds.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return minimumVolts <= maximumVolts;
    }

    /**
     * @brief Reports whether a voltage lies inside the closed interval.
     *
     * @param volts Voltage to test, in volts.
     * @return True when volts is greater than or equal to minimumVolts and less
     *         than or equal to maximumVolts; otherwise false.
     *
     * Calling this method on an invalid range returns an implementation-defined
     * logical result and should be avoided by validating the configuration.
     */
    [[nodiscard]] constexpr bool contains(const float volts) const noexcept {
        return volts >= minimumVolts && volts <= maximumVolts;
    }

    /**
     * @brief Limits a voltage to the nearest bound of the interval.
     *
     * @param volts Voltage to clamp, in volts.
     * @return minimumVolts when volts is below the range, maximumVolts when it
     *         is above the range, or volts unchanged when it is inside.
     *
     * The method has no side effects. The range must be valid.
     */
    [[nodiscard]] constexpr float clamp(const float volts) const noexcept {
        return volts < minimumVolts ? minimumVolts : (volts > maximumVolts ? maximumVolts : volts);
    }

    /**
     * @brief Calculates the width of the interval.
     *
     * @return maximumVolts minus minimumVolts, in volts.
     *
     * A negative result indicates an invalid range.
     */
    [[nodiscard]] constexpr float spanVolts() const noexcept {
        return maximumVolts - minimumVolts;
    }
};

/**
 * @brief Operational voltage limits for continuous control voltages.
 */
struct CvLimits final {
    /**
     * @brief Nominal audio voltage interval.
     *
     * The default is -5 V to +5 V, corresponding to approximately 10 V
     * peak-to-peak, a common Eurorack audio level.
     */
    VoltageRange audioVolts{eurorack_config::AUDIO_MIN_VOLTS, eurorack_config::AUDIO_MAX_VOLTS};

    /**
     * @brief Expected usable voltage interval at a protected CV input.
     *
     * The default of -10 V to +10 V is deliberately broad enough for common
     * bipolar modulation while remaining an operational range, not a statement
     * of absolute hardware survivability.
     */
    VoltageRange inputVolts{eurorack_config::CV_INPUT_MIN_VOLTS,
                            eurorack_config::CV_INPUT_MAX_VOLTS};

    /**
     * @brief Voltage interval the application may request from a CV output.
     *
     * The physical DAC and output stage may support a smaller range. Drivers
     * must validate or clamp against both this project limit and hardware limits.
     */
    VoltageRange outputVolts{eurorack_config::CV_OUTPUT_MIN_VOLTS,
                             eurorack_config::CV_OUTPUT_MAX_VOLTS};

    /**
     * @brief Intended pitch-CV interval.
     *
     * Zero to ten volts represents ten octaves at the default one volt per
     * octave scaling. Projects may select a bipolar range when required.
     */
    VoltageRange pitchInputVolts{eurorack_config::PITCH_INPUT_MIN_VOLTS,
                                 eurorack_config::PITCH_INPUT_MAX_VOLTS};

    /**
     * @brief Intended generated pitch-CV interval.
     */
    VoltageRange pitchOutputVolts{eurorack_config::PITCH_OUTPUT_MIN_VOLTS,
                                  eurorack_config::PITCH_OUTPUT_MAX_VOLTS};

    /**
     * @brief Pitch scaling in volts per octave.
     *
     * The Eurorack convention is 1 V/octave.
     */
    float voltsPerOctave{eurorack_config::PITCH_VOLTS_PER_OCTAVE};

    /**
     * @brief Number of equal divisions used per octave for chromatic pitch.
     */
    std::uint16_t semitonesPerOctave{eurorack_config::PITCH_STEPS_PER_OCTAVE};
};

/**
 * @brief Voltage thresholds and output levels for gates and triggers.
 */
struct GateLimits final {
    /**
     * @brief Highest input voltage guaranteed to be interpreted as logically low.
     */
    float inputLowMaximumVolts{eurorack_config::GATE_INPUT_LOW_MAX_VOLTS};

    /**
     * @brief Lowest input voltage guaranteed to be interpreted as logically high.
     */
    float inputHighMinimumVolts{eurorack_config::GATE_INPUT_HIGH_MIN_VOLTS};

    /**
     * @brief Nominal voltage emitted for a logical low output.
     */
    float outputLowVolts{eurorack_config::GATE_OUTPUT_LOW_VOLTS};

    /**
     * @brief Nominal voltage emitted for a logical high output.
     *
     * Five volts is broadly compatible with Eurorack gate and trigger inputs.
     */
    float outputHighVolts{eurorack_config::GATE_OUTPUT_HIGH_VOLTS};

    /**
     * @brief Default trigger pulse duration in milliseconds.
     */
    std::uint32_t triggerPulseMs{eurorack_config::TRIGGER_PULSE_MILLISECONDS};
};

/**
 * @brief Human-interface timing defaults shared by controls and simulators.
 */
struct InteractionTiming final {
    /**
     * @brief Default mechanical button debounce interval in milliseconds.
     */
    std::uint32_t buttonDebounceMs{eurorack_config::BUTTON_DEBOUNCE_MILLISECONDS};

    /**
     * @brief Default duration after which a press is considered long.
     */
    std::uint32_t longPressMs{eurorack_config::LONG_PRESS_MILLISECONDS};

    /**
     * @brief Maximum interval between presses for a double-click gesture.
     */
    std::uint32_t doubleClickMs{eurorack_config::DOUBLE_CLICK_MILLISECONDS};

    /**
     * @brief Default repeat delay before a held control starts auto-repeat.
     */
    std::uint32_t repeatDelayMs{eurorack_config::REPEAT_DELAY_MILLISECONDS};

    /**
     * @brief Default interval between auto-repeat events after the delay.
     */
    std::uint32_t repeatIntervalMs{eurorack_config::REPEAT_INTERVAL_MILLISECONDS};
};

/**
 * @brief Time-base defaults for firmware loops, rendering, and simulation.
 */
struct RuntimeTiming final {
    /**
     * @brief Recommended control-service frequency in hertz.
     */
    std::uint32_t controlUpdateHz{eurorack_config::CONTROL_UPDATE_HERTZ};

    /**
     * @brief Recommended UI rendering frequency in hertz.
     *
     * This is a presentation default, not a requirement for control processing.
     */
    std::uint32_t uiRefreshHz{eurorack_config::UI_REFRESH_HERTZ};

    /**
     * @brief Default deterministic simulator step in microseconds.
     */
    std::uint32_t simulatorStepUs{eurorack_config::SIMULATOR_STEP_MICROSECONDS};
};

/**
 * @brief Central configuration consumed by firmware, tests, and simulators.
 *
 * The type is intentionally an aggregate. A consuming project can copy
 * eurorackDefaults and override only the fields relevant to its hardware:
 *
 * @code{.cpp}
 * constexpr auto makeProjectConfig() {
 *     auto config = eurorack::core::eurorackDefaults;
 *     config.cv.outputVolts = {-5.0F, 5.0F};
 *     config.runtime.uiRefreshHz = 30U;
 *     return config;
 * }
 *
 * inline constexpr auto projectConfig = makeProjectConfig();
 * static_assert(eurorack::core::isValid(projectConfig));
 * @endcode
 */
struct FrameworkConfig final {
    CvLimits cv{};                   ///< Continuous and pitch CV limits.
    GateLimits gate{};               ///< Gate and trigger electrical limits.
    InteractionTiming interaction{}; ///< Human-interface timing limits.
    RuntimeTiming runtime{};         ///< Firmware and simulator timing defaults.
};

/**
 * @brief Validates a complete framework configuration.
 *
 * @param config Configuration to validate.
 * @return True when all ranges, thresholds, scales, counts, and timing values
 *         are internally consistent and nonzero where required; otherwise false.
 *
 * Validation checks logical consistency only. It cannot prove that a PCB input
 * stage, DAC, op-amp, GPIO, or power rail can safely realize the requested values.
 */
[[nodiscard]] constexpr bool isValid(const FrameworkConfig& config) noexcept {
    return config.cv.audioVolts.isValid() && config.cv.inputVolts.isValid() &&
           config.cv.outputVolts.isValid() && config.cv.pitchInputVolts.isValid() &&
           config.cv.pitchOutputVolts.isValid() && config.cv.voltsPerOctave > 0.0F &&
           config.cv.semitonesPerOctave > 0U &&
           config.gate.inputLowMaximumVolts < config.gate.inputHighMinimumVolts &&
           config.gate.outputLowVolts < config.gate.outputHighVolts &&
           config.gate.triggerPulseMs > 0U &&
           config.interaction.longPressMs >= config.interaction.buttonDebounceMs &&
           config.interaction.doubleClickMs >= config.interaction.buttonDebounceMs &&
           config.interaction.repeatDelayMs >= config.interaction.buttonDebounceMs &&
           config.interaction.repeatIntervalMs > 0U && config.runtime.controlUpdateHz > 0U &&
           config.runtime.uiRefreshHz > 0U && config.runtime.simulatorStepUs > 0U;
}

/**
 * @brief Conservative default configuration for a general Eurorack module.
 *
 * These values are operational software defaults. They do not replace input
 * protection, output-current limiting, rail-aware analog design, calibration,
 * or hardware-specific absolute-maximum ratings.
 */
inline constexpr FrameworkConfig eurorackDefaults{};

static_assert(isValid(eurorackDefaults));

} // namespace eurorack::core

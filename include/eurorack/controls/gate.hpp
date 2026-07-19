/**
 * @file include/eurorack/controls/gate.hpp
 * @brief Declares hysteretic gate input and timed trigger output models.
 *
 *
 * @details
 * Control objects translate raw hardware observations into stable, application-facing state.
 * They do not own referenced hardware interfaces; dependencies must outlive the control object.
 * Unless stated otherwise, calls are synchronous, allocate no memory, and are intended for a
 * cooperative firmware loop. Objects are not internally synchronized and require external
 * protection when shared between interrupt and foreground contexts.
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
#include <eurorack/core/framework_config.hpp>

namespace eurorack::controls {

/**
 * @brief Immutable gate-input state.
 */
struct GateInputSnapshot final {
    bool high{false};    ///< Current hysteretic logical gate state.
    bool rising{false};  ///< True after the latest update accepted a rising edge.
    bool falling{false}; ///< True after the latest update accepted a falling edge.
    float volts{0.0F};   ///< Most recently supplied gate voltage.
};

/**
 * @brief Applies hysteresis and edge detection to a gate voltage.
 */
class GateInput final {
  public:
    /**
     * @brief Constructs a gate-input model.
     *
     * @param lowMaximum Highest voltage guaranteed to be interpreted as low.
     * @param highMinimum Lowest voltage guaranteed to be interpreted as high.
     */
    GateInput(
        float lowMaximum = eurorack::core::eurorackDefaults.gate.inputLowMaximumVolts,
        float highMinimum = eurorack::core::eurorackDefaults.gate.inputHighMinimumVolts) noexcept;

    /**
     * @brief Processes one measured gate voltage.
     *
     * @param volts Measured gate voltage.
     */
    void update(float volts) noexcept;

    /**
     * @brief Returns the current immutable gate-input state.
     *
     * @return Constant reference to gate state and edge flags.
     */
    [[nodiscard]] const GateInputSnapshot& snapshot() const noexcept;

  private:
    float lowMaximum_{1.0F};       ///< Highest definite-low voltage.
    float highMinimum_{2.0F};      ///< Lowest definite-high voltage.
    GateInputSnapshot snapshot_{}; ///< Most recently calculated state.
};

/**
 * @brief Immutable timed trigger-output state.
 */
struct TriggerOutputSnapshot final {
    bool high{false};              ///< Current logical output state.
    bool started{false};           ///< True after trigger() until the next update().
    bool ended{false};             ///< True when the latest update ended the pulse.
    std::uint32_t startedAtMs{0U}; ///< Monotonic pulse start time in milliseconds.
};

/**
 * @brief Generates a deterministic timed trigger pulse.
 */
class TriggerOutput final {
  public:
    /**
     * @brief Constructs a trigger-output model.
     *
     * @param pulseMs Pulse duration in milliseconds.
     */
    explicit TriggerOutput(
        std::uint32_t pulseMs = eurorack::core::eurorackDefaults.gate.triggerPulseMs) noexcept;

    /**
     * @brief Starts or restarts a trigger pulse.
     *
     * @param nowMs Current monotonic time in milliseconds.
     */
    void trigger(std::uint32_t nowMs) noexcept;

    /**
     * @brief Advances trigger timing.
     *
     * @param nowMs Current monotonic time in milliseconds.
     */
    void update(std::uint32_t nowMs) noexcept;

    /**
     * @brief Returns the current immutable trigger-output state.
     *
     * @return Constant reference to trigger timing and edge flags.
     */
    [[nodiscard]] const TriggerOutputSnapshot& snapshot() const noexcept;

  private:
    std::uint32_t pulseMs_{5U};        ///< Configured pulse duration.
    TriggerOutputSnapshot snapshot_{}; ///< Current trigger state.
};

} // namespace eurorack::controls

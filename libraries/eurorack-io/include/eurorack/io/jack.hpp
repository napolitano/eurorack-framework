/**
 * @file include/eurorack/io/jack.hpp
 * @brief Declares generic Eurorack jack models.
 *
 * @details
 * Separates signal semantics from physical ADC, DAC, comparator, and GPIO implementations.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup io
 */

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/core/framework_config.hpp>

namespace eurorack::io {

/**
 * @brief Intended signal family carried by a panel jack.
 *
 * @details
 * The value is descriptive metadata for application logic, simulators, and
 * diagnostics. It does not configure protection, impedance, gain, or voltage
 * range in hardware.
 */
enum class JackSignalType : std::uint8_t { Audio, Cv, PitchCv, Gate, Trigger, Clock };

/**
 * @brief Stable connection state reported for a jack or patch point.
 *
 * @details
 * `Unknown` represents an uninitialized or unavailable detection state rather
 * than a third physical plug position.
 */
enum class ConnectionState : std::uint8_t { Unknown, Disconnected, Connected };

/**
 * @brief Latest raw, normalized, and connection state of an analog jack.
 *
 * @details
 * The snapshot combines the most recent ADC sample with the jack's optional
 * connection detector. Range and I/O status remain visible so application code
 * can distinguish a valid endpoint from a failed acquisition.
 */
struct AnalogJackSnapshot final {
    JackSignalType signalType{JackSignalType::Cv};        ///< Descriptive signal family, fixed at
                                                          ///< construction.
    ConnectionState connection{ConnectionState::Unknown}; ///< Last value passed to
                                                          ///< `setConnection`; independent of
                                                          ///< signal level.
    float rawVolts{0.0F};       ///< Unclamped voltage as supplied to `update`/`setVolts`.
    float effectiveVolts{0.0F}; ///< `rawVolts` clamped to the jack's configured voltage range.
    bool belowRange{false};     ///< True when `rawVolts` was below the configured minimum.
    bool aboveRange{false};     ///< True when `rawVolts` was above the configured maximum.
    bool changed{false};        ///< True when the most recent call changed `effectiveVolts`.
};

/**
 * @brief Clamps and normalizes sampled voltages for an analog input jack.
 *
 * @details
 * The class holds no reference to any ADC or input channel; it is a pure clamping and
 * bookkeeping model. Call `update` with an already sampled voltage and read the clamped result
 * from `snapshot`. Electrical protection and Eurorack voltage scaling belong to the hardware
 * frontend.
 */
class AnalogInputJack final {
  public:
    /**
     * @brief Constructs an analog input jack model.
     *
     * @param signalType Descriptive signal family stored in the snapshot; does not affect
     * clamping or conversion.
     * @param range Inclusive voltage range used to clamp samples and set the range-diagnostic
     * flags.
     */
    AnalogInputJack(JackSignalType signalType = JackSignalType::Cv,
                    eurorack::core::VoltageRange range =
                        eurorack::core::eurorackDefaults.cv.inputVolts) noexcept;

    /**
     * @brief Processes one sampled voltage.
     *
     * @details
     * Stores `volts` as `rawVolts`, sets `belowRange`/`aboveRange` against the configured range,
     * clamps it into `effectiveVolts`, and sets `changed` when `effectiveVolts` differs from its
     * previous value.
     *
     * @param volts Voltage as sampled from the analog frontend.
     */
    void update(float volts) noexcept;

    /**
     * @brief Records an externally observed plug-connection state.
     *
     * @details
     * Stores `connection` directly into the snapshot; this class does not detect connections
     * itself.
     *
     * @param connection Connection-state observation supplied by the application or hardware
     * adapter.
     */
    void setConnection(ConnectionState connection) noexcept;

    /**
     * @brief Returns the current immutable jack snapshot.
     *
     * @return Constant reference to the latest sampled and clamped state.
     */
    [[nodiscard]] const AnalogJackSnapshot& snapshot() const noexcept;

  private:
    JackSignalType signalType_{JackSignalType::Cv}; ///< Descriptive signal family, copied into
                                                    ///< the snapshot at construction.
    eurorack::core::VoltageRange range_{}; ///< Inclusive voltage range used for clamping and
                                           ///< range diagnostics.
    AnalogJackSnapshot snapshot_{};        ///< Most recently calculated state.
};

/**
 * @brief Clamps requested voltages for an analog output jack and tracks the result.
 *
 * @details
 * The class holds no reference to any DAC or output channel; it is a pure voltage-clamping and
 * bookkeeping model. Call `setVolts` with the desired voltage and read the clamped result from
 * `snapshot`; forwarding the clamped voltage to an actual DAC is the caller's responsibility.
 */
class AnalogOutputJack final {
  public:
    /**
     * @brief Constructs an analog output jack model.
     *
     * @param signalType Descriptive signal family stored in the snapshot; does not affect
     * clamping or conversion.
     * @param range Inclusive voltage range used to clamp requested voltages and set the
     * range-diagnostic flags.
     */
    AnalogOutputJack(JackSignalType signalType = JackSignalType::Cv,
                     eurorack::core::VoltageRange range =
                         eurorack::core::eurorackDefaults.cv.outputVolts) noexcept;

    /**
     * @brief Requests one output voltage.
     *
     * @details
     * Stores `volts` as `rawVolts`, sets `belowRange`/`aboveRange` against the configured range,
     * clamps it into `effectiveVolts`, and sets `changed` when `effectiveVolts` differs from its
     * previous value.
     *
     * @param volts Desired output voltage before clamping.
     */
    void setVolts(float volts) noexcept;

    /**
     * @brief Records an externally observed plug-connection state.
     *
     * @details
     * Stores `connection` directly into the snapshot; this class does not detect connections
     * itself.
     *
     * @param connection Connection-state observation supplied by the application or hardware
     * adapter.
     */
    void setConnection(ConnectionState connection) noexcept;

    /**
     * @brief Returns the current immutable jack snapshot.
     *
     * @return Constant reference to the latest requested and clamped state.
     */
    [[nodiscard]] const AnalogJackSnapshot& snapshot() const noexcept;

  private:
    JackSignalType signalType_{JackSignalType::Cv}; ///< Descriptive signal family, copied into
                                                    ///< the snapshot at construction.
    eurorack::core::VoltageRange range_{}; ///< Inclusive voltage range used for clamping and
                                           ///< range diagnostics.
    AnalogJackSnapshot snapshot_{};        ///< Most recently calculated state.
};

/**
 * @brief Stable logic, edge, and connection state of a digital jack.
 *
 * @details
 * Edge flags describe changes accepted by the most recent update. Connection
 * state is tracked independently when a switched-contact detector is available.
 */
struct DigitalJackSnapshot final {
    JackSignalType signalType{JackSignalType::Gate};      ///< Descriptive signal family, fixed at
                                                          ///< construction.
    ConnectionState connection{ConnectionState::Unknown}; ///< Last value passed to
                                                          ///< `setConnection`; independent of
                                                          ///< signal level.
    float volts{0.0F};   ///< For an input jack, the last sampled voltage as passed to `update`.
                         ///< For an output jack, `lowVolts` or `highVolts` depending on the last
                         ///< `setHigh` call.
    bool high{false};    ///< Current logical state.
    bool rising{false};  ///< True only for the call that transitioned `high` from false to true.
    bool falling{false}; ///< True only for the call that transitioned `high` from true to false.
};

/**
 * @brief Converts a sampled voltage into hysteresis-debounced logical gate/trigger state.
 *
 * @details
 * The class holds no reference to any comparator or digital input channel; it is a pure
 * threshold and bookkeeping model. Call `update` with an already sampled voltage. Two
 * independent thresholds provide Schmitt-trigger-style hysteresis: the signal must rise to at
 * least `highMinimum` to be accepted as high, and fall to at most `lowMaximum` to be accepted as
 * low again; voltages between the two thresholds leave the current state unchanged.
 */
class DigitalInputJack final {
  public:
    /**
     * @brief Constructs a digital input jack model.
     *
     * @param signalType Descriptive signal family stored in the snapshot; does not affect
     * threshold detection.
     * @param lowMaximum Highest voltage that is unambiguously interpreted as low; the signal
     * must fall to at or below this level to be accepted as low again from a high state.
     * @param highMinimum Lowest voltage that is unambiguously interpreted as high; the signal
     * must rise to at or above this level to be accepted as high from a low state.
     */
    DigitalInputJack(
        JackSignalType signalType = JackSignalType::Gate,
        float lowMaximum = eurorack::core::eurorackDefaults.gate.inputLowMaximumVolts,
        float highMinimum = eurorack::core::eurorackDefaults.gate.inputHighMinimumVolts) noexcept;

    /**
     * @brief Processes one sampled voltage.
     *
     * @details
     * Stores `volts` and clears `rising`/`falling`, then sets `rising` and `high` if the
     * previous state was low and `volts` has reached `highMinimum`, or sets `falling` and
     * clears `high` if the previous state was high and `volts` has fallen to `lowMaximum` or
     * below. A voltage between the two thresholds leaves `high` unchanged.
     *
     * @param volts Voltage as sampled from the digital frontend.
     */
    void update(float volts) noexcept;

    /**
     * @brief Records an externally observed plug-connection state.
     *
     * @details
     * Stores `connection` directly into the snapshot; this class does not detect connections
     * itself.
     *
     * @param connection Connection-state observation supplied by the application or hardware
     * adapter.
     */
    void setConnection(ConnectionState connection) noexcept;

    /**
     * @brief Returns the current immutable jack snapshot.
     *
     * @return Constant reference to the latest sampled state and edge flags.
     */
    [[nodiscard]] const DigitalJackSnapshot& snapshot() const noexcept;

  private:
    float lowMaximum_{1.0F};         ///< Highest voltage accepted as low.
    float highMinimum_{2.0F};        ///< Lowest voltage accepted as high.
    DigitalJackSnapshot snapshot_{}; ///< Most recently calculated state and edge flags.
};

/**
 * @brief Translates a logical gate/trigger state into low/high output voltages.
 *
 * @details
 * The class holds no reference to any DAC, GPIO, or output channel; it is a pure state and
 * voltage-lookup model. Call `setHigh` to change state and read `lowVolts`/`highVolts` from
 * `snapshot`; forwarding that voltage to actual hardware is the caller's responsibility. The
 * cached snapshot records software intent, not a measured physical voltage.
 */
class DigitalOutputJack final {
  public:
    /**
     * @brief Constructs a digital output jack model.
     *
     * @param signalType Descriptive signal family stored in the snapshot; does not affect the
     * output voltages.
     * @param lowVolts Voltage reported in the snapshot for the logical-low state.
     * @param highVolts Voltage reported in the snapshot for the logical-high state.
     */
    DigitalOutputJack(
        JackSignalType signalType = JackSignalType::Gate,
        float lowVolts = eurorack::core::eurorackDefaults.gate.outputLowVolts,
        float highVolts = eurorack::core::eurorackDefaults.gate.outputHighVolts) noexcept;

    /**
     * @brief Sets the logical output state.
     *
     * @details
     * Sets `rising` when transitioning from low to high, `falling` when transitioning from high
     * to low, and updates `volts` to `highVolts` or `lowVolts` accordingly.
     *
     * @param high Requested logical state.
     */
    void setHigh(bool high) noexcept;

    /**
     * @brief Records an externally observed plug-connection state.
     *
     * @details
     * Stores `connection` directly into the snapshot; this class does not detect connections
     * itself.
     *
     * @param connection Connection-state observation supplied by the application or hardware
     * adapter.
     */
    void setConnection(ConnectionState connection) noexcept;

    /**
     * @brief Returns the current immutable jack snapshot.
     *
     * @return Constant reference to the latest requested state and edge flags.
     */
    [[nodiscard]] const DigitalJackSnapshot& snapshot() const noexcept;

  private:
    float lowVolts_{0.0F};           ///< Voltage reported for the logical-low state.
    float highVolts_{5.0F};          ///< Voltage reported for the logical-high state.
    DigitalJackSnapshot snapshot_{}; ///< Most recently requested state and edge flags.
};

} // namespace eurorack::io

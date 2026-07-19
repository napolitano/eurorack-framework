/**
 * @file include/eurorack/io/jack.hpp
 * @brief Declares generic Eurorack jack models.
 *
 * @details
 * Separates signal semantics from physical ADC, DAC, comparator, and GPIO implementations.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup io
 */

#pragma once

#include <cstdint>
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
    JackSignalType signalType{JackSignalType::Cv};
    ConnectionState connection{ConnectionState::Unknown};
    float rawVolts{0.0F};
    float effectiveVolts{0.0F};
    bool belowRange{false};
    bool aboveRange{false};
    bool changed{false};
};

/**
 * @brief Reads and normalizes an analog input jack through injected interfaces.
 *
 * @details
 * The class retains non-owning references to an analog input channel and,
 * optionally, a digital plug-detection input. update() samples synchronously and
 * refreshes one immutable snapshot. Electrical protection and Eurorack voltage
 * scaling belong to the hardware frontend.
 */
class AnalogInputJack final {
  public:
    /**
     * @brief Provides the analog input jack operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param signalType Semantic signal type represented by the jack model.
     *
     * @param range Inclusive voltage range used for clamping and diagnostics.
     */
    AnalogInputJack(JackSignalType signalType = JackSignalType::Cv,
                    eurorack::core::VoltageRange range =
                        eurorack::core::eurorackDefaults.cv.inputVolts) noexcept;

    /**
     * @brief Provides the update operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param volts Voltage expressed in volts.
     */
    void update(float volts) noexcept;
    /**
     * @brief Sets connection.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param connection Connection-state observation supplied by the application or hardware adapter.
     */
    void setConnection(ConnectionState connection) noexcept;

    /**
     * @brief Returns the current immutable jack snapshot.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return A non-owning reference valid until the owning object is modified or destroyed.
     */
    [[nodiscard]] const AnalogJackSnapshot& snapshot() const noexcept;

  private:
    JackSignalType signalType_{JackSignalType::Cv};
    eurorack::core::VoltageRange range_{};
    AnalogJackSnapshot snapshot_{};
};

/**
 * @brief Writes raw converter codes to an analog output jack.
 *
 * @details
 * The object is a non-owning view over an AnalogOutputChannel. It validates
 * against the backend's maximum code and stores the most recent result for
 * diagnostics. It does not guarantee voltage accuracy or output protection.
 */
class AnalogOutputJack final {
  public:
    /**
     * @brief Provides the analog output jack operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param signalType Semantic signal type represented by the jack model.
     *
     * @param range Inclusive voltage range used for clamping and diagnostics.
     */
    AnalogOutputJack(JackSignalType signalType = JackSignalType::Cv,
                     eurorack::core::VoltageRange range =
                         eurorack::core::eurorackDefaults.cv.outputVolts) noexcept;

    /**
     * @brief Sets volts.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param volts Voltage expressed in volts.
     */
    void setVolts(float volts) noexcept;
    /**
     * @brief Sets connection.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param connection Connection-state observation supplied by the application or hardware adapter.
     */
    void setConnection(ConnectionState connection) noexcept;

    /**
     * @brief Returns the current immutable jack snapshot.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return A non-owning reference valid until the owning object is modified or destroyed.
     */
    [[nodiscard]] const AnalogJackSnapshot& snapshot() const noexcept;

  private:
    JackSignalType signalType_{JackSignalType::Cv};
    eurorack::core::VoltageRange range_{};
    AnalogJackSnapshot snapshot_{};
};

/**
 * @brief Stable logic, edge, and connection state of a digital jack.
 *
 * @details
 * Edge flags describe changes accepted by the most recent update. Connection
 * state is tracked independently when a switched-contact detector is available.
 */
struct DigitalJackSnapshot final {
    JackSignalType signalType{JackSignalType::Gate};
    ConnectionState connection{ConnectionState::Unknown};
    float volts{0.0F};
    bool high{false};
    bool rising{false};
    bool falling{false};
};

/**
 * @brief Samples a gate, trigger, or other digital input jack.
 *
 * @details
 * The class retains non-owning input and optional detection references. It
 * converts raw electrical polarity into logical state, reports edges, and keeps
 * plug detection separate from signal level.
 */
class DigitalInputJack final {
  public:
    /**
     * @brief Provides the digital input jack operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param signalType Semantic signal type represented by the jack model.
     *
     * @param lowMaximum Highest voltage that is unambiguously interpreted as low.
     *
     * @param highMinimum Lowest voltage that is unambiguously interpreted as high.
     */
    DigitalInputJack(
        JackSignalType signalType = JackSignalType::Gate,
        float lowMaximum = eurorack::core::eurorackDefaults.gate.inputLowMaximumVolts,
        float highMinimum = eurorack::core::eurorackDefaults.gate.inputHighMinimumVolts) noexcept;

    /**
     * @brief Provides the update operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param volts Voltage expressed in volts.
     */
    void update(float volts) noexcept;
    /**
     * @brief Sets connection.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param connection Connection-state observation supplied by the application or hardware adapter.
     */
    void setConnection(ConnectionState connection) noexcept;

    /**
     * @brief Returns the current immutable jack snapshot.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return A non-owning reference valid until the owning object is modified or destroyed.
     */
    [[nodiscard]] const DigitalJackSnapshot& snapshot() const noexcept;

  private:
    float lowMaximum_{1.0F};
    float highMinimum_{2.0F};
    DigitalJackSnapshot snapshot_{};
};

/**
 * @brief Drives a logical gate or trigger output through a digital backend.
 *
 * @details
 * Logical state is translated according to configured active polarity before
 * being forwarded to the output interface. The cached snapshot records software
 * intent, not a measured physical voltage.
 */
class DigitalOutputJack final {
  public:
    /**
     * @brief Provides the digital output jack operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param signalType Semantic signal type represented by the jack model.
     *
     * @param lowVolts Voltage represented by the logical-low output state.
     *
     * @param highVolts Voltage represented by the logical-high output state.
     */
    DigitalOutputJack(
        JackSignalType signalType = JackSignalType::Gate,
        float lowVolts = eurorack::core::eurorackDefaults.gate.outputLowVolts,
        float highVolts = eurorack::core::eurorackDefaults.gate.outputHighVolts) noexcept;

    /**
     * @brief Sets the logical state.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param high Logical state to apply.
     */
    void setHigh(bool high) noexcept;
    /**
     * @brief Sets connection.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param connection Connection-state observation supplied by the application or hardware adapter.
     */
    void setConnection(ConnectionState connection) noexcept;

    /**
     * @brief Returns the current immutable jack snapshot.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return A non-owning reference valid until the owning object is modified or destroyed.
     */
    [[nodiscard]] const DigitalJackSnapshot& snapshot() const noexcept;

  private:
    float lowVolts_{0.0F};
    float highVolts_{5.0F};
    DigitalJackSnapshot snapshot_{};
};

} // namespace eurorack::io

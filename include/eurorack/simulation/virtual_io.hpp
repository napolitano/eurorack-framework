/**
 * @file include/eurorack/simulation/virtual_io.hpp
 * @brief Declares virtual digital and analog hardware channels.
 *
 * @details
 * Provides inspectable input and output channels for tests and desktop simulation.
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
 * @ingroup simulation
 */

#if !defined(ARDUINO)

#pragma once

#include <cstdint>
#include <eurorack/io/analog_io.hpp>
#include <eurorack/io/digital_io.hpp>

namespace eurorack::simulation {

/**
 * @brief Virtual digital input controlled by the simulator.
 */
class VirtualDigitalInput final : public eurorack::io::DigitalInput {
  public:
    /**
     * @brief Provides the virtual digital input operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param high Logical state to apply.
     *
     * @return The value described by the operation.
     */
    explicit VirtualDigitalInput(bool high = false) noexcept;

    /**
     * @brief Reads the current logical state.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readHigh() const noexcept override;

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

  private:
    bool high_{false};
};

/**
 * @brief Virtual digital output with inspectable state.
 */
class VirtualDigitalOutput final : public eurorack::io::DigitalOutput {
  public:
    /**
     * @brief Provides the virtual digital output operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param high Logical state to apply.
     *
     * @return The value described by the operation.
     */
    explicit VirtualDigitalOutput(bool high = false) noexcept;

    /**
     * @brief Writes high.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param high Logical state to apply.
     */
    void writeHigh(bool high) noexcept override;

    /**
     * @brief Returns the last logical output state.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool lastWrittenHigh() const noexcept override;

    /**
     * @brief Returns the number of output-state transitions.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested integer value in the units documented by the enclosing API.
     */
    [[nodiscard]] std::uint32_t transitionCount() const noexcept;

  private:
    bool high_{false};
    std::uint32_t transitionCount_{0U};
};

/**
 * @brief Virtual bidirectional digital pin.
 */
class VirtualBidirectionalPin final : public eurorack::io::BidirectionalDigitalPin {
  public:
    /**
     * @brief Provides the virtual bidirectional pin operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     */
    VirtualBidirectionalPin() noexcept = default;

    /**
     * @brief Changes the digital pin direction.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param direction Requested input or output direction.
     */
    void setDirection(eurorack::io::BidirectionalDigitalPin::Direction direction) noexcept override;

    [[nodiscard]] eurorack::io::BidirectionalDigitalPin::Direction
    /**
     * @brief Provides the direction operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     */
    direction() const noexcept override;

    /**
     * @brief Reads the current logical state.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readHigh() const noexcept override;

    /**
     * @brief Writes high.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param high Logical state to apply.
     */
    void writeHigh(bool high) noexcept override;

    /**
     * @brief Returns the last logical output state.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool lastWrittenHigh() const noexcept override;

    /**
     * @brief Sets external high.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param high Logical state to apply.
     */
    void setExternalHigh(bool high) noexcept;

  private:
    eurorack::io::BidirectionalDigitalPin::Direction direction_{
        eurorack::io::BidirectionalDigitalPin::Direction::Input};
    bool outputHigh_{false};
    bool externalHigh_{false};
};

/**
 * @brief Virtual ADC input channel.
 */
class VirtualAnalogInput final : public eurorack::io::AnalogInputChannel {
  public:
    /**
     * @brief Provides the virtual analog input operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param maximumCode Inclusive maximum raw converter code.
     *
     * @return The value described by the operation.
     */
    explicit VirtualAnalogInput(std::uint32_t maximumCode = 4095U) noexcept;

    /**
     * @brief Reads one raw analog sample.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The value described by the operation.
     */
    [[nodiscard]] eurorack::io::AnalogSample readRaw() noexcept override;

    /**
     * @brief Returns the inclusive maximum converter code.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested integer value in the units documented by the enclosing API.
     */
    [[nodiscard]] std::uint32_t maximumCode() const noexcept override;

    /**
     * @brief Sets code.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param code Raw converter code.
     */
    void setCode(std::uint32_t code) noexcept;

    /**
     * @brief Sets result.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param result Result injected into the next simulated operation.
     */
    void setResult(eurorack::io::IoResult result) noexcept;

  private:
    std::uint32_t maximumCode_{4095U};
    std::uint32_t code_{0U};
    eurorack::io::IoResult result_{eurorack::io::IoResult::Success};
};

/**
 * @brief Virtual DAC output channel.
 */
class VirtualAnalogOutput final : public eurorack::io::AnalogOutputChannel {
  public:
    /**
     * @brief Provides the virtual analog output operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param maximumCode Inclusive maximum raw converter code.
     *
     * @return The value described by the operation.
     */
    explicit VirtualAnalogOutput(std::uint32_t maximumCode = 4095U) noexcept;

    /**
     * @brief Writes one raw analog output code.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param code Raw converter code.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    eurorack::io::IoResult writeRaw(std::uint32_t code) noexcept override;

    /**
     * @brief Returns the last accepted raw output code.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested integer value in the units documented by the enclosing API.
     */
    [[nodiscard]] std::uint32_t lastWrittenCode() const noexcept override;

    /**
     * @brief Returns the inclusive maximum converter code.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested integer value in the units documented by the enclosing API.
     */
    [[nodiscard]] std::uint32_t maximumCode() const noexcept override;

    /**
     * @brief Sets result.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param result Result injected into the next simulated operation.
     */
    void setResult(eurorack::io::IoResult result) noexcept;

  private:
    std::uint32_t maximumCode_{4095U};
    std::uint32_t code_{0U};
    eurorack::io::IoResult result_{eurorack::io::IoResult::Success};
};

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)

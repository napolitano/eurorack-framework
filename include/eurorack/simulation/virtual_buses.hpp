/**
 * @file include/eurorack/simulation/virtual_buses.hpp
 * @brief Declares inspectable virtual SPI and I2C buses.
 *
 * @details
 * Records transfers, supports deterministic response queues, and injects bus errors.
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

#include <cstddef>
#include <cstdint>
#include <deque>
#include <eurorack/io/i2c_bus.hpp>
#include <eurorack/io/spi_bus.hpp>
#include <vector>

namespace eurorack::simulation {

/**
 * @brief Immutable trace entry for one virtual SPI transfer.
 *
 * @details
 * The record captures transaction settings and complete transmitted and received
 * byte vectors, allowing exact protocol assertions after driver calls.
 */
struct SpiTransferRecord final {
    eurorack::io::SpiSettings settings{};
    std::vector<std::uint8_t> transmitted{};
    std::vector<std::uint8_t> received{};
};

/**
 * @brief Deterministic SPI test double with response queues and trace capture.
 *
 * @details
 * The virtual bus enforces begin/transfer/end transaction ordering, records every
 * attempted transfer, and supports one-shot result injection. It models API
 * behavior rather than electrical timing.
 */
class VirtualSpiBus final : public eurorack::io::SpiBus {
  public:
    eurorack::io::IoResult
    /**
     * @brief Provides the begin transaction operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param settings SPI transaction settings.
     */
    beginTransaction(const eurorack::io::SpiSettings& settings) noexcept override;

    /**
     * @brief Provides the transfer operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param transmit Optional transmit buffer valid for the duration of the transfer.
     *
     * @param receive Optional receive buffer valid for the duration of the transfer.
     *
     * @param size Number of bytes or elements involved.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    eurorack::io::IoResult transfer(const std::uint8_t* transmit,
                                    std::uint8_t* receive,
                                    std::size_t size) noexcept override;

    /**
     * @brief Provides the end transaction operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     */
    void endTransaction() noexcept override;

    /**
     * @brief Queues bytes for the next simulated read or transfer.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param response Response bytes consumed by the next simulated transfer.
     */
    void queueResponse(std::vector<std::uint8_t> response);

    /**
     * @brief Injects the result returned by the next simulated operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param result Result injected into the next simulated operation.
     */
    void setNextResult(eurorack::io::IoResult result) noexcept;

    /**
     * @brief Returns recorded bus transfers.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return A non-owning reference valid until the owning object is modified or destroyed.
     */
    [[nodiscard]] const std::vector<SpiTransferRecord>& transfers() const noexcept;

    /**
     * @brief Clears buffered or recorded state.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     */
    void clear() noexcept;

  private:
    eurorack::io::SpiSettings settings_{};
    bool transactionActive_{false};
    eurorack::io::IoResult nextResult_{eurorack::io::IoResult::Success};
    std::deque<std::vector<std::uint8_t>> responses_{};
    std::vector<SpiTransferRecord> transfers_{};
};

/**
 * @brief Immutable trace entry for one virtual I2C operation.
 *
 * @details
 * Address, write bytes, read bytes, and stop-condition behavior are preserved so
 * register transactions and repeated starts can be asserted precisely.
 */
struct I2cTransferRecord final {
    eurorack::io::I2cAddress address{0U};
    std::vector<std::uint8_t> written{};
    std::vector<std::uint8_t> read{};
};

/**
 * @brief Deterministic I2C test double with queued reads and trace capture.
 *
 * @details
 * The class records every operation in call order, serves queued device
 * responses, and supports one-shot error injection. It uses dynamic containers
 * and is intended only for native tests and simulators.
 */
class VirtualI2cBus final : public eurorack::io::I2cBus {
  public:
    /**
     * @brief Sets clock.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param frequencyHz Requested bus clock frequency in hertz.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    eurorack::io::IoResult setClock(std::uint32_t frequencyHz) noexcept override;

    /**
     * @brief Writes .
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param address Storage address, register address, or I2C address as defined by the enclosing
     * API.
     *
     * @param data Byte buffer used by the operation.
     *
     * @param size Number of bytes or elements involved.
     *
     * @param sendStop True to terminate the I2C write with a stop condition.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    eurorack::io::IoResult write(eurorack::io::I2cAddress address,
                                 const std::uint8_t* data,
                                 std::size_t size,
                                 bool sendStop = true) noexcept override;

    eurorack::io::IoResult
    /**
     * @brief Reads .
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param address Storage address, register address, or I2C address as defined by the enclosing
     * API.
     *
     * @param data Byte buffer used by the operation.
     *
     * @param size Number of bytes or elements involved.
     */
    read(eurorack::io::I2cAddress address, std::uint8_t* data, std::size_t size) noexcept override;

    /**
     * @brief Writes read.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param address Storage address, register address, or I2C address as defined by the enclosing
     * API.
     *
     * @param writeData Bytes transmitted before a repeated-start read.
     *
     * @param writeSize Number of bytes in the write phase.
     *
     * @param readData Destination for the read phase.
     *
     * @param readSize Number of bytes requested in the read phase.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    eurorack::io::IoResult writeRead(eurorack::io::I2cAddress address,
                                     const std::uint8_t* writeData,
                                     std::size_t writeSize,
                                     std::uint8_t* readData,
                                     std::size_t readSize) noexcept override;

    /**
     * @brief Queues bytes for the next simulated read or transfer.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param response Response bytes consumed by the next simulated transfer.
     */
    void queueResponse(std::vector<std::uint8_t> response);

    /**
     * @brief Injects the result returned by the next simulated operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param result Result injected into the next simulated operation.
     */
    void setNextResult(eurorack::io::IoResult result) noexcept;

    /**
     * @brief Returns the configured virtual I2C clock.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested integer value in the units documented by the enclosing API.
     */
    [[nodiscard]] std::uint32_t clockHz() const noexcept;

    /**
     * @brief Returns recorded bus transfers.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return A non-owning reference valid until the owning object is modified or destroyed.
     */
    [[nodiscard]] const std::vector<I2cTransferRecord>& transfers() const noexcept;

    /**
     * @brief Clears buffered or recorded state.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     */
    void clear() noexcept;

  private:
    /**
     * @brief Returns and clears the pending injected result.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult consumeResult() noexcept;

    std::uint32_t clockHz_{100'000U};
    eurorack::io::IoResult nextResult_{eurorack::io::IoResult::Success};
    std::deque<std::vector<std::uint8_t>> responses_{};
    std::vector<I2cTransferRecord> transfers_{};
};

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)

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
    eurorack::io::SpiSettings settings{}; ///< Settings passed to the `beginTransaction` call
                                            ///< active during this transfer.
    std::vector<std::uint8_t> transmitted{}; ///< Bytes as supplied to `transfer`'s transmit
                                               ///< buffer, or zero-filled if none was supplied.
    std::vector<std::uint8_t> received{}; ///< Bytes delivered to `transfer`'s receive buffer,
                                            ///< taken from the queued response (if any) or
                                            ///< zero-filled otherwise.
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
    /**
     * @brief Begins a transaction, enforcing that no other transaction is already open.
     *
     * @details
     * Stores `settings` for use by the next `transfer` calls and marks a transaction as open.
     *
     * @param settings SPI transaction settings recorded into each subsequent transfer.
     * @return `Busy` if a transaction is already open; otherwise `Success`.
     */
    eurorack::io::IoResult
    beginTransaction(const eurorack::io::SpiSettings& settings) noexcept override;

    /**
     * @brief Records one transfer and returns queued or injected data.
     *
     * @details
     * Returns `Busy` if no transaction is currently open. Otherwise records a `SpiTransferRecord`
     * with the active `settings`, the bytes from `transmit` (or zeros if null), and up to `size`
     * bytes popped from the front of the response queue (or zeros if the queue is empty); the
     * received bytes are also copied into `receive` if non-null. A transfer is recorded even
     * when a one-shot error was injected via `setNextResult`, so a test can assert both the
     * bytes involved and the returned error.
     *
     * @param transmit Optional transmit buffer valid for the duration of the call; null is
     * treated as all zeros.
     * @param receive Optional receive buffer valid for the duration of the call; may be null to
     * discard the response.
     * @param size Number of bytes transferred in both directions.
     * @return `Busy` if no transaction is open; otherwise the result last set via
     * `setNextResult` (defaulting to `Success`), which is then reset to `Success`.
     */
    eurorack::io::IoResult transfer(const std::uint8_t* transmit,
                                    std::uint8_t* receive,
                                    std::size_t size) noexcept override;

    /**
     * @brief Ends the current transaction, allowing `beginTransaction` to succeed again.
     */
    void endTransaction() noexcept override;

    /**
     * @brief Queues bytes to be returned by the next `transfer` call.
     *
     * @details
     * Responses are consumed in FIFO order, one per `transfer` call; a `transfer` call that
     * finds the queue empty returns all zeros instead.
     *
     * @param response Response bytes consumed by the next `transfer` call.
     */
    void queueResponse(std::vector<std::uint8_t> response);

    /**
     * @brief Injects the result returned by the next `transfer` call.
     *
     * @details
     * The injected result is consumed and reset to `Success` by the next `transfer` call,
     * regardless of whether a transaction is open; if no transaction is open, `transfer` returns
     * `Busy` instead without consuming the injected result.
     *
     * @param result Result returned by the next `transfer` call.
     */
    void setNextResult(eurorack::io::IoResult result) noexcept;

    /**
     * @brief Returns every transfer recorded since construction or the last `clear`.
     *
     * @return Constant reference to the recorded transfers, in call order.
     */
    [[nodiscard]] const std::vector<SpiTransferRecord>& transfers() const noexcept;

    /**
     * @brief Resets all recorded and queued state.
     *
     * @details
     * Clears recorded transfers and queued responses, closes any open transaction, and resets
     * the injected result to `Success`.
     */
    void clear() noexcept;

  private:
    eurorack::io::SpiSettings settings_{}; ///< Settings from the most recent `beginTransaction`
                                             ///< call, recorded into subsequent transfers.
    bool transactionActive_{false}; ///< True between a successful `beginTransaction` and the
                                      ///< matching `endTransaction`.
    eurorack::io::IoResult nextResult_{eurorack::io::IoResult::Success}; ///< One-shot result
        ///< returned and reset by the next `transfer` call.
    std::deque<std::vector<std::uint8_t>> responses_{}; ///< FIFO queue of byte sequences to
                                                           ///< return from `transfer`.
    std::vector<SpiTransferRecord> transfers_{}; ///< Every transfer recorded since construction
                                                   ///< or the last `clear`.
};

/**
 * @brief Immutable trace entry for one virtual I2C operation.
 *
 * @details
 * Address, write bytes, read bytes, and stop-condition behavior are preserved so
 * register transactions and repeated starts can be asserted precisely.
 */
struct I2cTransferRecord final {
    eurorack::io::I2cAddress address{0U}; ///< Target address passed to `write`, `read`, or
                                            ///< `writeRead`.
    std::vector<std::uint8_t> written{}; ///< Bytes from the write phase, if any; empty for a
                                           ///< plain `read`.
    std::vector<std::uint8_t> read{}; ///< Bytes delivered for the read phase, taken from the
                                        ///< queued response (if any) or zero-filled otherwise;
                                        ///< empty for a plain `write`.
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
     * @brief Stores the requested clock frequency.
     *
     * @details
     * Purely bookkeeping; no simulated timing depends on the configured frequency.
     *
     * @param frequencyHz Requested bus clock frequency in hertz, later returned by `clockHz`.
     * @return The result last set via `setNextResult` (defaulting to `Success`), which is then
     * reset to `Success`.
     */
    eurorack::io::IoResult setClock(std::uint32_t frequencyHz) noexcept override;

    /**
     * @brief Records a write-only transaction.
     *
     * @details
     * Appends an `I2cTransferRecord` with `address` and a copy of `data` (or an empty write
     * buffer if `data` is null); `sendStop` is accepted for interface compatibility but does not
     * affect the recorded trace.
     *
     * @param address Target I2C address.
     * @param data Bytes to record as written; may be null only if `size` is zero.
     * @param size Number of bytes to write.
     * @param sendStop Accepted for interface compatibility; not reflected in the recorded trace.
     * @return The result last set via `setNextResult` (defaulting to `Success`), which is then
     * reset to `Success`.
     */
    eurorack::io::IoResult write(eurorack::io::I2cAddress address,
                                 const std::uint8_t* data,
                                 std::size_t size,
                                 bool sendStop = true) noexcept override;

    /**
     * @brief Records a read-only transaction and delivers queued response data.
     *
     * @details
     * Appends an `I2cTransferRecord` whose `read` field is filled from the front of the response
     * queue (or zeros if the queue is empty), then copies that data into `data` if non-null.
     *
     * @param address Target I2C address.
     * @param data Destination for the read bytes; may be null to discard the response.
     * @param size Number of bytes to read.
     * @return The result last set via `setNextResult` (defaulting to `Success`), which is then
     * reset to `Success`.
     */
    eurorack::io::IoResult
    read(eurorack::io::I2cAddress address, std::uint8_t* data, std::size_t size) noexcept override;

    /**
     * @brief Records a combined write-then-repeated-start-read transaction.
     *
     * @details
     * Appends a single `I2cTransferRecord` carrying both the write bytes and the read bytes
     * delivered from the front of the response queue (or zeros if the queue is empty); the read
     * bytes are also copied into `readData` if non-null.
     *
     * @param address Target I2C address.
     * @param writeData Bytes transmitted before the repeated-start read; may be null only if
     * `writeSize` is zero.
     * @param writeSize Number of bytes in the write phase.
     * @param readData Destination for the read phase; may be null to discard the response.
     * @param readSize Number of bytes requested in the read phase.
     * @return The result last set via `setNextResult` (defaulting to `Success`), which is then
     * reset to `Success`.
     */
    eurorack::io::IoResult writeRead(eurorack::io::I2cAddress address,
                                     const std::uint8_t* writeData,
                                     std::size_t writeSize,
                                     std::uint8_t* readData,
                                     std::size_t readSize) noexcept override;

    /**
     * @brief Queues bytes to be returned by the next `read` or `writeRead` call.
     *
     * @details
     * Responses are consumed in FIFO order, one per read; a read that finds the queue empty
     * returns all zeros instead.
     *
     * @param response Response bytes consumed by the next `read` or `writeRead` call.
     */
    void queueResponse(std::vector<std::uint8_t> response);

    /**
     * @brief Injects the result returned by the next bus operation.
     *
     * @details
     * Consumed and reset to `Success` by the next call to `setClock`, `write`, `read`, or
     * `writeRead`.
     *
     * @param result Result returned by the next bus operation.
     */
    void setNextResult(eurorack::io::IoResult result) noexcept;

    /**
     * @brief Returns the most recently requested clock frequency.
     *
     * @return Clock frequency in hertz, as last passed to `setClock`.
     */
    [[nodiscard]] std::uint32_t clockHz() const noexcept;

    /**
     * @brief Returns every write, read, and write-read transaction recorded since construction
     * or the last `clear`.
     *
     * @return Constant reference to the recorded transfers, in call order.
     */
    [[nodiscard]] const std::vector<I2cTransferRecord>& transfers() const noexcept;

    /**
     * @brief Resets recorded and queued state.
     *
     * @details
     * Clears recorded transfers and queued responses and resets the injected result to
     * `Success`. Does not reset the configured clock frequency.
     */
    void clear() noexcept;

  private:
    /**
     * @brief Returns the pending injected result and resets it to `Success`.
     *
     * @return The result previously set via `setNextResult`, or `Success` if none was pending.
     */
    [[nodiscard]] eurorack::io::IoResult consumeResult() noexcept;

    std::uint32_t clockHz_{100'000U}; ///< Clock frequency last requested via `setClock`.
    eurorack::io::IoResult nextResult_{eurorack::io::IoResult::Success}; ///< One-shot result
        ///< returned and reset by the next bus operation.
    std::deque<std::vector<std::uint8_t>> responses_{}; ///< FIFO queue of byte sequences to
                                                           ///< return from `read`/`writeRead`.
    std::vector<I2cTransferRecord> transfers_{}; ///< Every transaction recorded since
                                                   ///< construction or the last `clear`.
};

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)

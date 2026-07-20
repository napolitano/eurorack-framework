/**
 * @file src/simulation/virtual_buses.cpp
 * @brief Implements inspectable virtual SPI and I2C buses.
 *
 * @details
 * Records all transfers and serves queued responses without real hardware.
 *
 * Every call is copied into an ordered trace so tests can inspect settings,
 * addresses, transmitted bytes, received bytes, and repeated-start behavior.
 * Queued responses emulate device data. setNextResult() injects a one-shot
 * failure and resets automatically after the next matching operation.
 *
 * These native-only classes use dynamic containers and are deterministic but
 * not thread-safe.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#if !defined(ARDUINO)

#if defined(__AVR__)
#include <eurorack/compat/avr/algorithm.hpp>
#else
#include <algorithm>
#endif
#include <eurorack/simulation/virtual_buses.hpp>

namespace eurorack::simulation {

eurorack::io::IoResult
VirtualSpiBus::beginTransaction(const eurorack::io::SpiSettings& settings) noexcept {
    if (transactionActive_) {
        return eurorack::io::IoResult::Busy;
    }

    settings_ = settings;
    transactionActive_ = true;
    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult VirtualSpiBus::transfer(const std::uint8_t* const transmit,
                                               std::uint8_t* const receive,
                                               const std::size_t size) noexcept {
    if (!transactionActive_) {
        return eurorack::io::IoResult::Busy;
    }

    const auto result = nextResult_;
    nextResult_ = eurorack::io::IoResult::Success; // Record the attempted transaction even when the
                                                   // injected result is an
    // error. Tests can then verify both the requested bytes and error handling.

    SpiTransferRecord record{};
    record.settings = settings_;
    record.transmitted.assign(size, 0U);
    record.received.assign(size, 0U);

    if (transmit != nullptr) {
        std::copy(transmit, transmit + size, record.transmitted.begin());
    }

    if (!responses_.empty()) {
        const auto response = responses_.front();
        responses_.pop_front();

        const std::size_t copySize = std::min(size, response.size());
        std::copy(response.begin(),
                  response.begin() + static_cast<std::ptrdiff_t>(copySize),
                  record.received.begin());
    }

    if (receive != nullptr) {
        std::copy(record.received.begin(), record.received.end(), receive);
    }

    transfers_.push_back(record);
    return result;
}

void VirtualSpiBus::endTransaction() noexcept {
    transactionActive_ = false;
}

void VirtualSpiBus::queueResponse(std::vector<std::uint8_t> response) {
    responses_.push_back(std::move(response));
}

void VirtualSpiBus::setNextResult(const eurorack::io::IoResult result) noexcept {
    nextResult_ = result;
}

const std::vector<SpiTransferRecord>& VirtualSpiBus::transfers() const noexcept {
    return transfers_;
}

void VirtualSpiBus::clear() noexcept {
    transfers_.clear();
    responses_.clear();
    transactionActive_ = false;
    nextResult_ = eurorack::io::IoResult::Success;
}

eurorack::io::IoResult VirtualI2cBus::setClock(const std::uint32_t frequencyHz) noexcept {
    clockHz_ = frequencyHz;
    return consumeResult();
}

eurorack::io::IoResult VirtualI2cBus::write(const eurorack::io::I2cAddress address,
                                            const std::uint8_t* const data,
                                            const std::size_t size,
                                            const bool sendStop) noexcept {
    I2cTransferRecord record{};
    record.address = address;

    if (data != nullptr) {
        record.written.assign(data, data + size);
    }

    static_cast<void>(sendStop);
    transfers_.push_back(record);
    return consumeResult();
}

eurorack::io::IoResult VirtualI2cBus::read(const eurorack::io::I2cAddress address,
                                           std::uint8_t* const data,
                                           const std::size_t size) noexcept {
    I2cTransferRecord record{};
    record.address = address;
    record.read.assign(size, 0U);

    if (!responses_.empty()) {
        const auto response = responses_.front();
        responses_.pop_front();

        const std::size_t copySize = std::min(size, response.size());
        std::copy(response.begin(),
                  response.begin() + static_cast<std::ptrdiff_t>(copySize),
                  record.read.begin());
    }

    if (data != nullptr) {
        std::copy(record.read.begin(), record.read.end(), data);
    }

    transfers_.push_back(record);
    return consumeResult();
}

eurorack::io::IoResult VirtualI2cBus::writeRead(const eurorack::io::I2cAddress address,
                                                const std::uint8_t* const writeData,
                                                const std::size_t writeSize,
                                                std::uint8_t* const readData,
                                                const std::size_t readSize) noexcept {
    I2cTransferRecord record{};
    record.address = address;

    if (writeData != nullptr) {
        record.written.assign(writeData, writeData + writeSize);
    }

    record.read.assign(readSize, 0U);

    if (!responses_.empty()) {
        const auto response = responses_.front();
        responses_.pop_front();

        const std::size_t copySize = std::min(readSize, response.size());
        std::copy(response.begin(),
                  response.begin() + static_cast<std::ptrdiff_t>(copySize),
                  record.read.begin());
    }

    if (readData != nullptr) {
        std::copy(record.read.begin(), record.read.end(), readData);
    }

    transfers_.push_back(record);
    return consumeResult();
}

void VirtualI2cBus::queueResponse(std::vector<std::uint8_t> response) {
    responses_.push_back(std::move(response));
}

void VirtualI2cBus::setNextResult(const eurorack::io::IoResult result) noexcept {
    nextResult_ = result;
}

std::uint32_t VirtualI2cBus::clockHz() const noexcept {
    return clockHz_;
}

const std::vector<I2cTransferRecord>& VirtualI2cBus::transfers() const noexcept {
    return transfers_;
}

void VirtualI2cBus::clear() noexcept {
    transfers_.clear();
    responses_.clear();
    nextResult_ = eurorack::io::IoResult::Success;
}

eurorack::io::IoResult VirtualI2cBus::consumeResult() noexcept {
    const auto result = nextResult_;
    nextResult_ = eurorack::io::IoResult::Success;
    return result;
}

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)

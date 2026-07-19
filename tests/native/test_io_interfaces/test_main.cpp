/**
 * @file tests/native/test_io_interfaces/test_main.cpp
 * @brief Tests contracts and usability of the generic hardware interfaces.
 *
 * @details
 * Uses in-memory fake backends to validate digital, analog, time, SPI, and I2C interface behavior.
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
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <eurorack/io/analog_io.hpp>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/i2c_bus.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>
#include <eurorack/io/time_source.hpp>
#include <unity.h>

namespace {

class FakeDigitalPin final : public eurorack::io::BidirectionalDigitalPin {
  public:
    [[nodiscard]] bool readHigh() const noexcept override {
        return level_;
    }
    void writeHigh(const bool high) noexcept override {
        level_ = high;
    }
    [[nodiscard]] bool lastWrittenHigh() const noexcept override {
        return level_;
    }
    void setDirection(const Direction direction) noexcept override {
        direction_ = direction;
    }
    [[nodiscard]] Direction direction() const noexcept override {
        return direction_;
    }

  private:
    bool level_{false};
    Direction direction_{Direction::Input};
};

class FakeAnalogChannel final : public eurorack::io::AnalogInputChannel,
                                public eurorack::io::AnalogOutputChannel {
  public:
    [[nodiscard]] eurorack::io::AnalogSample readRaw() noexcept override {
        return {code_, eurorack::io::IoResult::Success};
    }

    [[nodiscard]] std::uint32_t maximumCode() const noexcept override {
        return 4095U;
    }

    eurorack::io::IoResult writeRaw(const std::uint32_t code) noexcept override {
        if (code > maximumCode()) {
            return eurorack::io::IoResult::InvalidArgument;
        }
        code_ = code;
        return eurorack::io::IoResult::Success;
    }

    [[nodiscard]] std::uint32_t lastWrittenCode() const noexcept override {
        return code_;
    }

  private:
    std::uint32_t code_{0U};
};

class FakeTime final : public eurorack::io::TimeSource, public eurorack::io::DelayProvider {
  public:
    [[nodiscard]] std::uint32_t milliseconds() const noexcept override {
        return microseconds_ / 1000U;
    }

    [[nodiscard]] std::uint32_t microseconds() const noexcept override {
        return microseconds_;
    }

    void delayMicroseconds(const std::uint32_t microseconds) noexcept override {
        microseconds_ += microseconds;
    }

  private:
    std::uint32_t microseconds_{0U};
};

class FakeSpi final : public eurorack::io::SpiBus {
  public:
    eurorack::io::IoResult
    beginTransaction(const eurorack::io::SpiSettings& settings) noexcept override {
        settings_ = settings;
        active_ = true;
        return eurorack::io::IoResult::Success;
    }

    eurorack::io::IoResult transfer(const std::uint8_t* transmit,
                                    std::uint8_t* receive,
                                    const std::size_t size) noexcept override {
        if (!active_) {
            return eurorack::io::IoResult::Busy;
        }
        if (size > 0U && transmit == nullptr && receive == nullptr) {
            return eurorack::io::IoResult::InvalidArgument;
        }
        for (std::size_t index = 0U; index < size; ++index) {
            if (receive != nullptr) {
                receive[index] = transmit != nullptr ? transmit[index] : 0xFFU;
            }
        }
        return eurorack::io::IoResult::Success;
    }

    void endTransaction() noexcept override {
        active_ = false;
    }

    [[nodiscard]] bool active() const noexcept {
        return active_;
    }
    [[nodiscard]] const eurorack::io::SpiSettings& settings() const noexcept {
        return settings_;
    }

  private:
    bool active_{false};
    eurorack::io::SpiSettings settings_{};
};

class FakeI2c final : public eurorack::io::I2cBus {
  public:
    eurorack::io::IoResult setClock(const std::uint32_t clockHertz) noexcept override {
        clockHertz_ = clockHertz;
        return clockHertz > 0U ? eurorack::io::IoResult::Success
                               : eurorack::io::IoResult::InvalidArgument;
    }

    eurorack::io::IoResult write(const eurorack::io::I2cAddress address,
                                 const std::uint8_t* data,
                                 const std::size_t size,
                                 const bool sendStop) noexcept override {
        if (!address.isValid() || (size > 0U && data == nullptr)) {
            return eurorack::io::IoResult::InvalidArgument;
        }
        address_ = address.value();
        sendStop_ = sendStop;
        size_ = size;
        return eurorack::io::IoResult::Success;
    }

    eurorack::io::IoResult read(const eurorack::io::I2cAddress address,
                                std::uint8_t* data,
                                const std::size_t size) noexcept override {
        if (!address.isValid() || (size > 0U && data == nullptr)) {
            return eurorack::io::IoResult::InvalidArgument;
        }
        for (std::size_t index = 0U; index < size; ++index) {
            data[index] = static_cast<std::uint8_t>(index);
        }
        return eurorack::io::IoResult::Success;
    }

    eurorack::io::IoResult writeRead(const eurorack::io::I2cAddress address,
                                     const std::uint8_t* writeData,
                                     const std::size_t writeSize,
                                     std::uint8_t* readData,
                                     const std::size_t readSize) noexcept override {
        const auto result = write(address, writeData, writeSize, false);
        if (!eurorack::io::succeeded(result)) {
            return result;
        }
        return read(address, readData, readSize);
    }

    [[nodiscard]] std::uint32_t clockHertz() const noexcept {
        return clockHertz_;
    }
    [[nodiscard]] std::uint8_t address() const noexcept {
        return address_;
    }
    [[nodiscard]] bool sendStop() const noexcept {
        return sendStop_;
    }
    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

  private:
    std::uint32_t clockHertz_{0U};
    std::uint8_t address_{0U};
    bool sendStop_{true};
    std::size_t size_{0U};
};

void test_digital_pin_supports_direction_read_and_write() {
    FakeDigitalPin pin;
    pin.setDirection(eurorack::io::BidirectionalDigitalPin::Direction::Output);
    pin.writeHigh(true);
    TEST_ASSERT_TRUE(pin.readHigh());
    TEST_ASSERT_TRUE(pin.lastWrittenHigh());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(eurorack::io::BidirectionalDigitalPin::Direction::Output),
        static_cast<int>(pin.direction()));
}

void test_analog_channel_rejects_out_of_range_code() {
    FakeAnalogChannel channel;
    TEST_ASSERT_TRUE(eurorack::io::succeeded(channel.writeRaw(2048U)));
    TEST_ASSERT_EQUAL_UINT32(2048U, channel.readRaw().code);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::InvalidArgument),
                          static_cast<int>(channel.writeRaw(5000U)));
}

void test_time_source_and_delay_share_monotonic_time() {
    FakeTime time;
    time.delayMicroseconds(2500U);
    TEST_ASSERT_EQUAL_UINT32(2500U, time.microseconds());
    TEST_ASSERT_EQUAL_UINT32(2U, time.milliseconds());
}

void test_spi_transfer_uses_explicit_transaction() {
    FakeSpi spi;
    const eurorack::io::SpiSettings settings{8'000'000U,
                                             eurorack::io::SpiMode::Mode1,
                                             eurorack::io::SpiBitOrder::MostSignificantBitFirst};
    TEST_ASSERT_TRUE(eurorack::io::succeeded(spi.beginTransaction(settings)));

    const std::array<std::uint8_t, 3U> transmit{1U, 2U, 3U};
    std::array<std::uint8_t, 3U> receive{};
    TEST_ASSERT_TRUE(
        eurorack::io::succeeded(spi.transfer(transmit.data(), receive.data(), receive.size())));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(transmit.data(), receive.data(), transmit.size());
    TEST_ASSERT_EQUAL_UINT32(8'000'000U, spi.settings().clockHertz);

    spi.endTransaction();
    TEST_ASSERT_FALSE(spi.active());
}

void test_i2c_write_read_uses_seven_bit_address() {
    FakeI2c bus;
    TEST_ASSERT_TRUE(eurorack::io::succeeded(bus.setClock(400'000U)));

    const std::array<std::uint8_t, 1U> command{0x10U};
    std::array<std::uint8_t, 2U> response{};
    TEST_ASSERT_TRUE(eurorack::io::succeeded(bus.writeRead(eurorack::io::I2cAddress(0x3CU),
                                                           command.data(),
                                                           command.size(),
                                                           response.data(),
                                                           response.size())));
    TEST_ASSERT_EQUAL_UINT8(0x3CU, bus.address());
    TEST_ASSERT_FALSE(bus.sendStop());
    TEST_ASSERT_EQUAL_UINT8(0U, response[0]);
    TEST_ASSERT_EQUAL_UINT8(1U, response[1]);
}

void test_invalid_i2c_address_is_preserved_as_invalid() {
    const eurorack::io::I2cAddress address(0x80U);
    TEST_ASSERT_FALSE(address.isValid());
    TEST_ASSERT_EQUAL_UINT8(0U, address.value());
}

} // namespace

extern "C" {

/** @brief Unity setup hook executed before every test. */
void setUp() {}

/** @brief Unity teardown hook executed after every test. */
void tearDown() {}

} // extern "C"

/**
 * @brief Executes the generic hardware-interface test suite.
 *
 * @return Unity process exit code. Zero indicates success.
 */
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_digital_pin_supports_direction_read_and_write);
    RUN_TEST(test_analog_channel_rejects_out_of_range_code);
    RUN_TEST(test_time_source_and_delay_share_monotonic_time);
    RUN_TEST(test_spi_transfer_uses_explicit_transaction);
    RUN_TEST(test_i2c_write_read_uses_seven_bit_address);
    RUN_TEST(test_invalid_i2c_address_is_preserved_as_invalid);
    return UNITY_END();
}

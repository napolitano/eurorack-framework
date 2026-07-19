/**
 * @file include/eurorack/platform/arduino/arduino_spi.hpp
 * @brief Declares an Arduino SPI adapter.
 *
 * @details
 * Wraps SPIClass and SPISettings behind the framework SPI bus interface.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup platform_arduino
 */

#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#include <SPI.h>
#include <cstddef>
#include <cstdint>
#include <eurorack/io/spi_bus.hpp>

namespace eurorack::platform::arduino {

/**
 * @brief Arduino SPIClass adapter.
 */
class ArduinoSpiBus final : public eurorack::io::SpiBus {
  public:
    /**
     * @brief Constructs an SPI adapter.
     *
     * @param spi Arduino SPIClass instance.
     * @param fillerByte Byte transmitted when transmit is null.
     */
    explicit ArduinoSpiBus(SPIClass& spi = SPI, std::uint8_t fillerByte = 0x00U) noexcept
        : spi_(spi), fillerByte_(fillerByte) {}

    /**
     * @brief Begins the underlying SPI peripheral.
     */
    void begin() noexcept {
        spi_.begin();
    }

    /**
     * @brief Starts an SPI transaction.
     *
     * @param settings Framework SPI settings.
     * @return Success.
     */
    eurorack::io::IoResult
    beginTransaction(const eurorack::io::SpiSettings& settings) noexcept override {
        if (active_) {
            return eurorack::io::IoResult::Busy;
        }

        spi_.beginTransaction(SPISettings(
            settings.clockHertz,
            settings.bitOrder == eurorack::io::SpiBitOrder::MostSignificantBitFirst ? MSBFIRST
                                                                                    : LSBFIRST,
            modeValue(settings.mode)));
        active_ = true;
        return eurorack::io::IoResult::Success;
    }

    /**
     * @brief Transfers bytes in full duplex.
     *
     * @param transmit Source bytes or null.
     * @param receive Destination bytes or null.
     * @param size Number of bytes.
     * @return Success, Busy, or InvalidArgument.
     */
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
            const std::uint8_t output = transmit != nullptr ? transmit[index] : fillerByte_;
            const std::uint8_t input = spi_.transfer(output);

            if (receive != nullptr) {
                receive[index] = input;
            }
        }

        return eurorack::io::IoResult::Success;
    }

    /**
     * @brief Ends the active SPI transaction.
     */
    void endTransaction() noexcept override {
        if (active_) {
            spi_.endTransaction();
            active_ = false;
        }
    }

  private:
    /**
     * @brief Converts framework SPI mode to Arduino constant.
     *
     * @param mode Framework SPI mode.
     * @return Arduino SPI mode constant.
     */
    [[nodiscard]] static std::uint8_t modeValue(const eurorack::io::SpiMode mode) noexcept {
        switch (mode) {
        case eurorack::io::SpiMode::Mode1:
            return SPI_MODE1;
        case eurorack::io::SpiMode::Mode2:
            return SPI_MODE2;
        case eurorack::io::SpiMode::Mode3:
            return SPI_MODE3;
        case eurorack::io::SpiMode::Mode0:
        default:
            return SPI_MODE0;
        }
    }

    SPIClass& spi_;
    std::uint8_t fillerByte_{0x00U};
    bool active_{false};
};

} // namespace eurorack::platform::arduino

#endif // ARDUINO

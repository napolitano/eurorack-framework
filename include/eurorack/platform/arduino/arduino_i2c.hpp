/**
 * @file include/eurorack/platform/arduino/arduino_i2c.hpp
 * @brief Declares an Arduino Wire I2C adapter.
 *
 * @details
 * Wraps TwoWire writes, reads, and repeated-start transactions behind the framework I2C interface.
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
#include <Wire.h>
#include <cstddef>
#include <cstdint>
#include <eurorack/io/i2c_bus.hpp>

namespace eurorack::platform::arduino {

/**
 * @brief Arduino TwoWire adapter.
 */
class ArduinoI2cBus final : public eurorack::io::I2cBus {
  public:
    /**
     * @brief Constructs an I2C adapter.
     *
     * @param wire Arduino TwoWire instance.
     */
    explicit ArduinoI2cBus(TwoWire& wire = Wire) noexcept : wire_(wire) {}

    /**
     * @brief Begins the underlying I2C controller.
     */
    void begin() noexcept {
        wire_.begin();
    }

    /**
     * @brief Sets I2C clock frequency.
     *
     * @param clockHertz Requested clock.
     * @return Success.
     */
    eurorack::io::IoResult setClock(const std::uint32_t clockHertz) noexcept override {
        wire_.setClock(clockHertz);
        return eurorack::io::IoResult::Success;
    }

    /**
     * @brief Writes bytes to a device.
     *
     * @param address Seven-bit address.
     * @param data Source bytes.
     * @param size Byte count.
     * @param sendStop True to send a stop condition.
     * @return Mapped operation result.
     */
    eurorack::io::IoResult write(const eurorack::io::I2cAddress address,
                                 const std::uint8_t* data,
                                 const std::size_t size,
                                 const bool sendStop = true) noexcept override {
        if (!address.isValid() || (data == nullptr && size > 0U)) {
            return eurorack::io::IoResult::InvalidArgument;
        }

        wire_.beginTransmission(address.value());

        for (std::size_t index = 0U; index < size; ++index) {
            if (wire_.write(data[index]) != 1U) {
                return eurorack::io::IoResult::DataError;
            }
        }

        return mapEndTransmission(wire_.endTransmission(sendStop));
    }

    /**
     * @brief Reads bytes from a device.
     *
     * @param address Seven-bit address.
     * @param data Destination bytes.
     * @param size Byte count.
     * @return Success when the requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    eurorack::io::IoResult read(const eurorack::io::I2cAddress address,
                                std::uint8_t* data,
                                const std::size_t size) noexcept override {
        if (!address.isValid() || (data == nullptr && size > 0U)) {
            return eurorack::io::IoResult::InvalidArgument;
        }

        const std::size_t requested = static_cast<std::size_t>(wire_.requestFrom(
            static_cast<int>(address.value()), static_cast<int>(size), static_cast<int>(true)));

        if (requested != size) {
            return eurorack::io::IoResult::DataError;
        }

        for (std::size_t index = 0U; index < size; ++index) {
            if (wire_.available() <= 0) {
                return eurorack::io::IoResult::DataError;
            }

            data[index] = static_cast<std::uint8_t>(wire_.read());
        }

        return eurorack::io::IoResult::Success;
    }

    /**
     * @brief Performs write then repeated-start read.
     *
     * @param address Seven-bit address.
     * @param writeData Write bytes.
     * @param writeSize Write byte count.
     * @param readData Read destination.
     * @param readSize Read byte count.
     * @return Success when the requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    eurorack::io::IoResult writeRead(const eurorack::io::I2cAddress address,
                                     const std::uint8_t* writeData,
                                     const std::size_t writeSize,
                                     std::uint8_t* readData,
                                     const std::size_t readSize) noexcept override {
        const auto writeResult = write(address, writeData, writeSize, false);

        if (writeResult != eurorack::io::IoResult::Success) {
            return writeResult;
        }

        return read(address, readData, readSize);
    }

  private:
    /**
     * @brief Maps Arduino Wire status codes.
     *
     * @param status Wire endTransmission status.
     * @return Framework operation result.
     */
    [[nodiscard]] static eurorack::io::IoResult
    mapEndTransmission(const std::uint8_t status) noexcept {
        switch (status) {
        case 0U:
            return eurorack::io::IoResult::Success;
        case 1U:
            return eurorack::io::IoResult::DataError;
        case 2U:
        case 3U:
            return eurorack::io::IoResult::NoDevice;
        case 4U:
            return eurorack::io::IoResult::BusError;
        case 5U:
            return eurorack::io::IoResult::Timeout;
        default:
            return eurorack::io::IoResult::UnknownError;
        }
    }

    TwoWire& wire_;
};

} // namespace eurorack::platform::arduino

#endif // ARDUINO

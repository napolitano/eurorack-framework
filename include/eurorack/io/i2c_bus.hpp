/**
 * @file include/eurorack/io/i2c_bus.hpp
 * @brief Declares a platform-independent I2C bus interface.
 *
 * @details
 * The interface supports writes, reads, and repeated-start write/read transactions using seven-bit
 * addresses.
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
 *
 * @ingroup io
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <eurorack/io/io_result.hpp>

namespace eurorack::io {

/**
 * @brief Seven-bit I2C device address.
 */
class I2cAddress final {
  public:
    /**
     * @brief Constructs a seven-bit I2C address.
     *
     * @param value Address value from 0x00 through 0x7F.
     */
    explicit constexpr I2cAddress(const std::uint8_t value) noexcept
        : value_(static_cast<std::uint8_t>(value & 0x7FU)), valid_(value <= 0x7FU) {}

    /**
     * @brief Returns the stored seven-bit address.
     *
     * @return Address value with the read/write bit excluded.
     */
    [[nodiscard]] constexpr std::uint8_t value() const noexcept {
        return value_;
    }

    /**
     * @brief Reports whether the supplied constructor value fit in seven bits.
     *
     * @return True for values from 0x00 through 0x7F; otherwise false.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return valid_;
    }

  private:
    std::uint8_t value_{0U}; ///< Stored seven-bit address.
    bool valid_{true};       ///< True when the original value was valid.
};

/**
 * @brief I2C controller bus interface.
 */
class I2cBus {
  public:
    /**
     * @brief Destroys an I2C backend through its interface.
     */
    virtual ~I2cBus() = default;

    /**
     * @brief Configures the requested I2C clock frequency.
     *
     * @param clockHertz Requested bus clock in hertz.
     * @return Operation status.
     */
    virtual IoResult setClock(std::uint32_t clockHertz) noexcept = 0;

    /**
     * @brief Writes bytes to an addressed device.
     *
     * @param address Seven-bit device address.
     * @param data Source buffer.
     * @param size Number of bytes to write.
     * @param sendStop True to finish with a stop condition; false to retain the bus.
     * @return Operation status.
     */
    virtual IoResult write(I2cAddress address,
                           const std::uint8_t* data,
                           std::size_t size,
                           bool sendStop = true) noexcept = 0;

    /**
     * @brief Reads bytes from an addressed device.
     *
     * @param address Seven-bit device address.
     * @param data Destination buffer.
     * @param size Number of bytes to read.
     * @return Operation status.
     */
    virtual IoResult read(I2cAddress address, std::uint8_t* data, std::size_t size) noexcept = 0;

    /**
     * @brief Performs a write followed by a repeated-start read.
     *
     * @param address Seven-bit device address used for both phases.
     * @param writeData Bytes sent before the repeated start.
     * @param writeSize Number of bytes to write.
     * @param readData Destination for bytes read after the repeated start.
     * @param readSize Number of bytes to read.
     * @return Operation status.
     */
    virtual IoResult writeRead(I2cAddress address,
                               const std::uint8_t* writeData,
                               std::size_t writeSize,
                               std::uint8_t* readData,
                               std::size_t readSize) noexcept = 0;
};

} // namespace eurorack::io

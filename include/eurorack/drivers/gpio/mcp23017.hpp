/**
 * @file include/eurorack/drivers/gpio/mcp23017.hpp
 * @brief Declares a cached driver for the MCP23017 16-bit I2C GPIO expander.
 *
 * @details
 * The driver treats GPIOA as bits 0 through 7 and GPIOB as bits 8 through 15.
 * Direction, pull-up, polarity, and output values are cached locally. Input
 * accessors return the most recently sampled GPIO register state and do not
 * initiate I2C traffic.
 *
 * The I2C bus is a non-owning dependency. Calls are synchronous and not
 * internally synchronized. Electrical pull-ups, interrupt wiring, input
 * protection, and address-pin configuration remain outside this class.
 *
 * @ingroup drivers
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include <eurorack/io/i2c_bus.hpp>
#include <eurorack/io/io_result.hpp>

namespace eurorack::drivers::gpio {

/**
 * @brief MCP23017 GPIO-expander driver with 16-bit cached state.
 */
class Mcp23017 final {
  public:
    /**
     * @brief Constructs a driver for one I2C address.
     * @param bus I2C bus that must outlive the driver.
     * @param address Valid seven-bit MCP23017 address.
     */
    Mcp23017(eurorack::io::I2cBus& bus,
             eurorack::io::I2cAddress address) noexcept;

    /**
     * @brief Writes all cached configuration and output state to the device.
     * @return Success when every register write succeeds; otherwise the first
     * I2C error encountered.
     */
    eurorack::io::IoResult initialize() noexcept;

    /**
     * @brief Configures input and output directions for all pins.
     * @param directionMask One bit per pin. A set bit selects input mode and a
     * cleared bit selects output mode.
     * @return Result of writing the IODIRA and IODIRB register pair.
     */
    eurorack::io::IoResult setDirection(std::uint16_t directionMask) noexcept;

    /**
     * @brief Enables or disables weak internal pull-ups.
     * @param pullUpMask One bit per pin. A set bit enables the pull-up for an
     * input pin.
     * @return Result of writing the GPPUA and GPPUB register pair.
     */
    eurorack::io::IoResult setPullUps(std::uint16_t pullUpMask) noexcept;

    /**
     * @brief Configures input-polarity inversion.
     * @param polarityMask One bit per pin. A set bit inverts the corresponding
     * sampled input.
     * @return Result of writing the IPOLA and IPOLB register pair.
     */
    eurorack::io::IoResult setInputPolarity(std::uint16_t polarityMask) noexcept;

    /**
     * @brief Replaces and writes the complete output latch mask.
     * @param outputMask One bit per output pin.
     * @return Result of writing the OLATA and OLATB register pair.
     */
    eurorack::io::IoResult writeOutputs(std::uint16_t outputMask) noexcept;

    /**
     * @brief Changes one cached output bit and writes the complete output mask.
     * @param pin Zero-based pin index in the inclusive range 0 through 15.
     * @param high True to set the output bit, false to clear it.
     * @return InvalidArgument for an out-of-range pin or the result of the I2C
     * output-register write.
     */
    eurorack::io::IoResult writePin(std::size_t pin, bool high) noexcept;

    /**
     * @brief Samples both GPIO input registers into the local cache.
     * @return Result of the combined register-address write and two-byte read.
     */
    eurorack::io::IoResult sampleInputs() noexcept;

    /**
     * @brief Returns one bit from the most recently sampled input cache.
     * @param pin Zero-based pin index.
     * @return Cached logical state, or false when @p pin is out of range.
     */
    [[nodiscard]] bool input(std::size_t pin) const noexcept;

    /**
     * @brief Returns the complete most recently sampled input mask.
     * @return GPIOA in bits 0 through 7 and GPIOB in bits 8 through 15.
     */
    [[nodiscard]] std::uint16_t inputMask() const noexcept;

    /**
     * @brief Returns the complete cached output-latch mask.
     * @return OLATA in bits 0 through 7 and OLATB in bits 8 through 15.
     */
    [[nodiscard]] std::uint16_t outputMask() const noexcept;

  private:
    /**
     * @brief Writes a consecutive pair of eight-bit registers.
     * @param firstRegister Address of the bank-A register.
     * @param value Sixteen-bit value written least-significant byte first.
     * @return Result returned by the I2C bus.
     */
    eurorack::io::IoResult writeRegister16(
        std::uint8_t firstRegister,
        std::uint16_t value) noexcept;

    /**
     * @brief Reads a consecutive pair of eight-bit registers.
     * @param firstRegister Address of the bank-A register.
     * @param value Destination receiving the combined little-endian value after
     * a successful read. It is unchanged on failure.
     * @return Result returned by the I2C bus.
     */
    eurorack::io::IoResult readRegister16(
        std::uint8_t firstRegister,
        std::uint16_t& value) noexcept;

    eurorack::io::I2cBus& bus_;              ///< Non-owning I2C bus reference.
    eurorack::io::I2cAddress address_;       ///< Seven-bit device address.
    std::uint16_t direction_{0xFFFFU};       ///< Cached direction mask.
    std::uint16_t pullups_{0U};              ///< Cached pull-up mask.
    std::uint16_t polarity_{0U};             ///< Cached polarity-inversion mask.
    std::uint16_t outputs_{0U};              ///< Cached output-latch mask.
    std::uint16_t inputs_{0U};               ///< Last sampled GPIO input mask.
};

} // namespace eurorack::drivers::gpio

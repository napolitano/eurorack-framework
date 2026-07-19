/**
 * @file include/eurorack/drivers/display/sh1106.hpp
 * @brief Declares an SH1106 monochrome OLED display driver.
 *
 * @details
 * Provides I2C initialization and page-oriented framebuffer transfer for common 128x64 panels.
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
 * @ingroup drivers
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <eurorack/display/monochrome_canvas.hpp>
#include <eurorack/io/i2c_bus.hpp>
#include <eurorack/io/io_result.hpp>

namespace eurorack::drivers::display {

/** @brief SH1106 I2C panel configuration. */
struct Sh1106Config final {
    eurorack::io::I2cAddress address{0x3CU};
    std::uint16_t width{128U};
    std::uint16_t height{64U};
    std::uint8_t columnOffset{2U};
    std::uint32_t i2cClockHz{400'000U};
    std::uint8_t contrast{0x7FU};
    bool segmentRemap{true};
    bool comScanRemapped{true};
    bool chargePumpEnabled{true};
};

/** @brief I2C driver for SH1106 monochrome OLED controllers. */
class Sh1106 final {
  public:
    /**
     * @brief Constructs an SH1106 driver.
     * @param bus I2C bus.
     * @param config Panel configuration.
     */
    Sh1106(eurorack::io::I2cBus& bus, Sh1106Config config = {}) noexcept;

    /** @brief Initializes the display controller. @return Success when the requested operation
     * completes; otherwise a specific validation, bus, or storage error. */
    [[nodiscard]] eurorack::io::IoResult initialize() noexcept;

    /**
     * @brief Transfers a complete canvas.
     * @param canvas Source canvas.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    [[nodiscard]] eurorack::io::IoResult
    flush(const eurorack::display::MonochromeCanvas& canvas) noexcept;

    /** @brief Sets display contrast. @param contrast Contrast. @return Success when the requested
     * operation completes; otherwise a specific validation, bus, or storage error. */
    [[nodiscard]] eurorack::io::IoResult setContrast(std::uint8_t contrast) noexcept;

    /** @brief Enables or disables inversion. @param inverted Desired state. @return Operation
     * result. */
    [[nodiscard]] eurorack::io::IoResult setInverted(bool inverted) noexcept;

    /** @brief Turns the display on or off. @param enabled Desired state. @return Success when the
     * requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    [[nodiscard]] eurorack::io::IoResult setDisplayEnabled(bool enabled) noexcept;

  private:
    /** @brief Writes one command. @param command Command byte. @return Success when the requested
     * operation completes; otherwise a specific validation, bus, or storage error. */
    [[nodiscard]] eurorack::io::IoResult writeCommand(std::uint8_t command) noexcept;

    /** @brief Writes command bytes. @param commands Source. @param size Count. @return Operation
     * result. */
    [[nodiscard]] eurorack::io::IoResult writeCommands(const std::uint8_t* commands,
                                                       std::size_t size) noexcept;

    /** @brief Writes display data. @param data Source. @param size Count. @return Success when the
     * requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    [[nodiscard]] eurorack::io::IoResult writeData(const std::uint8_t* data,
                                                   std::size_t size) noexcept;

    eurorack::io::I2cBus& bus_;
    Sh1106Config config_{};
    bool initialized_{false};
};

} // namespace eurorack::drivers::display

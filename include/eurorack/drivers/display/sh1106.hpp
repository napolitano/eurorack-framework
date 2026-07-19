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
    eurorack::io::I2cAddress address{0x3CU}; ///< 7-bit I2C address of the controller.
    std::uint16_t width{128U}; ///< Visible panel width in pixels; `flush` rejects a canvas whose
                               ///< width differs, and rejects widths above 128.
    std::uint16_t height{64U}; ///< Visible panel height in pixels; must be a non-zero multiple of
                               ///< 8, and `flush` rejects a canvas whose height differs.
    std::uint8_t columnOffset{2U}; ///< Column RAM offset applied before the visible image; SH1106
                                   ///< controllers expose 132 columns of RAM even for
                                   ///< 128-pixel-wide panels, so the visible area typically
                                   ///< starts at column 2.
    std::uint32_t i2cClockHz{400'000U}; ///< I2C bus clock applied by `initialize`.
    std::uint8_t contrast{0x7FU};       ///< Contrast value sent during `initialize`; see also
                                        ///< `setContrast`.
    bool segmentRemap{true};    ///< Selects the controller's column-address mapping (segment remap
                                ///< command 0xA1 vs. 0xA0) to match the panel's physical wiring.
    bool comScanRemapped{true}; ///< Selects the controller's row-scan direction (COM scan command
                                ///< 0xC8 vs. 0xC0) to match the panel's physical wiring.
    bool chargePumpEnabled{true}; ///< Enables the controller's internal charge pump (command 0x8B
                                  ///< vs. 0x8A), required by most panels that have no external
                                  ///< boost supply.
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

    eurorack::io::I2cBus& bus_; ///< I2C bus used for every command and data transfer; the driver
                                ///< does not own it.
    Sh1106Config config_{};     ///< Panel configuration applied by `initialize` and validated by
                                ///< `flush`.
    bool initialized_{false};   ///< True once `initialize` has completed successfully; `flush`
                                ///< returns `Busy` while false.
};

} // namespace eurorack::drivers::display

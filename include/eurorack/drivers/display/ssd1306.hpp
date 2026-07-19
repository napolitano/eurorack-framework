/**
 * @file include/eurorack/drivers/display/ssd1306.hpp
 * @brief Declares an SSD1306 monochrome OLED display driver.
 *
 * @details
 * Provides I2C initialization, framebuffer transfer, power, contrast, and inversion control.
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

/**
 * @brief Supported panel heights and their corresponding multiplex geometry.
 *
 * @details
 * The controller can address several panel sizes, but this driver explicitly
 * supports the common 32- and 64-row variants. Width remains separately
 * configurable.
 */
enum class Ssd1306Height : std::uint8_t { Height32 = 32U, Height64 = 64U };

/**
 * @brief Static bus, geometry, orientation, and power configuration.
 *
 * @details
 * The configuration is copied into the driver. It controls I2C address and
 * clock, visible dimensions, segment and COM orientation, contrast, and charge
 * pump setup. It does not own the bus or framebuffer.
 */
struct Ssd1306Config final {
    eurorack::io::I2cAddress address{0x3CU};
    std::uint16_t width{128U};
    Ssd1306Height height{Ssd1306Height::Height64};
    std::uint32_t i2cClockHz{400'000U};
    std::uint8_t contrast{0x7FU};
    bool segmentRemap{true};
    bool comScanRemapped{true};
    bool chargePumpEnabled{true};
};

/**
 * @brief I2C driver for SSD1306-compatible monochrome OLED controllers.
 *
 * @details
 * The driver owns configuration and a fixed page-conversion scratch buffer but
 * not the I2C bus or canvas storage. initialize() programs controller state;
 * flush() validates canvas geometry, transposes pixels into controller page
 * order, and sends bounded packets. Methods are synchronous and not thread-safe.
 */
class Ssd1306 final {
  public:
    Ssd1306(eurorack::io::I2cBus& bus, Ssd1306Config config = {}) noexcept;

    /**
     * @brief Initializes the controller and its addressing mode.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult initialize() noexcept;

    [[nodiscard]] eurorack::io::IoResult
    /**
     * @brief Transfers buffered state to the underlying controller.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
     */
    flush(const eurorack::display::MonochromeCanvas& canvas) noexcept;

    /**
     * @brief Sets contrast.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param contrast Value required by this operation.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult setContrast(std::uint8_t contrast) noexcept;

    /**
     * @brief Sets inverted.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param inverted Value required by this operation.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult setInverted(bool inverted) noexcept;

    /**
     * @brief Sets display enabled.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param enabled Value required by this operation.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult setDisplayEnabled(bool enabled) noexcept;

    /**
     * @brief Returns the configured display width.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested integer value in the units documented by the enclosing API.
     */
    [[nodiscard]] std::uint16_t width() const noexcept;

    /**
     * @brief Returns the configured display height.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested integer value in the units documented by the enclosing API.
     */
    [[nodiscard]] std::uint16_t height() const noexcept;

  private:
    /**
     * @brief Writes command.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param command Value required by this operation.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult writeCommand(std::uint8_t command) noexcept;

    /**
     * @brief Writes commands.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param commands Value required by this operation.
     *
     * @param size Number of bytes or elements involved.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult writeCommands(const std::uint8_t* commands,
                                                       std::size_t size) noexcept;

    /**
     * @brief Writes data.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param data Byte buffer used by the operation.
     *
     * @param size Number of bytes or elements involved.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult writeData(const std::uint8_t* data,
                                                   std::size_t size) noexcept;

    /**
     * @brief Provides the configure address window operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return Success when the complete operation succeeds; otherwise the specific error returned
     * by validation, storage, filesystem, or bus access.
     */
    [[nodiscard]] eurorack::io::IoResult configureAddressWindow() noexcept;

    eurorack::io::I2cBus& bus_;
    Ssd1306Config config_{};
    bool initialized_{false};
};

} // namespace eurorack::drivers::display

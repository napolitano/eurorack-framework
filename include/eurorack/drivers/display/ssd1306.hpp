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
    eurorack::io::I2cAddress address{0x3CU}; ///< 7-bit I2C address of the controller.
    std::uint16_t width{128U}; ///< Visible panel width in pixels; `flush` rejects a canvas whose
                               ///< width differs, zero, or a value above 128.
    Ssd1306Height height{
        Ssd1306Height::Height64}; ///< Visible panel height; selects the COM-pin
                                  ///< configuration byte and multiplex ratio sent by `initialize`.
    std::uint32_t i2cClockHz{400'000U}; ///< I2C bus clock applied by `initialize`.
    std::uint8_t contrast{0x7FU}; ///< Contrast value sent during `initialize`; kept in sync by
                                  ///< `setContrast`.
    bool segmentRemap{true};    ///< Selects the controller's column-address mapping (segment remap
                                ///< command 0xA1 vs. 0xA0) to match the panel's physical wiring.
    bool comScanRemapped{true}; ///< Selects the controller's row-scan direction (COM scan command
                                ///< 0xC8 vs. 0xC0) to match the panel's physical wiring.
    bool chargePumpEnabled{true}; ///< Enables the controller's internal charge pump and switches
                                  ///< the precharge period to the value recommended for it
                                  ///< (0x14/0xF1 vs. 0x10/0x22), for panels with no external
                                  ///< boost supply.
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
    /**
     * @brief Constructs an SSD1306 driver over an existing I2C bus.
     *
     * @param bus I2C bus used for every command and data transfer; the driver does not own it
     * and does not communicate until @ref initialize is called.
     * @param config Address, geometry, orientation, contrast, and charge-pump settings, copied
     * into the driver.
     */
    Ssd1306(eurorack::io::I2cBus& bus, Ssd1306Config config = {}) noexcept;

    /**
     * @brief Programs controller geometry, orientation, contrast, and charge pump, then turns
     * the display on.
     *
     * @details
     * Sets the I2C clock, sends the fixed SSD1306 initialization sequence built from `config`
     * (multiplex ratio, COM pin configuration, charge pump, segment remap, COM scan direction,
     * contrast, precharge period), configures the addressing window for the full panel via
     * @ref configureAddressWindow, and finally enables the display. On success, later calls to
     * @ref flush are accepted; `initialize` may be called again to reprogram the controller.
     *
     * @return Success when every command is accepted by the bus; otherwise the first error
     * returned by the bus, propagated unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult initialize() noexcept;

    /**
     * @brief Transposes a canvas into controller page order and transfers it over I2C.
     *
     * @details
     * Returns `Busy` if @ref initialize has not yet completed successfully. Returns
     * `InvalidArgument` if `config.width` is zero or above 128, or if the canvas dimensions do
     * not exactly match the configured width and height. Otherwise, for each 8-row page, packs
     * the framework's row-major pixels into the controller's vertical byte-per-column layout,
     * sets the page's column/page address window, and writes the page data in bounded I2C
     * chunks. A `Success` result means the bus accepted every byte; it does not confirm that the
     * panel physically displayed them.
     *
     * @param canvas Source canvas; must match the configured width and height exactly. The
     * caller retains ownership of the canvas and its buffer.
     * @return Success when every command and data packet is accepted by the bus; otherwise the
     * first error encountered, propagated unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult
    flush(const eurorack::display::MonochromeCanvas& canvas) noexcept;

    /**
     * @brief Sends a new contrast value and stores it in the configuration.
     *
     * @details
     * Writes the SSD1306 contrast command (0x81) followed by `contrast`. On success, the value
     * is also stored in the driver's copy of the configuration so it survives a later
     * @ref initialize call.
     *
     * @param contrast New contrast value.
     * @return Success when the bus accepts the command; otherwise the bus error, propagated
     * unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult setContrast(std::uint8_t contrast) noexcept;

    /**
     * @brief Enables or disables all-pixel inversion.
     *
     * @details
     * Writes the SSD1306 normal-display (0xA6) or inverted-display (0xA7) command.
     *
     * @param inverted True to invert pixel meaning (set pixels appear dark), false for normal
     * polarity.
     * @return Success when the bus accepts the command; otherwise the bus error, propagated
     * unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult setInverted(bool inverted) noexcept;

    /**
     * @brief Turns the panel output on or off without altering RAM contents or configuration.
     *
     * @details
     * Writes the SSD1306 display-off (0xAE) or display-on (0xAF) command.
     *
     * @param enabled True to turn the display on, false to turn it off.
     * @return Success when the bus accepts the command; otherwise the bus error, propagated
     * unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult setDisplayEnabled(bool enabled) noexcept;

    /**
     * @brief Returns the configured panel width.
     *
     * @return Width in pixels, as supplied in `Ssd1306Config::width`.
     */
    [[nodiscard]] std::uint16_t width() const noexcept;

    /**
     * @brief Returns the configured panel height.
     *
     * @return Height in pixels, derived from `Ssd1306Config::height`.
     */
    [[nodiscard]] std::uint16_t height() const noexcept;

  private:
    /**
     * @brief Sends a single-byte command.
     *
     * @details
     * Equivalent to calling `writeCommands(&command, 1)`.
     *
     * @param command Command byte.
     * @return Success when the bus accepts the command; otherwise the bus error, propagated
     * unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult writeCommand(std::uint8_t command) noexcept;

    /**
     * @brief Sends a sequence of command bytes as one I2C write.
     *
     * @details
     * Prefixes `commands` with the SSD1306 command control byte (0x00) and writes the combined
     * packet in a single bus transaction. Rejects a null pointer with a non-zero `size`, and
     * rejects `size` above 31 bytes (the fixed internal packet buffer capacity).
     *
     * @param commands Command bytes to send; may be null only if `size` is zero.
     * @param size Number of command bytes; must not exceed 31.
     * @return Success when the bus accepts the packet; `InvalidArgument` for a null buffer with
     * non-zero size or for `size` above 31; otherwise the bus error, propagated unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult writeCommands(const std::uint8_t* commands,
                                                       std::size_t size) noexcept;

    /**
     * @brief Sends display data, split into bus-sized chunks.
     *
     * @details
     * Prefixes each chunk with the SSD1306 data control byte (0x40) and writes it as its own bus
     * transaction, so the driver does not depend on a particular platform's I2C buffer capacity.
     * Chunks are at most 16 bytes. Rejects a null pointer with a non-zero `size`.
     *
     * @param data Data bytes to send; may be null only if `size` is zero.
     * @param size Number of data bytes.
     * @return Success when every chunk is accepted by the bus; otherwise the first bus error
     * encountered, propagated unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult writeData(const std::uint8_t* data,
                                                   std::size_t size) noexcept;

    /**
     * @brief Sets the controller's column and page address window to the full configured panel.
     *
     * @details
     * Sends the horizontal-addressing-mode column range (0 to `width - 1`) and page range (0 to
     * `height / 8 - 1`) commands so that a following data write advances through the whole
     * panel automatically.
     *
     * @return Success when the bus accepts the command; otherwise the bus error, propagated
     * unchanged.
     */
    [[nodiscard]] eurorack::io::IoResult configureAddressWindow() noexcept;

    eurorack::io::I2cBus& bus_; ///< I2C bus used for every command and data transfer; the driver
                                ///< does not own it.
    Ssd1306Config config_{};    ///< Address, geometry, orientation, contrast, and charge-pump
                                ///< settings; `contrast` is kept in sync by `setContrast`.
    bool initialized_{false};   ///< True once `initialize` has completed successfully; `flush`
                                ///< returns `Busy` while false.
};

} // namespace eurorack::drivers::display

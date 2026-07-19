/**
 * @file include/eurorack/drivers/shift/shift_register_74hc595.hpp
 * @brief Declares a buffered output driver for cascaded 74HC595 shift registers.
 *
 * @details
 * Output setters modify only a local byte buffer. flush() transfers the complete
 * chain and pulses the storage-register latch so all outputs update together.
 * An optional active-low output-enable pin can blank the complete chain without
 * changing the buffered output state.
 *
 * The class allocates its output buffer during construction. Bus and pin
 * dependencies are non-owning and must outlive the driver.
 *
 * @ingroup drivers
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>
#include <vector>

namespace eurorack::drivers::shift {

/**
 * @brief Cascaded 74HC595 serial-output shift-register driver.
 */
class ShiftRegister74Hc595 final {
  public:
    /**
     * @brief Constructs a driver for a fixed shift-register chain.
     * @param spi SPI bus that must outlive this object.
     * @param latch Storage-register latch output that must outlive this object.
     * @param deviceCount Number of cascaded 74HC595 devices. Each contributes
     * eight logical outputs.
     * @param outputEnable Optional active-low OE output. Pass nullptr when OE is
     * permanently enabled or managed externally.
     */
    ShiftRegister74Hc595(eurorack::io::SpiBus& spi,
                         eurorack::io::DigitalOutput& latch,
                         std::size_t deviceCount,
                         eurorack::io::DigitalOutput* outputEnable = nullptr) noexcept;

    /**
     * @brief Returns the number of logical output bits.
     * @return Eight times the configured device count.
     */
    [[nodiscard]] std::size_t outputCount() const noexcept;

    /**
     * @brief Changes one buffered output bit.
     * @param output Zero-based logical output index.
     * @param high True to set the bit, false to clear it.
     * @return Success or InvalidArgument when @p output is out of range.
     */
    eurorack::io::IoResult setOutput(std::size_t output, bool high) noexcept;

    /**
     * @brief Returns one buffered output bit.
     * @param output Zero-based logical output index.
     * @return Buffered state, or false when @p output is out of range.
     */
    [[nodiscard]] bool output(std::size_t output) const noexcept;

    /**
     * @brief Clears every buffered output without transferring it.
     * @return Success.
     */
    eurorack::io::IoResult clear() noexcept;

    /**
     * @brief Transfers the complete buffered chain and pulses the latch.
     * @return Success or the error returned by the SPI bus.
     */
    eurorack::io::IoResult flush() noexcept;

    /**
     * @brief Controls the optional active-low output-enable pin.
     * @param enabled True to enable physical outputs, false to place them in the
     * disabled state defined by the external circuit.
     */
    void setEnabled(bool enabled) noexcept;

  private:
    eurorack::io::SpiBus& spi_;                 ///< Non-owning SPI bus reference.
    eurorack::io::DigitalOutput& latch_;        ///< Storage-register latch output.
    eurorack::io::DigitalOutput* outputEnable_; ///< Optional active-low OE output.
    std::vector<std::uint8_t> buffer_;          ///< Buffered chain output bytes.
};

} // namespace eurorack::drivers::shift

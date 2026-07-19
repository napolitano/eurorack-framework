/**
 * @file include/eurorack/drivers/shift/shift_register_74hc165.hpp
 * @brief Declares a buffered input driver for cascaded 74HC165 shift registers.
 *
 * @details
 * The driver pulses the active-low parallel-load signal, clocks one byte per
 * configured device through the abstract SPI bus, and stores the sampled bits
 * in a local buffer. Input accessors never perform bus traffic and return the
 * most recently sampled state.
 *
 * The class allocates its sample buffer during construction. The SPI bus and
 * load output are non-owning dependencies and must outlive the driver.
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

#if defined(__AVR__)
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <cstddef>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>
#include <vector>

namespace eurorack::drivers::shift {

/**
 * @brief Cascaded 74HC165 parallel-input shift-register driver.
 */
class ShiftRegister74Hc165 final {
  public:
    /**
     * @brief Constructs a driver for a fixed shift-register chain.
     * @param spi SPI bus that must outlive this object.
     * @param load Active-low parallel-load output that must outlive this object.
     * @param deviceCount Number of cascaded 74HC165 devices. Each contributes
     * eight sampled inputs.
     */
    ShiftRegister74Hc165(eurorack::io::SpiBus& spi,
                         eurorack::io::DigitalOutput& load,
                         std::size_t deviceCount) noexcept;

    /**
     * @brief Returns the number of logical sampled inputs.
     * @return Eight times the configured device count.
     */
    [[nodiscard]] std::size_t inputCount() const noexcept;

    /**
     * @brief Captures parallel inputs and reads the complete serial chain.
     *
     * @details
     * The load line is pulsed low then high before the SPI transaction. On a
     * successful transfer, the local sample buffer contains one byte per device.
     * If the transfer fails, callers must treat the cached state as potentially
     * stale.
     *
     * @return Success or the error returned by the SPI bus.
     */
    eurorack::io::IoResult sample() noexcept;

    /**
     * @brief Returns one bit from the most recently sampled buffer.
     * @param input Zero-based logical input index.
     * @return Sampled state, or false when @p input is out of range.
     */
    [[nodiscard]] bool input(std::size_t input) const noexcept;

  private:
    eurorack::io::SpiBus& spi_;         ///< Non-owning SPI bus reference.
    eurorack::io::DigitalOutput& load_; ///< Active-low parallel-load output.
    std::vector<std::uint8_t> buffer_;  ///< Last sampled chain bytes.
};

} // namespace eurorack::drivers::shift

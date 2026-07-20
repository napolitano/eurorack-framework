/**
 * @file include/eurorack/drivers/adc/mcp3208.hpp
 * @brief Declares an allocation-free driver for the MCP3208 8-channel 12-bit SPI ADC.
 *
 * @details
 * The driver transmits one 3-byte command frame per conversion and decodes the 12-bit result
 * from the reply. The SPI bus and chip-select output are non-owning dependencies and must
 * outlive the driver. The class operates on raw ADC codes; reference-voltage selection, input
 * protection, and voltage calibration remain responsibilities of the consuming hardware and
 * firmware. Methods are synchronous and are not internally synchronized.
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
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>

namespace eurorack::drivers::adc {

/**
 * @brief Selects the MCP3208 single-ended or differential conversion mode.
 *
 * @details
 * SingleEnded measures one channel against the device's ground. Differential measures the
 * voltage between two adjacent channels; the MCP3208 datasheet defines the channel pairing.
 */
enum class Mcp3208InputMode : std::uint8_t {
    Differential, ///< Measures between a pair of adjacent channels.
    SingleEnded   ///< Measures one channel against ground.
};

/**
 * @brief Static MCP3208 bus configuration.
 */
struct Mcp3208Config final {
    std::uint32_t spiClockHertz{1'000'000U}; ///< SPI clock requested for every transaction.
};

/**
 * @brief Allocation-free MCP3208 8-channel, 12-bit SPI ADC driver.
 *
 * @details
 * Construction performs no SPI transfer; it places chip select high (inactive) only. Each call
 * to `read()` performs one complete, independent conversion.
 */
class Mcp3208 final {
  public:
    /**
     * @brief Constructs a driver around caller-owned bus and chip-select adapters.
     *
     * @param spi SPI bus used for all command transfers. It must outlive this object.
     * @param chipSelect Active-low chip-select output. It must outlive this object.
     * @param config SPI clock configuration.
     */
    Mcp3208(eurorack::io::SpiBus& spi,
            eurorack::io::DigitalOutput& chipSelect,
            Mcp3208Config config = {}) noexcept;

    /**
     * @brief Performs one conversion on the given channel.
     *
     * @details
     * Builds and transmits the MCP3208's 3-byte start/mode/channel command frame, then decodes
     * the 12-bit result from the final two reply bytes. Chip select is asserted only for the
     * duration of the transfer and is always restored high afterward, and the SPI transaction is
     * always closed, regardless of whether the transfer succeeded.
     *
     * @param channel Zero-based input channel; must be 0 through 7.
     * @param value Receives the 12-bit conversion result on success; left unchanged on failure.
     * @param mode Single-ended or differential conversion mode.
     * @return `InvalidArgument` if `channel` exceeds 7; otherwise `Success` once the transfer
     * completes, or the first bus error encountered.
     */
    eurorack::io::IoResult read(std::uint8_t channel,
                                std::uint16_t& value,
                                Mcp3208InputMode mode = Mcp3208InputMode::SingleEnded) noexcept;

  private:
    eurorack::io::SpiBus& spi_;       ///< Non-owning SPI bus reference.
    eurorack::io::DigitalOutput& cs_; ///< Active-low chip-select output.
    Mcp3208Config config_{};          ///< SPI clock configuration.
};

} // namespace eurorack::drivers::adc

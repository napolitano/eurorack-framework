/**
 * @file src/drivers/adc/mcp3208.cpp
 * @brief Implements MCP3208 command framing, SPI transaction cleanup, and 12-bit result decoding.
 *
 * @details
 * The implementation is allocation-free and keeps hardware access explicit through framework
 * interfaces.

 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */

#include <eurorack/drivers/adc/mcp3208.hpp>

namespace eurorack::drivers::adc {

Mcp3208::Mcp3208(eurorack::io::SpiBus& spi,
                 eurorack::io::DigitalOutput& chipSelect,
                 Mcp3208Config config) noexcept
    : spi_(spi), cs_(chipSelect), config_(config) {
    cs_.writeHigh(true);
}

eurorack::io::IoResult
Mcp3208::read(std::uint8_t channel, std::uint16_t& value, Mcp3208InputMode mode) noexcept {
    if (channel > 7U) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    const std::uint8_t modeAndChannel = static_cast<std::uint8_t>(
        ((mode == Mcp3208InputMode::SingleEnded ? 0x08U : 0U) | channel) << 4U);
    std::uint8_t tx[3] = {0x01U, modeAndChannel, 0U};
    std::uint8_t rx[3] = {};

    eurorack::io::IoResult result =
        spi_.beginTransaction({config_.spiClockHertz,
                               eurorack::io::SpiMode::Mode0,
                               eurorack::io::SpiBitOrder::MostSignificantBitFirst});
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    cs_.writeHigh(false);
    result = spi_.transfer(tx, rx, 3U);
    cs_.writeHigh(true);
    spi_.endTransaction();

    if (result == eurorack::io::IoResult::Success) {
        value = static_cast<std::uint16_t>(((rx[1] & 0x0FU) << 8U) | rx[2]);
    }

    return result;
}

} // namespace eurorack::drivers::adc

/**
 * @file src/drivers/dac/mcp4822.cpp
 * @brief Implements MCP4822 command framing, channel state, and optional LDAC control.
 *
 * @details
 * `frame()` builds the MCP4822's 16-bit command word directly from the buffered per-channel
 * state: bit 15 selects channel A or B, bit 13 carries the gain selection (set for `OneX`, clear
 * for `TwoX`), bit 12 is the active-high shutdown-disable bit (set when the channel is enabled),
 * and the low 12 bits carry the clamped DAC code unchanged. `write()` opens one SPI transaction
 * per frame, asserts chip select only for the two-byte transfer itself, and always restores chip
 * select and closes the transaction afterward regardless of the transfer's outcome, so a bus
 * error cannot leave the chip-select line stuck low. `flushBoth()` transfers channel A and then
 * channel B, returning immediately on the first failure without touching the LDAC pin; the pin is
 * pulsed only once both channels have already been written successfully, so a partial failure
 * never latches a mismatched pair of outputs. The implementation is allocation-free and keeps all
 * hardware access explicit through the injected framework interfaces.

 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */

#include <eurorack/drivers/dac/mcp4822.hpp>

namespace eurorack::drivers::dac {

namespace {
std::size_t idx(Mcp4822Channel channel) {
    return channel == Mcp4822Channel::A ? 0U : 1U;
}
} // namespace

Mcp4822::Mcp4822(eurorack::io::SpiBus& spi,
                 eurorack::io::DigitalOutput& chipSelect,
                 eurorack::io::DigitalOutput* loadDac,
                 Mcp4822Config config) noexcept
    : spi_(spi), cs_(chipSelect), ldac_(loadDac), config_(config) {
    cs_.writeHigh(true);
    if (ldac_ != nullptr) {
        ldac_->writeHigh(true);
    }
}

void Mcp4822::setCode(Mcp4822Channel channel, std::uint16_t code) noexcept {
    codes_[idx(channel)] = code > 4095U ? 4095U : code;
}

void Mcp4822::setGain(Mcp4822Channel channel, Mcp4822Gain gain) noexcept {
    gains_[idx(channel)] = gain;
}

void Mcp4822::setEnabled(Mcp4822Channel channel, bool enabled) noexcept {
    enabled_[idx(channel)] = enabled;
}

std::uint16_t Mcp4822::code(Mcp4822Channel channel) const noexcept {
    return codes_[idx(channel)];
}

std::uint16_t Mcp4822::frame(Mcp4822Channel channel) const noexcept {
    const std::size_t i = idx(channel);
    std::uint16_t f = channel == Mcp4822Channel::B ? 0x8000U : 0U;
    if (gains_[i] == Mcp4822Gain::OneX) {
        f |= 0x2000U;
    }
    if (enabled_[i]) {
        f |= 0x1000U;
    }
    return static_cast<std::uint16_t>(f | codes_[i]);
}

eurorack::io::IoResult Mcp4822::write(std::uint16_t frame) noexcept {
    eurorack::io::IoResult result =
        spi_.beginTransaction({config_.spiClockHertz,
                               eurorack::io::SpiMode::Mode0,
                               eurorack::io::SpiBitOrder::MostSignificantBitFirst});
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    std::uint8_t bytes[2] = {static_cast<std::uint8_t>(frame >> 8U),
                             static_cast<std::uint8_t>(frame)};
    cs_.writeHigh(false);
    result = spi_.transfer(bytes, nullptr, 2U);
    cs_.writeHigh(true);
    spi_.endTransaction();
    return result;
}

eurorack::io::IoResult Mcp4822::flushChannel(Mcp4822Channel channel) noexcept {
    return write(frame(channel));
}

eurorack::io::IoResult Mcp4822::flushBoth() noexcept {
    eurorack::io::IoResult result = flushChannel(Mcp4822Channel::A);
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    result = flushChannel(Mcp4822Channel::B);
    if (result == eurorack::io::IoResult::Success) {
        latchOutputs();
    }
    return result;
}

void Mcp4822::latchOutputs() noexcept {
    if (ldac_ != nullptr) {
        ldac_->writeHigh(false);
        ldac_->writeHigh(true);
    }
}

} // namespace eurorack::drivers::dac

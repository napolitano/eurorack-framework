/**
 * @file src/drivers/dac/mcp4922.cpp
 * @brief Implements the MCP4922 driver.
 *
 * @details
 * `buildFrame()` constructs the MCP4922's 16-bit command word directly from the buffered
 * per-channel state: bit 15 selects channel A or B, bit 14 is the configurable reference-buffer
 * bit (set only when `referenceBuffer` is `Buffered`), bit 13 carries the gain selection (set for
 * `OneX`, clear for `TwoX`), bit 12 is the active-high shutdown-disable bit, and the low 12 bits
 * carry the clamped DAC code unchanged. `writeFrame()` opens one SPI transaction per frame,
 * asserts chip select only for the two-byte transfer itself, and always restores chip select and
 * closes the transaction afterward regardless of the transfer's outcome. `flushBoth()` transfers
 * channel A and then channel B, returning immediately on the first failure without touching the
 * LDAC pin; the pin is pulsed only once both channels have already been written successfully, so
 * a partial failure never latches a mismatched pair of outputs.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#include <eurorack/drivers/dac/mcp4922.hpp>
namespace eurorack::drivers::dac {
namespace {
std::size_t ix(Mcp4922Channel c) noexcept {
    return c == Mcp4922Channel::A ? 0U : 1U;
}
} // namespace
Mcp4922::Mcp4922(eurorack::io::SpiBus& spi,
                 eurorack::io::DigitalOutput& cs,
                 eurorack::io::DigitalOutput* ldac,
                 Mcp4922Config config) noexcept
    : spi_(spi), cs_(cs), ldac_(ldac), config_(config) {
    cs_.writeHigh(true);
    if (ldac_ != nullptr)
        ldac_->writeHigh(true);
}
void Mcp4922::setCode(Mcp4922Channel c, std::uint16_t v) noexcept {
    codes_[ix(c)] = v > 0x0FFFU ? 0x0FFFU : v;
}
std::uint16_t Mcp4922::code(Mcp4922Channel c) const noexcept {
    return codes_[ix(c)];
}
void Mcp4922::setGain(Mcp4922Channel c, Mcp4922Gain g) noexcept {
    gains_[ix(c)] = g;
}
void Mcp4922::setEnabled(Mcp4922Channel c, bool e) noexcept {
    enabled_[ix(c)] = e;
}
void Mcp4922::setReferenceBuffer(Mcp4922ReferenceBuffer m) noexcept {
    config_.referenceBuffer = m;
}
Mcp4922ReferenceBuffer Mcp4922::referenceBuffer() const noexcept {
    return config_.referenceBuffer;
}
std::uint16_t Mcp4922::buildFrame(Mcp4922Channel c) const noexcept {
    auto i = ix(c);
    std::uint16_t f = c == Mcp4922Channel::B ? 0x8000U : 0U;
    if (config_.referenceBuffer == Mcp4922ReferenceBuffer::Buffered)
        f = static_cast<std::uint16_t>(f | 0x4000U);
    if (gains_[i] == Mcp4922Gain::OneX)
        f = static_cast<std::uint16_t>(f | 0x2000U);
    if (enabled_[i])
        f = static_cast<std::uint16_t>(f | 0x1000U);
    return static_cast<std::uint16_t>(f | codes_[i]);
}
eurorack::io::IoResult Mcp4922::writeFrame(std::uint16_t f) noexcept {
    const std::uint8_t d[2] = {static_cast<std::uint8_t>(f >> 8U),
                               static_cast<std::uint8_t>(f & 0xFFU)};
    const eurorack::io::SpiSettings s{config_.spiClockHertz,
                                      eurorack::io::SpiMode::Mode0,
                                      eurorack::io::SpiBitOrder::MostSignificantBitFirst};
    auto r = spi_.beginTransaction(s);
    if (r != eurorack::io::IoResult::Success)
        return r;
    cs_.writeHigh(false);
    r = spi_.transfer(d, nullptr, 2U);
    cs_.writeHigh(true);
    spi_.endTransaction();
    return r;
}
eurorack::io::IoResult Mcp4922::flushChannel(Mcp4922Channel c) noexcept {
    return writeFrame(buildFrame(c));
}
eurorack::io::IoResult Mcp4922::flushBoth() noexcept {
    auto r = flushChannel(Mcp4922Channel::A);
    if (r != eurorack::io::IoResult::Success)
        return r;
    r = flushChannel(Mcp4922Channel::B);
    if (r == eurorack::io::IoResult::Success)
        latchOutputs();
    return r;
}
void Mcp4922::latchOutputs() noexcept {
    if (ldac_ != nullptr) {
        ldac_->writeHigh(false);
        ldac_->writeHigh(true);
    }
}
} // namespace eurorack::drivers::dac

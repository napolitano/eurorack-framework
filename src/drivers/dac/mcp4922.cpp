/**
 * @file src/drivers/dac/mcp4922.cpp
 * @brief Implements the MCP4922 dual DAC driver. *
 * @details
 * Each 16-bit frame combines channel selection, buffering, gain, shutdown
 * state, and the 12-bit data code. Desired codes are buffered independently for
 * both channels. Voltage scaling, output amplification, and calibration remain
 * outside this transport-level driver.

 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <algorithm>
#include <eurorack/drivers/dac/mcp4922.hpp>
namespace eurorack::drivers::dac {
namespace {
std::size_t ix(const Mcp4922Channel c) noexcept {
    return c == Mcp4922Channel::A ? 0U : 1U;
}
} // namespace
Mcp4922::Mcp4922(eurorack::io::SpiBus& spi,
                 eurorack::io::DigitalOutput& cs,
                 eurorack::io::DigitalOutput* const ldac) noexcept
    : spi_(spi), cs_(cs), ldac_(ldac) {
    cs_.writeHigh(true);
    if (ldac_ != nullptr)
        ldac_->writeHigh(true);
}
void Mcp4922::setCode(const Mcp4922Channel c, const std::uint16_t v) noexcept {
    codes_[ix(c)] = std::min<std::uint16_t>(v, 0x0FFFU);
}
std::uint16_t Mcp4922::code(const Mcp4922Channel c) const noexcept {
    return codes_[ix(c)];
}
void Mcp4922::setGain(const Mcp4922Channel c, const Mcp4922Gain g) noexcept {
    gains_[ix(c)] = g;
}
void Mcp4922::setEnabled(const Mcp4922Channel c, const bool e) noexcept {
    enabled_[ix(c)] = e;
}
std::uint16_t Mcp4922::buildFrame(const Mcp4922Channel c) const noexcept {
    const auto i = ix(c);
    std::uint16_t f = c == Mcp4922Channel::B ? 0x8000U : 0U;
    f = static_cast<std::uint16_t>(f | 0x4000U);
    if (gains_[i] == Mcp4922Gain::OneX)
        f = static_cast<std::uint16_t>(f | 0x2000U);
    if (enabled_[i])
        f = static_cast<std::uint16_t>(f | 0x1000U);
    return static_cast<std::uint16_t>(f | codes_[i]);
}
eurorack::io::IoResult Mcp4922::writeFrame(const std::uint16_t f) noexcept {
    const std::uint8_t d[2] = {static_cast<std::uint8_t>(f >> 8U),
                               static_cast<std::uint8_t>(f & 0xFFU)};
    const eurorack::io::SpiSettings s{20000000U,
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
eurorack::io::IoResult Mcp4922::flushChannel(const Mcp4922Channel c) noexcept {
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

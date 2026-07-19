/**
 * @file src/drivers/gpio/mcp23017.cpp
 * @brief Implements the MCP23017 GPIO expander driver. *
 * @details
 * The driver maintains software shadows for direction, polarity, pull-up, and
 * output registers. Bit-level setters update the relevant shadow and then write
 * the corresponding register pair. A failed bus write leaves the attempted
 * shadow value visible; callers requiring rollback must perform it explicitly.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/drivers/gpio/mcp23017.hpp>
namespace eurorack::drivers::gpio {
namespace {
constexpr std::uint8_t IODIRA = 0x00U, IPOLA = 0x02U, GPPUA = 0x0CU, GPIOA = 0x12U, OLATA = 0x14U;
}
Mcp23017::Mcp23017(eurorack::io::I2cBus& b, const eurorack::io::I2cAddress a) noexcept
    : bus_(b), address_(a) {}
eurorack::io::IoResult Mcp23017::initialize() noexcept {
    auto r = setDirection(direction_);
    if (r != eurorack::io::IoResult::Success)
        return r;
    r = setPullUps(pullups_);
    if (r != eurorack::io::IoResult::Success)
        return r;
    r = setInputPolarity(polarity_);
    return r == eurorack::io::IoResult::Success ? writeOutputs(outputs_) : r;
}
eurorack::io::IoResult Mcp23017::setDirection(const std::uint16_t v) noexcept {
    direction_ = v;
    return writeRegister16(IODIRA, v);
}
eurorack::io::IoResult Mcp23017::setPullUps(const std::uint16_t v) noexcept {
    pullups_ = v;
    return writeRegister16(GPPUA, v);
}
eurorack::io::IoResult Mcp23017::setInputPolarity(const std::uint16_t v) noexcept {
    polarity_ = v;
    return writeRegister16(IPOLA, v);
}
eurorack::io::IoResult Mcp23017::writeOutputs(const std::uint16_t v) noexcept {
    outputs_ = v;
    return writeRegister16(OLATA, v);
}
eurorack::io::IoResult Mcp23017::writePin(const std::size_t p, const bool h) noexcept {
    if (p >= 16U)
        return eurorack::io::IoResult::InvalidArgument;
    const auto m = static_cast<std::uint16_t>(1U << p);
    outputs_ = h ? static_cast<std::uint16_t>(outputs_ | m)
                 : static_cast<std::uint16_t>(outputs_ & static_cast<std::uint16_t>(~m));
    return writeOutputs(outputs_);
}
eurorack::io::IoResult Mcp23017::sampleInputs() noexcept {
    return readRegister16(GPIOA, inputs_);
}
bool Mcp23017::input(const std::size_t p) const noexcept {
    return p < 16U && (inputs_ & static_cast<std::uint16_t>(1U << p)) != 0U;
}
std::uint16_t Mcp23017::inputMask() const noexcept {
    return inputs_;
}
std::uint16_t Mcp23017::outputMask() const noexcept {
    return outputs_;
}
eurorack::io::IoResult Mcp23017::writeRegister16(const std::uint8_t r,
                                                 const std::uint16_t v) noexcept {
    const std::uint8_t d[3] = {
        r, static_cast<std::uint8_t>(v & 0xFFU), static_cast<std::uint8_t>(v >> 8U)};
    return bus_.write(address_, d, 3U);
}
eurorack::io::IoResult Mcp23017::readRegister16(const std::uint8_t r, std::uint16_t& v) noexcept {
    std::uint8_t d[2]{};
    const auto x = bus_.writeRead(address_, &r, 1U, d, 2U);
    if (x == eurorack::io::IoResult::Success)
        v = static_cast<std::uint16_t>(
            d[0] | static_cast<std::uint16_t>(static_cast<std::uint16_t>(d[1]) << 8U));
    return x;
}
} // namespace eurorack::drivers::gpio

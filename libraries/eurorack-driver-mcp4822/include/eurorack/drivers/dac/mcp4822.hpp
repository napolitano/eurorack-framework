/** @file mcp4822.hpp
 * @brief Declares an allocation-free MCP4822 dual DAC driver.
 */
/**
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#pragma once
#include <eurorack/compat/avr/array.hpp>
#include <eurorack/compat/avr/cstdint.hpp>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>
namespace eurorack::drivers::dac {
enum class Mcp4822Channel : std::uint8_t { A, B }; enum class Mcp4822Gain : std::uint8_t { OneX, TwoX };
struct Mcp4822Config final { std::uint32_t spiClockHertz{8'000'000U}; };
class Mcp4822 final { public: Mcp4822(eurorack::io::SpiBus&,eurorack::io::DigitalOutput&,eurorack::io::DigitalOutput* =nullptr,Mcp4822Config={}) noexcept; void setCode(Mcp4822Channel,std::uint16_t) noexcept; void setGain(Mcp4822Channel,Mcp4822Gain) noexcept; void setEnabled(Mcp4822Channel,bool) noexcept; [[nodiscard]] std::uint16_t code(Mcp4822Channel) const noexcept; eurorack::io::IoResult flushChannel(Mcp4822Channel) noexcept; eurorack::io::IoResult flushBoth() noexcept; void latchOutputs() noexcept; private: [[nodiscard]] std::uint16_t frame(Mcp4822Channel) const noexcept; eurorack::io::IoResult write(std::uint16_t) noexcept; eurorack::io::SpiBus& spi_; eurorack::io::DigitalOutput& cs_; eurorack::io::DigitalOutput* ldac_; Mcp4822Config config_{}; std::array<std::uint16_t,2> codes_{}; std::array<Mcp4822Gain,2> gains_{Mcp4822Gain::OneX,Mcp4822Gain::OneX}; std::array<bool,2> enabled_{true,true}; };
}

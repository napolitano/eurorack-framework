/** @file mcp3208.hpp
 * @brief Declares an allocation-free MCP3208 SPI ADC driver.
 */
/**
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#pragma once
#include <eurorack/compat/avr/cstdint.hpp>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>
namespace eurorack::drivers::adc {
enum class Mcp3208InputMode : std::uint8_t { Differential, SingleEnded }; struct Mcp3208Config final { std::uint32_t spiClockHertz{1'000'000U}; };
class Mcp3208 final { public: Mcp3208(eurorack::io::SpiBus&,eurorack::io::DigitalOutput&,Mcp3208Config={}) noexcept; eurorack::io::IoResult read(std::uint8_t channel,std::uint16_t& value,Mcp3208InputMode mode=Mcp3208InputMode::SingleEnded) noexcept; private: eurorack::io::SpiBus& spi_;eurorack::io::DigitalOutput& cs_;Mcp3208Config config_{}; };
}

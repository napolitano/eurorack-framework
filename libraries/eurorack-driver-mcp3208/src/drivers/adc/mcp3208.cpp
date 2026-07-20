/**
 * @file mcp3208.cpp
 * @brief Implements MCP3208 command framing, SPI transaction cleanup, and 12-bit result decoding.
 * @details The implementation is allocation-free and keeps hardware access explicit through framework interfaces.
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#include <eurorack/drivers/adc/mcp3208.hpp>
namespace eurorack::drivers::adc { Mcp3208::Mcp3208(eurorack::io::SpiBus&s,eurorack::io::DigitalOutput&cs,Mcp3208Config c) noexcept:spi_(s),cs_(cs),config_(c){cs_.writeHigh(true);} eurorack::io::IoResult Mcp3208::read(std::uint8_t ch,std::uint16_t& value,Mcp3208InputMode mode) noexcept{if(ch>7U)return eurorack::io::IoResult::InvalidArgument;std::uint8_t tx[3]={0x01U,static_cast<std::uint8_t>(((mode==Mcp3208InputMode::SingleEnded?0x08U:0U)|ch)<<4U),0U};std::uint8_t rx[3]={};auto r=spi_.beginTransaction({config_.spiClockHertz,eurorack::io::SpiMode::Mode0,eurorack::io::SpiBitOrder::MostSignificantBitFirst});if(r!=eurorack::io::IoResult::Success)return r;cs_.writeHigh(false);r=spi_.transfer(tx,rx,3U);cs_.writeHigh(true);spi_.endTransaction();if(r==eurorack::io::IoResult::Success)value=static_cast<std::uint16_t>(((rx[1]&0x0FU)<<8U)|rx[2]);return r;} }

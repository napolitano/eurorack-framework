/**
 * @file mcp4822.cpp
 * @brief Implements MCP4822 command framing, channel state, and optional LDAC control.
 * @details The implementation is allocation-free and keeps hardware access explicit through framework interfaces.
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#include <eurorack/drivers/dac/mcp4822.hpp>
namespace eurorack::drivers::dac { namespace { std::size_t idx(Mcp4822Channel c){return c==Mcp4822Channel::A?0U:1U;} }
Mcp4822::Mcp4822(eurorack::io::SpiBus& s,eurorack::io::DigitalOutput& cs,eurorack::io::DigitalOutput* l,Mcp4822Config c) noexcept:spi_(s),cs_(cs),ldac_(l),config_(c){cs_.writeHigh(true);if(ldac_)ldac_->writeHigh(true);}
void Mcp4822::setCode(Mcp4822Channel c,std::uint16_t v) noexcept{codes_[idx(c)]=v>4095U?4095U:v;} void Mcp4822::setGain(Mcp4822Channel c,Mcp4822Gain g) noexcept{gains_[idx(c)]=g;} void Mcp4822::setEnabled(Mcp4822Channel c,bool e) noexcept{enabled_[idx(c)]=e;} std::uint16_t Mcp4822::code(Mcp4822Channel c) const noexcept{return codes_[idx(c)];}
std::uint16_t Mcp4822::frame(Mcp4822Channel c) const noexcept{auto i=idx(c);std::uint16_t f=c==Mcp4822Channel::B?0x8000U:0U; if(gains_[i]==Mcp4822Gain::OneX)f|=0x2000U; if(enabled_[i])f|=0x1000U; return static_cast<std::uint16_t>(f|codes_[i]);}
eurorack::io::IoResult Mcp4822::write(std::uint16_t f) noexcept{auto r=spi_.beginTransaction({config_.spiClockHertz,eurorack::io::SpiMode::Mode0,eurorack::io::SpiBitOrder::MostSignificantBitFirst});if(r!=eurorack::io::IoResult::Success)return r;std::uint8_t b[2]={static_cast<std::uint8_t>(f>>8U),static_cast<std::uint8_t>(f)};cs_.writeHigh(false);r=spi_.transfer(b,nullptr,2U);cs_.writeHigh(true);spi_.endTransaction();return r;}
eurorack::io::IoResult Mcp4822::flushChannel(Mcp4822Channel c) noexcept{return write(frame(c));} eurorack::io::IoResult Mcp4822::flushBoth() noexcept{auto r=flushChannel(Mcp4822Channel::A);if(r!=eurorack::io::IoResult::Success)return r;r=flushChannel(Mcp4822Channel::B);if(r==eurorack::io::IoResult::Success)latchOutputs();return r;} void Mcp4822::latchOutputs() noexcept{if(ldac_){ldac_->writeHigh(false);ldac_->writeHigh(true);}}
}

#include "../driver_example_support.hpp"
#include <eurorack/drivers/dac/mcp4922.hpp>
int main(){ example::SpiBus spi; example::DigitalOutput cs,ldac; eurorack::drivers::dac::Mcp4922 dac(spi,cs,&ldac); dac.setCode(eurorack::drivers::dac::Mcp4922Channel::A,2048U); dac.setGain(eurorack::drivers::dac::Mcp4922Channel::A,eurorack::drivers::dac::Mcp4922Gain::OneX); const auto r=dac.flushBoth(); return r==eurorack::io::IoResult::Success && spi.transfers()==2U ? 0:1; }

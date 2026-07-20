#include "../driver_example_support.hpp"
#include <eurorack/drivers/adc/mcp3208.hpp>
int main(){ example::SpiBus spi; example::DigitalOutput cs; eurorack::drivers::adc::Mcp3208 adc(spi,cs); std::uint16_t value=0U; const auto r=adc.read(3U,value,eurorack::drivers::adc::Mcp3208InputMode::SingleEnded); return r==eurorack::io::IoResult::Success && spi.transfers()==1U ? 0:1; }

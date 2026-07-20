#include "../driver_example_support.hpp"
#include <array>
#include <eurorack/drivers/led/tlc5947.hpp>
int main(){ example::SpiBus spi; example::DigitalOutput latch,oe; std::array<std::uint16_t,24U> values{}; std::array<std::uint8_t,36U> frame{}; eurorack::drivers::led::Tlc5947 leds(spi,latch,values.data(),values.size(),frame.data(),frame.size(),1U,&oe); if(!leds.valid()||leds.initialize()!=eurorack::io::IoResult::Success) return 1; if(leds.setBrightness(0U,32768U)!=eurorack::io::IoResult::Success) return 1; return leds.flush()==eurorack::io::IoResult::Success ? 0:1; }

#include "../driver_example_support.hpp"
#include <array>
#include <eurorack/drivers/mux/analog_multiplexer.hpp>
int main(){ example::DigitalOutput s0,s1,s2; example::AnalogInput adc(3072U); example::Delay delay; std::array<eurorack::io::DigitalOutput*,3U> select{&s0,&s1,&s2}; eurorack::drivers::mux::AnalogMultiplexer<3U> mux(select,adc,delay,5U); const auto sample=mux.readChannel(5U); return sample.result==eurorack::io::IoResult::Success && sample.code==3072U && mux.currentChannel()==5U ? 0:1; }

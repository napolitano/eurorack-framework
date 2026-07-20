#include "../driver_example_support.hpp"

#include <array>
#include <eurorack/drivers/mux/analog_multiplexer.hpp>

int main() {
    example::DigitalOutput select0;
    example::DigitalOutput select1;
    example::DigitalOutput select2;
    example::AnalogInput adc(3072U);
    example::Delay delay;

    std::array<eurorack::io::DigitalOutput*, 3U> selectPins{&select0, &select1, &select2};
    eurorack::drivers::mux::AnalogMultiplexer<3U> multiplexer(selectPins, adc, delay, 5U);

    const auto sample = multiplexer.readChannel(5U);
    const bool validSample =
        sample.result == eurorack::io::IoResult::Success && sample.code == 3072U;
    return validSample && multiplexer.currentChannel() == 5U ? 0 : 1;
}

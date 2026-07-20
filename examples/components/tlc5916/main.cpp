#include "../driver_example_support.hpp"

#include <eurorack/drivers/led/tlc5916.hpp>

int main() {
    example::SpiBus spi;
    example::DigitalOutput latch;
    example::DigitalOutput outputEnable;
    eurorack::drivers::led::Tlc5916 leds(spi, latch, 1U, &outputEnable);

    if (leds.setBrightness(2U, 65535U) != eurorack::io::IoResult::Success) {
        return 1;
    }

    return leds.flush() == eurorack::io::IoResult::Success && leds.brightness(2U) != 0U ? 0 : 1;
}

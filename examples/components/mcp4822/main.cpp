#include "../driver_example_support.hpp"

#include <eurorack/drivers/dac/mcp4822.hpp>

int main() {
    example::SpiBus spi;
    example::DigitalOutput chipSelect;
    example::DigitalOutput ldac;
    eurorack::drivers::dac::Mcp4822 dac(spi, chipSelect, &ldac);

    dac.setCode(eurorack::drivers::dac::Mcp4822Channel::A, 1024U);
    dac.setGain(eurorack::drivers::dac::Mcp4822Channel::A,
                eurorack::drivers::dac::Mcp4822Gain::TwoX);

    return dac.flushBoth() == eurorack::io::IoResult::Success && spi.transfers() == 2U ? 0 : 1;
}

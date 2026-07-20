#include "../driver_example_support.hpp"

#include <eurorack/drivers/dac/dac8568.hpp>

int main() {
    example::SpiBus spi;
    example::DigitalOutput sync;
    eurorack::drivers::dac::Dac8568 dac(spi, sync);

    dac.setCode(eurorack::drivers::dac::Dac8568Channel::A, 32768U);
    const auto result = dac.writeAndUpdate(eurorack::drivers::dac::Dac8568Channel::A);

    return result == eurorack::io::IoResult::Success && spi.transfers() == 1U ? 0 : 1;
}

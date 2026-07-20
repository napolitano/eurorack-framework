#include "../driver_example_support.hpp"

#include <eurorack/drivers/shift/shift_register_74hc165.hpp>

int main() {
    example::SpiBus spi;
    example::DigitalOutput parallelLoad;
    eurorack::drivers::shift::ShiftRegister74Hc165 inputs(spi, parallelLoad, 2U);

    const auto result = inputs.sample();
    return result == eurorack::io::IoResult::Success && inputs.inputCount() == 16U ? 0 : 1;
}

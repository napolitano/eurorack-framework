#include "../driver_example_support.hpp"

#include <eurorack/drivers/shift/shift_register_74hc595.hpp>

int main() {
    example::SpiBus spi;
    example::DigitalOutput latch;
    example::DigitalOutput outputEnable;
    eurorack::drivers::shift::ShiftRegister74Hc595 outputs(spi, latch, 2U, &outputEnable);

    if (outputs.setOutput(9U, true) != eurorack::io::IoResult::Success) {
        return 1;
    }

    outputs.setEnabled(true);
    return outputs.flush() == eurorack::io::IoResult::Success && outputs.output(9U) ? 0 : 1;
}

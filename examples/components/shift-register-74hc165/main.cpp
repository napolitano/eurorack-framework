#include "../driver_example_support.hpp"
#include <eurorack/drivers/shift/shift_register_74hc165.hpp>
int main(){ example::SpiBus spi; example::DigitalOutput load; eurorack::drivers::shift::ShiftRegister74Hc165 inputs(spi,load,2U); const auto r=inputs.sample(); return r==eurorack::io::IoResult::Success && inputs.inputCount()==16U ? 0:1; }

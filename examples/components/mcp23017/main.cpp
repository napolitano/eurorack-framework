#include "../driver_example_support.hpp"
#include <eurorack/drivers/gpio/mcp23017.hpp>
int main(){ example::I2cBus bus; eurorack::drivers::gpio::Mcp23017 gpio(bus,eurorack::io::I2cAddress{0x20U}); if(gpio.initialize()!=eurorack::io::IoResult::Success) return 1; if(gpio.setDirection(0xFF00U)!=eurorack::io::IoResult::Success) return 1; return gpio.writePin(0U,true)==eurorack::io::IoResult::Success ? 0:1; }

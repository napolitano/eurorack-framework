#include "../driver_example_support.hpp"
#include <array>
#include <eurorack/drivers/display/sh1106.hpp>
int main(){ example::I2cBus bus; eurorack::drivers::display::Sh1106 display(bus); std::array<std::uint8_t,1024U> pixels{}; eurorack::display::MonochromeCanvas canvas(pixels.data(),pixels.size(),128,64); canvas.applyPixel({10,10},eurorack::display::PixelOperation::Set); if(display.initialize()!=eurorack::io::IoResult::Success) return 1; return display.flush(canvas)==eurorack::io::IoResult::Success ? 0:1; }

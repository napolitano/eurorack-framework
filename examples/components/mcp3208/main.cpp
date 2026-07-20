#include "../driver_example_support.hpp"

#include <cstdint>
#include <eurorack/drivers/adc/mcp3208.hpp>

int main() {
    example::SpiBus spi;
    example::DigitalOutput chipSelect;
    eurorack::drivers::adc::Mcp3208 adc(spi, chipSelect);

    std::uint16_t value = 0U;
    const auto result = adc.read(3U, value, eurorack::drivers::adc::Mcp3208InputMode::SingleEnded);

    return result == eurorack::io::IoResult::Success && spi.transfers() == 1U ? 0 : 1;
}

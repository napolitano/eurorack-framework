#include <eurorack/drivers/adc/mcp3208.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
using namespace eurorack;
namespace {
void single_ended_decode() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::adc::Mcp3208 a(spi, cs);
    spi.queueResponse({0, 0x0A, 0xBC});
    std::uint16_t v = 0;
    TEST_ASSERT_EQUAL_INT(0, (int)a.read(5, v));
    TEST_ASSERT_EQUAL_UINT16(0xABCU, v);
    TEST_ASSERT_EQUAL_HEX8(0xD0U, spi.transfers()[0].transmitted[1]);
    TEST_ASSERT_TRUE(cs.lastWrittenHigh());
}
void differential_command() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::adc::Mcp3208 a(spi, cs);
    spi.queueResponse({0, 0x01, 0x23});
    std::uint16_t v = 0;
    TEST_ASSERT_EQUAL_INT(0, (int)a.read(3, v, drivers::adc::Mcp3208InputMode::Differential));
    TEST_ASSERT_EQUAL_UINT16(0x123U, v);
    TEST_ASSERT_EQUAL_HEX8(0x30U, spi.transfers()[0].transmitted[1]);
}
void rejects_and_cleans() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::adc::Mcp3208 a(spi, cs);
    std::uint16_t v = 77;
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument, (int)a.read(8, v));
    TEST_ASSERT_EQUAL_UINT16(77U, v);
    spi.setNextResult(io::IoResult::BusError);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError, (int)a.read(0, v));
    TEST_ASSERT_TRUE(cs.lastWrittenHigh());
}
} // namespace
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(single_ended_decode);
    RUN_TEST(differential_command);
    RUN_TEST(rejects_and_cleans);
    return UNITY_END();
}

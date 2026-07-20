#include <eurorack/drivers/dac/mcp4822.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
using namespace eurorack;
namespace {
void frame_and_clamp() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs, ldac;
    drivers::dac::Mcp4822 d(spi, cs, &ldac);
    d.setCode(drivers::dac::Mcp4822Channel::A, 6000U);
    d.setGain(drivers::dac::Mcp4822Channel::A, drivers::dac::Mcp4822Gain::OneX);
    TEST_ASSERT_EQUAL_UINT16(4095U, d.code(drivers::dac::Mcp4822Channel::A));
    TEST_ASSERT_EQUAL_INT(0, (int)d.flushChannel(drivers::dac::Mcp4822Channel::A));
    auto b = spi.transfers()[0].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x3FU, b[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFFU, b[1]);
}
void disabled_b_and_latch() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs, ldac;
    drivers::dac::Mcp4822 d(spi, cs, &ldac);
    d.setCode(drivers::dac::Mcp4822Channel::B, 0x456U);
    d.setGain(drivers::dac::Mcp4822Channel::B, drivers::dac::Mcp4822Gain::TwoX);
    d.setEnabled(drivers::dac::Mcp4822Channel::B, false);
    TEST_ASSERT_EQUAL_INT(0, (int)d.flushBoth());
    TEST_ASSERT_EQUAL_UINT32(2U, spi.transfers().size());
    TEST_ASSERT_EQUAL_HEX8(0x80U, spi.transfers()[1].transmitted[0] & 0xF0U);
    TEST_ASSERT_TRUE(ldac.transitionCount() >= 2U);
}
void bus_error_stops_second_channel() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::dac::Mcp4822 d(spi, cs);
    spi.setNextResult(io::IoResult::BusError);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError, (int)d.flushBoth());
    TEST_ASSERT_EQUAL_UINT32(1U, spi.transfers().size());
    TEST_ASSERT_TRUE(cs.lastWrittenHigh());
    d.latchOutputs();
}
} // namespace
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(frame_and_clamp);
    RUN_TEST(disabled_b_and_latch);
    RUN_TEST(bus_error_stops_second_channel);
    return UNITY_END();
}

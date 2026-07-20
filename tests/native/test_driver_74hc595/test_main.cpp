#include <eurorack/drivers/shift/shift_register_74hc595.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
using namespace eurorack;
namespace {
void buffers_flushes_and_clears() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput latch, oe;
    drivers::shift::ShiftRegister74Hc595 d(spi, latch, 2U, &oe);
    TEST_ASSERT_EQUAL_UINT32(16U, d.outputCount());
    TEST_ASSERT_FALSE(oe.lastWrittenHigh());
    TEST_ASSERT_EQUAL_INT(0, (int)d.setOutput(0U, true));
    TEST_ASSERT_EQUAL_INT(0, (int)d.setOutput(9U, true));
    TEST_ASSERT_TRUE(d.output(9U));
    TEST_ASSERT_EQUAL_INT(0, (int)d.flush());
    auto b = spi.transfers()[0].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x01U, b[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02U, b[1]);
    TEST_ASSERT_FALSE(latch.lastWrittenHigh());
    TEST_ASSERT_EQUAL_INT(0, (int)d.clear());
    TEST_ASSERT_FALSE(d.output(0U));
}
void invalid_and_bus_error_paths() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput latch;
    drivers::shift::ShiftRegister74Hc595 d(spi, latch, 1U);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument, (int)d.setOutput(8U, true));
    TEST_ASSERT_FALSE(d.output(8U));
    spi.setNextResult(io::IoResult::BusError);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError, (int)d.flush());
    TEST_ASSERT_FALSE(latch.lastWrittenHigh());
}
void output_enable_is_active_low() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput latch, oe;
    drivers::shift::ShiftRegister74Hc595 d(spi, latch, 1U, &oe);
    d.setEnabled(false);
    TEST_ASSERT_TRUE(oe.lastWrittenHigh());
    d.setEnabled(true);
    TEST_ASSERT_FALSE(oe.lastWrittenHigh());
}
} // namespace
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(buffers_flushes_and_clears);
    RUN_TEST(invalid_and_bus_error_paths);
    RUN_TEST(output_enable_is_active_low);
    return UNITY_END();
}

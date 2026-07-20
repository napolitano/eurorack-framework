#include <eurorack/drivers/dac/dac8568.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
using namespace eurorack;
namespace {
void write_frame_and_code_cache() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::dac::Dac8568 d(spi, cs);
    d.setCode(drivers::dac::Dac8568Channel::C, 0x1234U);
    TEST_ASSERT_EQUAL_HEX16(0x1234U, d.code(drivers::dac::Dac8568Channel::C));
    TEST_ASSERT_EQUAL_UINT16(0U, d.code(drivers::dac::Dac8568Channel::All));
    TEST_ASSERT_EQUAL_INT(0, (int)d.writeAndUpdate(drivers::dac::Dac8568Channel::C));
    auto b = spi.transfers()[0].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x03, b[0]);
    TEST_ASSERT_EQUAL_HEX8(0x21, b[1]);
    TEST_ASSERT_EQUAL_HEX8(0x23, b[2]);
    TEST_ASSERT_EQUAL_HEX8(0x40, b[3]);
    TEST_ASSERT_TRUE(cs.lastWrittenHigh());
}
void rejects_all_pseudo_channel() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::dac::Dac8568 d(spi, cs);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument,
                          (int)d.writeInput(drivers::dac::Dac8568Channel::All));
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument,
                          (int)d.writeAndUpdate(drivers::dac::Dac8568Channel::All));
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument,
                          (int)d.writeAndUpdateAll(drivers::dac::Dac8568Channel::All));
    TEST_ASSERT_EQUAL_UINT32(0U, spi.transfers().size());
}
void control_commands_are_encoded() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::dac::Dac8568 d(spi, cs);
    TEST_ASSERT_EQUAL_INT(
        0, (int)d.setPowerMode(0x81U, drivers::dac::Dac8568PowerMode::PowerDown100K));
    TEST_ASSERT_EQUAL_INT(0, (int)d.setInternalReference(true));
    TEST_ASSERT_EQUAL_INT(0, (int)d.setInternalReference(false));
    TEST_ASSERT_EQUAL_INT(0, (int)d.setClearCode(drivers::dac::Dac8568ClearCode::FullScale));
    TEST_ASSERT_EQUAL_INT(0, (int)d.reset());
    TEST_ASSERT_EQUAL_UINT32(5U, spi.transfers().size());
    TEST_ASSERT_EQUAL_HEX8(0x04U, spi.transfers()[0].transmitted[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02U, spi.transfers()[0].transmitted[2]);
    TEST_ASSERT_EQUAL_HEX8(0x81U, spi.transfers()[0].transmitted[3]);
}
void flush_all_stops_on_bus_error() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::dac::Dac8568 d(spi, cs);
    for (unsigned i = 0; i < 8; i++)
        d.setCode(static_cast<drivers::dac::Dac8568Channel>(i),
                  static_cast<std::uint16_t>(i * 100U));
    spi.setNextResult(io::IoResult::BusError);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError, (int)d.flushAll());
    TEST_ASSERT_EQUAL_UINT32(1U, spi.transfers().size());
    TEST_ASSERT_TRUE(cs.lastWrittenHigh());
}
} // namespace
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(write_frame_and_code_cache);
    RUN_TEST(rejects_all_pseudo_channel);
    RUN_TEST(control_commands_are_encoded);
    RUN_TEST(flush_all_stops_on_bus_error);
    return UNITY_END();
}

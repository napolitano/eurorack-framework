#include <eurorack/drivers/adc/mcp3208.hpp>
#include <eurorack/drivers/dac/mcp4822.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
namespace {
void mcp4822_frames() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput cs, ldac;
    eurorack::drivers::dac::Mcp4822 d(spi, cs, &ldac);
    d.setCode(eurorack::drivers::dac::Mcp4822Channel::A, 0x123);
    d.setGain(eurorack::drivers::dac::Mcp4822Channel::A, eurorack::drivers::dac::Mcp4822Gain::OneX);
    TEST_ASSERT_EQUAL_INT(0, (int)d.flushChannel(eurorack::drivers::dac::Mcp4822Channel::A));
    auto b = spi.transfers()[0].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x31, b[0]);
    TEST_ASSERT_EQUAL_HEX8(0x23, b[1]);
}
void mcp4822_clamps() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput cs;
    eurorack::drivers::dac::Mcp4822 d(spi, cs);
    d.setCode(eurorack::drivers::dac::Mcp4822Channel::B, 6000);
    TEST_ASSERT_EQUAL_UINT16(4095, d.code(eurorack::drivers::dac::Mcp4822Channel::B));
}
void mcp3208_command_and_decode() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput cs;
    eurorack::drivers::adc::Mcp3208 a(spi, cs);
    spi.queueResponse({0, 0x0A, 0xBC});
    std::uint16_t v = 0;
    TEST_ASSERT_EQUAL_INT(0, (int)a.read(5, v));
    TEST_ASSERT_EQUAL_UINT16(0xABC, v);
    auto t = spi.transfers()[0].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x01, t[0]);
    TEST_ASSERT_EQUAL_HEX8(0xD0, t[1]);
}
void mcp3208_differential_command() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput cs;
    eurorack::drivers::adc::Mcp3208 a(spi, cs);
    spi.queueResponse({0, 0, 1});
    std::uint16_t v = 0;
    a.read(2, v, eurorack::drivers::adc::Mcp3208InputMode::Differential);
    TEST_ASSERT_EQUAL_HEX8(0x20, spi.transfers()[0].transmitted[1]);
}
void mcp3208_bus_error_cleans_up() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput cs;
    eurorack::drivers::adc::Mcp3208 a(spi, cs);
    spi.setNextResult(eurorack::io::IoResult::BusError);
    std::uint16_t v = 0;
    TEST_ASSERT_EQUAL_INT((int)eurorack::io::IoResult::BusError, (int)a.read(0, v));
    TEST_ASSERT_TRUE(cs.lastWrittenHigh());
    TEST_ASSERT_EQUAL_INT(0, (int)spi.beginTransaction({}));
    spi.endTransaction();
}
void mcp3208_rejects_channel() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput cs;
    eurorack::drivers::adc::Mcp3208 a(spi, cs);
    std::uint16_t v = 0;
    TEST_ASSERT_EQUAL_INT((int)eurorack::io::IoResult::InvalidArgument, (int)a.read(8, v));
    TEST_ASSERT_EQUAL_UINT32(0, spi.transfers().size());
}
} // namespace
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(mcp4822_frames);
    RUN_TEST(mcp4822_clamps);
    RUN_TEST(mcp3208_command_and_decode);
    RUN_TEST(mcp3208_differential_command);
    RUN_TEST(mcp3208_bus_error_cleans_up);
    RUN_TEST(mcp3208_rejects_channel);
    return UNITY_END();
}

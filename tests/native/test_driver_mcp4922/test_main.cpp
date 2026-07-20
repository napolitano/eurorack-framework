#include <eurorack/drivers/dac/mcp4922.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
using namespace eurorack;
namespace {
void frame_configuration_and_clamp() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs, ldac;
    drivers::dac::Mcp4922Config c;
    c.referenceBuffer = drivers::dac::Mcp4922ReferenceBuffer::Buffered;
    drivers::dac::Mcp4922 d(spi, cs, &ldac, c);
    d.setCode(drivers::dac::Mcp4922Channel::A, 5000U);
    d.setGain(drivers::dac::Mcp4922Channel::A, drivers::dac::Mcp4922Gain::OneX);
    TEST_ASSERT_EQUAL_UINT16(4095U, d.code(drivers::dac::Mcp4922Channel::A));
    TEST_ASSERT_EQUAL_INT((int)drivers::dac::Mcp4922ReferenceBuffer::Buffered,
                          (int)d.referenceBuffer());
    TEST_ASSERT_EQUAL_INT(0, (int)d.flushChannel(drivers::dac::Mcp4922Channel::A));
    auto b = spi.transfers()[0].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x7FU, b[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFFU, b[1]);
}
void channel_b_shutdown_and_flush_both() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs, ldac;
    drivers::dac::Mcp4922 d(spi, cs, &ldac);
    d.setCode(drivers::dac::Mcp4922Channel::B, 0x123U);
    d.setEnabled(drivers::dac::Mcp4922Channel::B, false);
    TEST_ASSERT_EQUAL_INT(0, (int)d.flushBoth());
    TEST_ASSERT_EQUAL_UINT32(2U, spi.transfers().size());
    TEST_ASSERT_EQUAL_HEX8(0xA0U, spi.transfers()[1].transmitted[0] & 0xF0U);
    TEST_ASSERT_TRUE(ldac.transitionCount() >= 2U);
}
void bus_error_cleanup_and_reference_change() {
    simulation::VirtualSpiBus spi;
    simulation::VirtualDigitalOutput cs;
    drivers::dac::Mcp4922 d(spi, cs);
    d.setReferenceBuffer(drivers::dac::Mcp4922ReferenceBuffer::Buffered);
    TEST_ASSERT_EQUAL_INT((int)drivers::dac::Mcp4922ReferenceBuffer::Buffered,
                          (int)d.referenceBuffer());
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
    RUN_TEST(frame_configuration_and_clamp);
    RUN_TEST(channel_b_shutdown_and_flush_both);
    RUN_TEST(bus_error_cleanup_and_reference_change);
    return UNITY_END();
}

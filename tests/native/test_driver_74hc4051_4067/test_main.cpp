#include <array>
#include <eurorack/drivers/mux/analog_multiplexer.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
using namespace eurorack;
namespace {
class Delay final : public io::DelayProvider {
  public:
    void delayMicroseconds(std::uint32_t us) noexcept override {
        last = us;
        ++calls;
    }
    std::uint32_t last{0}, calls{0};
};
void selects_and_reads() {
    simulation::VirtualDigitalOutput s0, s1, s2;
    simulation::VirtualAnalogInput adc;
    adc.setCode(3072U);
    Delay delay;
    std::array<io::DigitalOutput*, 3> lines{&s0, &s1, &s2};
    drivers::mux::AnalogMultiplexer<3> m(lines, adc, delay, 5U);
    auto r = m.readChannel(5U);
    TEST_ASSERT_EQUAL_INT(0, (int)r.result);
    TEST_ASSERT_EQUAL_UINT32(3072U, r.code);
    TEST_ASSERT_EQUAL_UINT32(5U, m.currentChannel());
    TEST_ASSERT_TRUE(s0.lastWrittenHigh());
    TEST_ASSERT_FALSE(s1.lastWrittenHigh());
    TEST_ASSERT_TRUE(s2.lastWrittenHigh());
    TEST_ASSERT_EQUAL_UINT32(5U, delay.last);
}
void wraps_channel_to_select_width() {
    simulation::VirtualDigitalOutput s0, s1;
    simulation::VirtualAnalogInput adc;
    Delay delay;
    std::array<io::DigitalOutput*, 2> lines{&s0, &s1};
    drivers::mux::AnalogMultiplexer<2> m(lines, adc, delay, 0U);
    auto r = m.readChannel(7U);
    TEST_ASSERT_EQUAL_INT(0, (int)r.result);
    TEST_ASSERT_EQUAL_UINT32(3U, m.currentChannel());
    TEST_ASSERT_TRUE(s0.lastWrittenHigh());
    TEST_ASSERT_TRUE(s1.lastWrittenHigh());
}
void propagates_adc_error_and_reports_maximum() {
    simulation::VirtualDigitalOutput s0, s1, s2, s3;
    simulation::VirtualAnalogInput adc(1023U);
    adc.setResult(io::IoResult::BusError);
    Delay delay;
    std::array<io::DigitalOutput*, 4> lines{&s0, &s1, &s2, &s3};
    drivers::mux::AnalogMultiplexer<4> m(lines, adc, delay, 10U);
    auto r = m.readChannel(1U);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError, (int)r.result);
    TEST_ASSERT_EQUAL_UINT32(1023U, m.maximumCode());
    TEST_ASSERT_EQUAL_UINT32(1U, delay.calls);
}
} // namespace
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(selects_and_reads);
    RUN_TEST(wraps_channel_to_select_width);
    RUN_TEST(propagates_adc_error_and_reports_maximum);
    return UNITY_END();
}

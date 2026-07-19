#include <array>
#include <cstdint>
#include <eurorack/display/monochrome_canvas.hpp>
#include <eurorack/simulation/canvas_export.hpp>
#include <eurorack/simulation/scenario.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <eurorack/simulation/virtual_time.hpp>
#include <string>
#include <unity.h>

using namespace eurorack::simulation;

namespace {

void test_virtual_time_advances_deterministically() {
    VirtualTime time;
    time.delayMilliseconds(2U);
    time.delayMicroseconds(500U);

    TEST_ASSERT_EQUAL_UINT32(2U, time.milliseconds());
    TEST_ASSERT_EQUAL_UINT32(2500U, time.microseconds());
    TEST_ASSERT_EQUAL_UINT64(2500U, time.microseconds64());
}

void test_virtual_digital_output_counts_transitions() {
    VirtualDigitalOutput output;
    output.writeHigh(true);
    output.writeHigh(true);
    output.writeHigh(false);

    TEST_ASSERT_EQUAL_UINT32(2U, output.transitionCount());
    TEST_ASSERT_FALSE(output.lastWrittenHigh());
}

void test_virtual_analog_channels_enforce_range() {
    VirtualAnalogInput input(1023U);
    input.setCode(2000U);
    TEST_ASSERT_EQUAL_UINT32(1023U, input.readRaw().code);

    VirtualAnalogOutput output(4095U);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::InvalidArgument),
                          static_cast<int>(output.writeRaw(5000U)));
}

void test_virtual_spi_records_transfer() {
    VirtualSpiBus bus;
    bus.queueResponse({0xAAU, 0x55U});

    const eurorack::io::SpiSettings settings{};
    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                          static_cast<int>(bus.beginTransaction(settings)));

    const std::uint8_t tx[2]{0x10U, 0x20U};
    std::uint8_t rx[2]{};

    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                          static_cast<int>(bus.transfer(tx, rx, 2U)));

    TEST_ASSERT_EQUAL_HEX8(0xAAU, rx[0]);
    TEST_ASSERT_EQUAL_HEX8(0x55U, rx[1]);
    TEST_ASSERT_EQUAL_HEX8(0x10U, bus.transfers()[0].transmitted[0]);
}

void test_virtual_i2c_records_write_read() {
    VirtualI2cBus bus;
    bus.queueResponse({0x34U, 0x12U});

    const std::uint8_t reg = 0x10U;
    std::uint8_t data[2]{};

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(eurorack::io::IoResult::Success),
        static_cast<int>(bus.writeRead(eurorack::io::I2cAddress(0x20U), &reg, 1U, data, 2U)));

    TEST_ASSERT_EQUAL_HEX8(0x34U, data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x12U, data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x10U, bus.transfers()[0].written[0]);
}

void test_scenario_sorts_events() {
    Scenario scenario;
    scenario.addAnalogEvent(200U, 0U, 100U);
    scenario.addDigitalEvent(100U, 1U, true);
    scenario.sort();

    TEST_ASSERT_EQUAL_UINT64(100U, scenario.events()[0].atMicroseconds);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ScenarioEventType::DigitalInput),
                          static_cast<int>(scenario.events()[0].type));
}

void test_canvas_ascii_export() {
    std::array<std::uint8_t, 2> buffer{};
    eurorack::display::MonochromeCanvas canvas(buffer.data(), buffer.size(), 4, 2);

    canvas.applyPixel({1, 0}, eurorack::display::PixelOperation::Set);

    const std::string output = canvasToAscii(canvas, '.', '#');

    TEST_ASSERT_EQUAL_STRING(".#..\n....\n", output.c_str());
}

} // namespace

extern "C" {

void setUp() {}

void tearDown() {}

} // extern "C"

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_virtual_time_advances_deterministically);
    RUN_TEST(test_virtual_digital_output_counts_transitions);
    RUN_TEST(test_virtual_analog_channels_enforce_range);
    RUN_TEST(test_virtual_spi_records_transfer);
    RUN_TEST(test_virtual_i2c_records_write_read);
    RUN_TEST(test_scenario_sorts_events);
    RUN_TEST(test_canvas_ascii_export);

    return UNITY_END();
}

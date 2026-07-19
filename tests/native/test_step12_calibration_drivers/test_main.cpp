#include <array>
#include <cstddef>
#include <cstdint>
#include <eurorack/drivers/dac/dac8568.hpp>
#include <eurorack/drivers/display/sh1106.hpp>
#include <eurorack/io/analog_calibration.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>

namespace {

void test_two_point_calibration_round_trip() {
    const auto calibration =
        eurorack::io::LinearCodeCalibration::fromTwoPoints(65535U, 0U, -5.0F, 65535U, 5.0F);

    TEST_ASSERT_FLOAT_WITHIN(0.0002F, 0.0F, calibration.codeToVolts(32768U));

    const auto conversion = calibration.voltsToCode(2.5F);
    TEST_ASSERT_FALSE(conversion.belowRange);
    TEST_ASSERT_FALSE(conversion.aboveRange);
    TEST_ASSERT_UINT32_WITHIN(1U, 49151U, conversion.code);
}

void test_calibration_reports_clamping() {
    const eurorack::io::LinearCodeCalibration calibration(4095U, 10.0F / 4095.0F, 0.0F);

    const auto result = calibration.voltsToCode(12.0F);
    TEST_ASSERT_TRUE(result.aboveRange);
    TEST_ASSERT_EQUAL_UINT32(4095U, result.code);
}

void test_dac8568_builds_write_update_frame() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput chipSelect;
    eurorack::drivers::dac::Dac8568 dac(spi, chipSelect);

    dac.setCode(eurorack::drivers::dac::Dac8568Channel::C, 0x1234U);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(eurorack::io::IoResult::Success),
        static_cast<int>(dac.writeAndUpdate(eurorack::drivers::dac::Dac8568Channel::C)));

    const auto& bytes = spi.transfers()[0].transmitted;
    TEST_ASSERT_EQUAL_UINT32(4U, bytes.size());
    TEST_ASSERT_EQUAL_HEX8(0x03U, bytes[0]);
    TEST_ASSERT_EQUAL_HEX8(0x21U, bytes[1]);
    TEST_ASSERT_EQUAL_HEX8(0x23U, bytes[2]);
    TEST_ASSERT_EQUAL_HEX8(0x40U, bytes[3]);
}

void test_sh1106_requires_initialization() {
    eurorack::simulation::VirtualI2cBus bus;
    eurorack::drivers::display::Sh1106 display(bus);

    std::array<std::uint8_t, 1024> buffer{};
    eurorack::display::MonochromeCanvas canvas(buffer.data(), buffer.size(), 128, 64);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Busy),
                          static_cast<int>(display.flush(canvas)));
}

void test_sh1106_flush_writes_page_commands_and_data() {
    eurorack::simulation::VirtualI2cBus bus;
    eurorack::drivers::display::Sh1106 display(bus);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                          static_cast<int>(display.initialize()));

    std::array<std::uint8_t, 1024> buffer{};
    eurorack::display::MonochromeCanvas canvas(buffer.data(), buffer.size(), 128, 64);

    canvas.applyPixel({0, 0}, eurorack::display::PixelOperation::Set);

    const std::size_t before = bus.transfers().size();

    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                          static_cast<int>(display.flush(canvas)));

    const auto& pageCommand = bus.transfers()[before].written;
    TEST_ASSERT_EQUAL_HEX8(0x00U, pageCommand[0]);
    TEST_ASSERT_EQUAL_HEX8(0xB0U, pageCommand[1]);
    TEST_ASSERT_EQUAL_HEX8(0x02U, pageCommand[2]);

    const auto& firstData = bus.transfers()[before + 1U].written;
    TEST_ASSERT_EQUAL_HEX8(0x40U, firstData[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01U, firstData[1]);
}

void test_dac8568_special_command_frames() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput chipSelect;
    eurorack::drivers::dac::Dac8568 dac(spi, chipSelect);

    static_cast<void>(
        dac.setPowerMode(0xA5U, eurorack::drivers::dac::Dac8568PowerMode::PowerDownHighImpedance));
    static_cast<void>(dac.setInternalReference(true));
    static_cast<void>(dac.reset());
    static_cast<void>(dac.setClearCode(eurorack::drivers::dac::Dac8568ClearCode::FullScale));

    TEST_ASSERT_EQUAL_UINT32(4U, spi.transfers().size());

    const auto& power = spi.transfers()[0].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x04U, power[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, power[1]);
    TEST_ASSERT_EQUAL_HEX8(0x03U, power[2]);
    TEST_ASSERT_EQUAL_HEX8(0xA5U, power[3]);

    const auto& reference = spi.transfers()[1].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x08U, reference[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, reference[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, reference[2]);
    TEST_ASSERT_EQUAL_HEX8(0x01U, reference[3]);

    const auto& reset = spi.transfers()[2].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x07U, reset[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, reset[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, reset[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, reset[3]);

    const auto& clearCode = spi.transfers()[3].transmitted;
    TEST_ASSERT_EQUAL_HEX8(0x05U, clearCode[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, clearCode[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, clearCode[2]);
    TEST_ASSERT_EQUAL_HEX8(0x02U, clearCode[3]);
}

} // namespace

extern "C" {
void setUp() {}
void tearDown() {}
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_two_point_calibration_round_trip);
    RUN_TEST(test_calibration_reports_clamping);
    RUN_TEST(test_dac8568_builds_write_update_frame);
    RUN_TEST(test_dac8568_special_command_frames);
    RUN_TEST(test_sh1106_requires_initialization);
    RUN_TEST(test_sh1106_flush_writes_page_commands_and_data);
    return UNITY_END();
}

#include <array>
#include <eurorack/drivers/display/ssd1306.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <unity.h>
using namespace eurorack;
namespace {
void lifecycle_and_data_transpose() {
    simulation::VirtualI2cBus bus;
    drivers::display::Ssd1306 d(bus);
    std::array<std::uint8_t, 1024> b{};
    display::MonochromeCanvas c(b.data(), b.size(), 128, 64);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::Busy, (int)d.flush(c));
    TEST_ASSERT_EQUAL_INT(0, (int)d.initialize());
    c.applyPixel({0, 0}, display::PixelOperation::Set);
    const auto before = bus.transfers().size();
    TEST_ASSERT_EQUAL_INT(0, (int)d.flush(c));
    TEST_ASSERT_TRUE(bus.transfers().size() > before);
    bool sawData = false;
    for (size_t i = before; i < bus.transfers().size(); ++i) {
        auto& w = bus.transfers()[i].written;
        if (w.size() > 1U && w[0] == 0x40U) {
            sawData = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(sawData);
}
void control_commands_and_geometry() {
    simulation::VirtualI2cBus bus;
    drivers::display::Ssd1306 d(bus);
    TEST_ASSERT_EQUAL_UINT16(128U, d.width());
    TEST_ASSERT_EQUAL_UINT16(64U, d.height());
    TEST_ASSERT_EQUAL_INT(0, (int)d.setContrast(0x22U));
    TEST_ASSERT_EQUAL_INT(0, (int)d.setInverted(true));
    TEST_ASSERT_EQUAL_INT(0, (int)d.setInverted(false));
    TEST_ASSERT_EQUAL_INT(0, (int)d.setDisplayEnabled(false));
    TEST_ASSERT_EQUAL_INT(0, (int)d.setDisplayEnabled(true));
    TEST_ASSERT_EQUAL_HEX8(0xAFU, bus.transfers().back().written[1]);
}
void rejects_wrong_canvas_and_width() {
    simulation::VirtualI2cBus bus;
    drivers::display::Ssd1306 d(bus);
    TEST_ASSERT_EQUAL_INT(0, (int)d.initialize());
    std::array<std::uint8_t, 64> b{};
    display::MonochromeCanvas c(b.data(), b.size(), 32, 16);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument, (int)d.flush(c));
    drivers::display::Ssd1306Config cfg;
    cfg.width = 129U;
    drivers::display::Ssd1306 tooWide(bus, cfg);
    TEST_ASSERT_EQUAL_INT(0, (int)tooWide.initialize());
    std::array<std::uint8_t, 2048> big{};
    display::MonochromeCanvas bigCanvas(big.data(), big.size(), 129, 64);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument, (int)tooWide.flush(bigCanvas));
}
void initialization_and_flush_propagate_bus_errors() {
    simulation::VirtualI2cBus bus;
    drivers::display::Ssd1306 d(bus);
    bus.setNextResult(io::IoResult::BusError);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError, (int)d.initialize());
    simulation::VirtualI2cBus bus2;
    drivers::display::Ssd1306 d2(bus2);
    TEST_ASSERT_EQUAL_INT(0, (int)d2.initialize());
    std::array<std::uint8_t, 1024> b{};
    display::MonochromeCanvas c(b.data(), b.size(), 128, 64);
    bus2.setNextResult(io::IoResult::BusError);
    TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError, (int)d2.flush(c));
}
} // namespace
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(lifecycle_and_data_transpose);
    RUN_TEST(control_commands_and_geometry);
    RUN_TEST(rejects_wrong_canvas_and_width);
    RUN_TEST(initialization_and_flush_propagate_bus_errors);
    return UNITY_END();
}

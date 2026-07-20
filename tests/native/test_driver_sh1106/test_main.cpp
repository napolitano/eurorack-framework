#include <array>
#include <eurorack/drivers/display/sh1106.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <unity.h>
using namespace eurorack;
namespace {
void lifecycle_and_page_addressing(){simulation::VirtualI2cBus bus;drivers::display::Sh1106 d(bus);std::array<std::uint8_t,1024> b{};display::MonochromeCanvas c(b.data(),b.size(),128,64);TEST_ASSERT_EQUAL_INT((int)io::IoResult::Busy,(int)d.flush(c));TEST_ASSERT_EQUAL_INT(0,(int)d.initialize());c.applyPixel({0,0},display::PixelOperation::Set);const auto before=bus.transfers().size();TEST_ASSERT_EQUAL_INT(0,(int)d.flush(c));TEST_ASSERT_EQUAL_HEX8(0xB0U,bus.transfers()[before].written[1]);TEST_ASSERT_EQUAL_HEX8(0x02U,bus.transfers()[before].written[2]);}
void controls_are_encoded(){simulation::VirtualI2cBus bus;drivers::display::Sh1106 d(bus);TEST_ASSERT_EQUAL_INT(0,(int)d.setContrast(0x22U));TEST_ASSERT_EQUAL_INT(0,(int)d.setInverted(true));TEST_ASSERT_EQUAL_INT(0,(int)d.setInverted(false));TEST_ASSERT_EQUAL_INT(0,(int)d.setDisplayEnabled(false));TEST_ASSERT_EQUAL_INT(0,(int)d.setDisplayEnabled(true));TEST_ASSERT_EQUAL_HEX8(0xAFU,bus.transfers().back().written[1]);}
void invalid_geometry_is_rejected(){simulation::VirtualI2cBus bus;drivers::display::Sh1106 d(bus);TEST_ASSERT_EQUAL_INT(0,(int)d.initialize());std::array<std::uint8_t,64> b{};display::MonochromeCanvas c(b.data(),b.size(),32,16);TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument,(int)d.flush(c));drivers::display::Sh1106Config cfg;cfg.width=129U;drivers::display::Sh1106 wide(bus,cfg);TEST_ASSERT_EQUAL_INT(0,(int)wide.initialize());std::array<std::uint8_t,2048> big{};display::MonochromeCanvas bc(big.data(),big.size(),129,64);TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument,(int)wide.flush(bc));}
void errors_propagate(){simulation::VirtualI2cBus bus;drivers::display::Sh1106 d(bus);bus.setNextResult(io::IoResult::BusError);TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError,(int)d.initialize());simulation::VirtualI2cBus bus2;drivers::display::Sh1106 d2(bus2);TEST_ASSERT_EQUAL_INT(0,(int)d2.initialize());std::array<std::uint8_t,1024> b{};display::MonochromeCanvas c(b.data(),b.size(),128,64);bus2.setNextResult(io::IoResult::BusError);TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError,(int)d2.flush(c));}
}
extern "C" {void setUp(){} void tearDown(){}}
int main(){UNITY_BEGIN();RUN_TEST(lifecycle_and_page_addressing);RUN_TEST(controls_are_encoded);RUN_TEST(invalid_geometry_is_rejected);RUN_TEST(errors_propagate);return UNITY_END();}

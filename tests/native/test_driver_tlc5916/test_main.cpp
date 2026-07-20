#include <eurorack/drivers/led/tlc5916.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
using namespace eurorack;
namespace {void brightness_flush_and_clear(){simulation::VirtualSpiBus spi;simulation::VirtualDigitalOutput latch,oe;drivers::led::Tlc5916 d(spi,latch,2U,&oe);TEST_ASSERT_EQUAL_UINT32(16U,d.channelCount());TEST_ASSERT_EQUAL_INT(0,(int)d.setBrightness(2U,65535U));TEST_ASSERT_EQUAL_INT(0,(int)d.setBrightness(9U,1U));TEST_ASSERT_EQUAL_INT(0,(int)d.flush());TEST_ASSERT_EQUAL_HEX8(0x04U,spi.transfers()[0].transmitted[0]);TEST_ASSERT_EQUAL_HEX8(0x02U,spi.transfers()[0].transmitted[1]);TEST_ASSERT_EQUAL_INT(0,(int)d.clear());TEST_ASSERT_EQUAL_UINT16(0U,d.brightness(2U));}
void rejects_channel_and_reports_zero(){simulation::VirtualSpiBus spi;simulation::VirtualDigitalOutput latch;drivers::led::Tlc5916 d(spi,latch,1U);TEST_ASSERT_EQUAL_INT((int)io::IoResult::InvalidArgument,(int)d.setBrightness(8U,1U));TEST_ASSERT_EQUAL_UINT16(0U,d.brightness(8U));}
void bus_error_and_output_enable(){simulation::VirtualSpiBus spi;simulation::VirtualDigitalOutput latch,oe;drivers::led::Tlc5916 d(spi,latch,1U,&oe);d.setEnabled(false);TEST_ASSERT_TRUE(oe.lastWrittenHigh());d.setEnabled(true);TEST_ASSERT_FALSE(oe.lastWrittenHigh());spi.setNextResult(io::IoResult::BusError);TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError,(int)d.flush());TEST_ASSERT_FALSE(latch.lastWrittenHigh());}}
extern "C" {void setUp(){} void tearDown(){}}
int main(){UNITY_BEGIN();RUN_TEST(brightness_flush_and_clear);RUN_TEST(rejects_channel_and_reports_zero);RUN_TEST(bus_error_and_output_enable);return UNITY_END();}

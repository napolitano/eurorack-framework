#include <eurorack/drivers/shift/shift_register_74hc165.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <unity.h>
using namespace eurorack;
namespace {void samples_chain_and_maps_bits(){simulation::VirtualSpiBus spi;simulation::VirtualDigitalOutput load;drivers::shift::ShiftRegister74Hc165 d(spi,load,2U);spi.queueResponse({0xA5U,0x5AU});TEST_ASSERT_EQUAL_INT(0,(int)d.sample());TEST_ASSERT_EQUAL_UINT32(16U,d.inputCount());TEST_ASSERT_TRUE(load.lastWrittenHigh());TEST_ASSERT_TRUE(d.input(0U));TEST_ASSERT_FALSE(d.input(1U));TEST_ASSERT_TRUE(d.input(15U)==false);}
void invalid_input_and_zero_chain(){simulation::VirtualSpiBus spi;simulation::VirtualDigitalOutput load;drivers::shift::ShiftRegister74Hc165 d(spi,load,0U);TEST_ASSERT_EQUAL_UINT32(0U,d.inputCount());TEST_ASSERT_FALSE(d.input(0U));TEST_ASSERT_EQUAL_INT(0,(int)d.sample());}
void bus_error_keeps_load_released(){simulation::VirtualSpiBus spi;simulation::VirtualDigitalOutput load;drivers::shift::ShiftRegister74Hc165 d(spi,load,1U);spi.setNextResult(io::IoResult::BusError);TEST_ASSERT_EQUAL_INT((int)io::IoResult::BusError,(int)d.sample());TEST_ASSERT_TRUE(load.lastWrittenHigh());}}
extern "C" {void setUp(){} void tearDown(){}}
int main(){UNITY_BEGIN();RUN_TEST(samples_chain_and_maps_bits);RUN_TEST(invalid_input_and_zero_chain);RUN_TEST(bus_error_keeps_load_released);return UNITY_END();}

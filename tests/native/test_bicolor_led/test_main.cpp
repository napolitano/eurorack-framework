#include <eurorack/controls/bicolor_led.hpp>
#include <unity.h>
using namespace eurorack::controls;
void test_common_anode() {
    BiColorLed l;
    l.setColor(BiColorLedColor::Mixed);
    TEST_ASSERT_EQUAL_INT((int)PinDrive::Low, (int)l.snapshot().pinA);
    TEST_ASSERT_EQUAL_INT((int)PinDrive::Low, (int)l.snapshot().pinB);
}
void test_bipolar_mixed() {
    BiColorLed l({BiColorLedTopology::Bipolar, BiColorLedColor::Off});
    l.setColor(BiColorLedColor::Mixed);
    TEST_ASSERT_TRUE(l.snapshot().multiplexingRequired);
}
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_common_anode);
    RUN_TEST(test_bipolar_mixed);
    return UNITY_END();
}

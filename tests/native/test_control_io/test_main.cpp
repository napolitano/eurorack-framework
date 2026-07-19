#include <eurorack/controls/cv.hpp>
#include <eurorack/controls/gate.hpp>
#include <eurorack/controls/rotary_encoder.hpp>
#include <unity.h>
using namespace eurorack::controls;
void test_cv() {
    CvInput i({-5.0F, 5.0F});
    i.update(8.0F);
    TEST_ASSERT_TRUE(i.snapshot().aboveRange);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 5.0F, i.snapshot().volts);
}
void test_gate() {
    GateInput g(1.0F, 2.0F);
    g.update(2.5F);
    TEST_ASSERT_TRUE(g.snapshot().rising);
    g.update(1.5F);
    TEST_ASSERT_TRUE(g.snapshot().high);
    g.update(0.5F);
    TEST_ASSERT_TRUE(g.snapshot().falling);
}
void test_trigger() {
    TriggerOutput t(5U);
    t.trigger(10U);
    t.update(15U);
    TEST_ASSERT_TRUE(t.snapshot().ended);
}
void test_encoder() {
    RotaryEncoder e({4U, false});
    e.reset(false, false);
    e.update(false, true);
    e.update(true, true);
    e.update(true, false);
    e.update(false, false);
    TEST_ASSERT_NOT_EQUAL(0, e.snapshot().position);
}
extern "C" {
void setUp() {}
void tearDown() {}
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_cv);
    RUN_TEST(test_gate);
    RUN_TEST(test_trigger);
    RUN_TEST(test_encoder);
    return UNITY_END();
}

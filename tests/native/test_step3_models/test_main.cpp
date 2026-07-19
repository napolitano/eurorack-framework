#include <eurorack/controls/encoder_value.hpp>
#include <eurorack/controls/potentiometer.hpp>
#include <eurorack/io/jack.hpp>
#include <unity.h>

using eurorack::controls::EncoderBoundaryMode;
using eurorack::controls::EncoderValue;
using eurorack::controls::Potentiometer;
using eurorack::core::VoltageRange;
using eurorack::io::AnalogInputJack;
using eurorack::io::DigitalInputJack;
using eurorack::io::DigitalOutputJack;
using eurorack::io::JackSignalType;

namespace {

void test_potentiometer_midpoint() {
    Potentiometer potentiometer({0U, 1000U, false, 0.0F, 1.0F});
    potentiometer.reset(500U);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 0.5F, potentiometer.snapshot().normalized);
}

void test_encoder_value_wraps() {
    EncoderValue value({0, 3, 1, EncoderBoundaryMode::Wrap}, 3);
    value.applyDetents(1);
    TEST_ASSERT_EQUAL_INT32(0, value.snapshot().value);
    TEST_ASSERT_TRUE(value.snapshot().wrapped);
}

void test_analog_jack_clamps() {
    AnalogInputJack jack(JackSignalType::Cv, VoltageRange{-5.0F, 5.0F});
    jack.update(7.0F);
    TEST_ASSERT_TRUE(jack.snapshot().aboveRange);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 5.0F, jack.snapshot().effectiveVolts);
}

void test_digital_input_hysteresis() {
    DigitalInputJack jack(JackSignalType::Gate, 1.0F, 2.0F);
    jack.update(2.5F);
    TEST_ASSERT_TRUE(jack.snapshot().rising);
    jack.update(1.5F);
    TEST_ASSERT_TRUE(jack.snapshot().high);
    jack.update(0.5F);
    TEST_ASSERT_TRUE(jack.snapshot().falling);
}

void test_digital_output_voltage() {
    DigitalOutputJack jack(JackSignalType::Trigger, 0.0F, 5.0F);
    jack.setHigh(true);
    TEST_ASSERT_TRUE(jack.snapshot().rising);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 5.0F, jack.snapshot().volts);
}

} // namespace

extern "C" {

void setUp() {}

void tearDown() {}

} // extern "C"

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_potentiometer_midpoint);
    RUN_TEST(test_encoder_value_wraps);
    RUN_TEST(test_analog_jack_clamps);
    RUN_TEST(test_digital_input_hysteresis);
    RUN_TEST(test_digital_output_voltage);

    return UNITY_END();
}

/**
 * @file tests/native/test_digital_led/test_main.cpp
 * @brief Defines host-based unit tests for DigitalLed.
 *
 * @details
 * The tests validate initial state, active-high and active-low electrical mapping, state
 * transitions, one-call event flags, idempotent writes, toggle behavior, reset behavior, and
 * transition counting.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @license PolyForm Noncommercial License 1.0.0
 *
 * This file is part of Eurorack Framework. Noncommercial use, modification,
 * and redistribution are permitted under the terms in the repository-root
 * LICENSE file. The separate ADDITIONAL_PERMISSION.md file permits limited
 * five-unit, cost-recovery distribution under its stated conditions.
 * Commercial use requires prior written permission.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/controls/digital_led.hpp>
#include <unity.h>

using eurorack::controls::DigitalLed;
using eurorack::controls::DigitalLedConfig;
using eurorack::controls::LedActiveLevel;

namespace {

void test_active_high_led_starts_off() {
    const DigitalLed led;

    const auto state = led.snapshot();

    TEST_ASSERT_FALSE(state.on);
    TEST_ASSERT_FALSE(state.rawOutputHigh);
    TEST_ASSERT_FALSE(state.justTurnedOn);
    TEST_ASSERT_FALSE(state.justTurnedOff);
    TEST_ASSERT_EQUAL_UINT32(0U, state.transitionCount);
}

void test_active_low_led_starts_off_with_high_output() {
    const DigitalLed led({LedActiveLevel::Low, false});

    const auto state = led.snapshot();

    TEST_ASSERT_FALSE(state.on);
    TEST_ASSERT_TRUE(state.rawOutputHigh);
}

void test_turn_on_sets_state_output_event_and_count() {
    DigitalLed led;

    led.turnOn();
    const auto state = led.snapshot();

    TEST_ASSERT_TRUE(state.on);
    TEST_ASSERT_TRUE(state.rawOutputHigh);
    TEST_ASSERT_TRUE(state.justTurnedOn);
    TEST_ASSERT_FALSE(state.justTurnedOff);
    TEST_ASSERT_EQUAL_UINT32(1U, state.transitionCount);
}

void test_active_low_turn_on_generates_low_output() {
    DigitalLed led({LedActiveLevel::Low, false});

    led.turnOn();
    const auto state = led.snapshot();

    TEST_ASSERT_TRUE(state.on);
    TEST_ASSERT_FALSE(state.rawOutputHigh);
}

void test_repeating_same_state_is_idempotent_and_clears_event() {
    DigitalLed led;

    led.turnOn();
    led.turnOn();
    const auto state = led.snapshot();

    TEST_ASSERT_TRUE(state.on);
    TEST_ASSERT_FALSE(state.justTurnedOn);
    TEST_ASSERT_FALSE(state.justTurnedOff);
    TEST_ASSERT_EQUAL_UINT32(1U, state.transitionCount);
}

void test_turn_off_reports_one_off_transition() {
    DigitalLed led({LedActiveLevel::High, true});

    led.turnOff();
    const auto state = led.snapshot();

    TEST_ASSERT_FALSE(state.on);
    TEST_ASSERT_FALSE(state.rawOutputHigh);
    TEST_ASSERT_FALSE(state.justTurnedOn);
    TEST_ASSERT_TRUE(state.justTurnedOff);
    TEST_ASSERT_EQUAL_UINT32(1U, state.transitionCount);
}

void test_toggle_inverts_state_on_every_call() {
    DigitalLed led;

    led.toggle();
    auto state = led.snapshot();
    TEST_ASSERT_TRUE(state.on);
    TEST_ASSERT_TRUE(state.justTurnedOn);

    led.toggle();
    state = led.snapshot();
    TEST_ASSERT_FALSE(state.on);
    TEST_ASSERT_TRUE(state.justTurnedOff);
    TEST_ASSERT_EQUAL_UINT32(2U, state.transitionCount);
}

void test_reset_restores_initial_state_and_clears_history() {
    DigitalLed led({LedActiveLevel::Low, true});

    led.turnOff();
    led.reset();
    const auto state = led.snapshot();

    TEST_ASSERT_TRUE(state.on);
    TEST_ASSERT_FALSE(state.rawOutputHigh);
    TEST_ASSERT_FALSE(state.justTurnedOn);
    TEST_ASSERT_FALSE(state.justTurnedOff);
    TEST_ASSERT_EQUAL_UINT32(0U, state.transitionCount);
}

} // namespace

extern "C" {

/**
 * @brief Unity setup hook executed before every test.
 */
void setUp() {}

/**
 * @brief Unity teardown hook executed after every test.
 */
void tearDown() {}

} // extern "C"

/**
 * @brief Executes this native Unity test suite.
 *
 * @return Unity process exit code. Zero indicates that every test passed.
 */
int main() {
    UNITY_BEGIN();

    RUN_TEST(test_active_high_led_starts_off);
    RUN_TEST(test_active_low_led_starts_off_with_high_output);
    RUN_TEST(test_turn_on_sets_state_output_event_and_count);
    RUN_TEST(test_active_low_turn_on_generates_low_output);
    RUN_TEST(test_repeating_same_state_is_idempotent_and_clears_event);
    RUN_TEST(test_turn_off_reports_one_off_transition);
    RUN_TEST(test_toggle_inverts_state_on_every_call);
    RUN_TEST(test_reset_restores_initial_state_and_clears_history);

    return UNITY_END();
}

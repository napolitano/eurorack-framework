/**
 * @file tests/native/test_momentary_button/test_main.cpp
 * @brief Defines host-based unit tests for MomentaryButton.
 *
 * @details
 * The tests validate initialization, active-low and active-high polarity, time-based debounce,
 * contact bounce rejection, edge lifetime, hold duration, zero-debounce operation, transition
 * counts, and 32-bit timer wraparound without requiring target hardware.
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

#include <cstdint>
#include <eurorack/controls/momentary_button.hpp>
#include <limits>
#include <unity.h>

using eurorack::controls::ActiveLevel;
using eurorack::controls::MomentaryButton;
using eurorack::controls::MomentaryButtonConfig;

namespace {

void test_first_update_initializes_without_synthetic_event() {
    MomentaryButton button;

    button.update(true, 100U);
    const auto state = button.snapshot();

    TEST_ASSERT_FALSE(state.pressed);
    TEST_ASSERT_FALSE(state.justPressed);
    TEST_ASSERT_FALSE(state.justReleased);
    TEST_ASSERT_EQUAL_UINT32(0U, state.transitionCount);
}

void test_active_low_press_is_accepted_after_debounce_time() {
    MomentaryButton button({ActiveLevel::Low, 20U});
    button.reset(true, 0U);

    button.update(false, 5U);
    auto state = button.snapshot();
    TEST_ASSERT_FALSE(state.pressed);

    button.update(false, 24U);
    state = button.snapshot();
    TEST_ASSERT_FALSE(state.pressed);

    button.update(false, 25U);
    state = button.snapshot();
    TEST_ASSERT_TRUE(state.pressed);
    TEST_ASSERT_TRUE(state.justPressed);
    TEST_ASSERT_FALSE(state.justReleased);
    TEST_ASSERT_EQUAL_UINT32(1U, state.transitionCount);
}

void test_contact_bounce_does_not_generate_false_transition() {
    MomentaryButton button({ActiveLevel::Low, 10U});
    button.reset(true, 0U);

    button.update(false, 1U);
    button.update(true, 3U);
    button.update(false, 5U);
    button.update(true, 8U);

    button.update(true, 30U);
    auto state = button.snapshot();
    TEST_ASSERT_FALSE(state.pressed);
    TEST_ASSERT_FALSE(state.justPressed);
    TEST_ASSERT_FALSE(state.justReleased);
    TEST_ASSERT_EQUAL_UINT32(0U, state.transitionCount);
}

void test_release_is_debounced_and_reported_once() {
    MomentaryButton button({ActiveLevel::Low, 5U});
    button.reset(true, 0U);

    button.update(false, 1U);
    button.update(false, 6U);
    auto state = button.snapshot();
    TEST_ASSERT_TRUE(state.justPressed);

    button.update(true, 7U);
    state = button.snapshot();
    TEST_ASSERT_TRUE(state.pressed);
    TEST_ASSERT_FALSE(state.justReleased);

    button.update(true, 12U);
    state = button.snapshot();
    TEST_ASSERT_FALSE(state.pressed);
    TEST_ASSERT_TRUE(state.justReleased);
    TEST_ASSERT_EQUAL_UINT32(6U, state.lastPressDurationMs);
    TEST_ASSERT_EQUAL_UINT32(2U, state.transitionCount);

    button.update(true, 13U);
    state = button.snapshot();
    TEST_ASSERT_FALSE(state.justReleased);
    TEST_ASSERT_EQUAL_UINT32(2U, state.transitionCount);
}

void test_active_high_configuration() {
    MomentaryButton button({ActiveLevel::High, 3U});
    button.reset(false, 0U);

    button.update(true, 1U);
    button.update(true, 4U);
    const auto state = button.snapshot();

    TEST_ASSERT_TRUE(state.pressed);
    TEST_ASSERT_TRUE(state.justPressed);
}

void test_held_duration_starts_at_accepted_press() {
    MomentaryButton button({ActiveLevel::Low, 10U});
    button.reset(true, 0U);

    button.update(false, 50U);
    button.update(false, 60U);
    auto state = button.snapshot();
    TEST_ASSERT_TRUE(state.justPressed);
    TEST_ASSERT_EQUAL_UINT32(0U, state.heldForMs);

    button.update(false, 85U);
    state = button.snapshot();
    TEST_ASSERT_TRUE(state.pressed);
    TEST_ASSERT_FALSE(state.justPressed);
    TEST_ASSERT_EQUAL_UINT32(25U, state.heldForMs);
}

void test_zero_debounce_accepts_transition_immediately() {
    MomentaryButton button({ActiveLevel::Low, 0U});
    button.reset(true, 0U);

    button.update(false, 1U);
    const auto state = button.snapshot();

    TEST_ASSERT_TRUE(state.pressed);
    TEST_ASSERT_TRUE(state.justPressed);
}

void test_timer_wraparound_is_handled_by_unsigned_arithmetic() {
    constexpr std::uint32_t nearWrap = std::numeric_limits<std::uint32_t>::max() - 4U;

    MomentaryButton button({ActiveLevel::Low, 5U});
    button.reset(true, nearWrap);
    button.update(false, nearWrap);

    button.update(false, 0U);
    const auto state = button.snapshot();

    TEST_ASSERT_TRUE(state.pressed);
    TEST_ASSERT_TRUE(state.justPressed);
}

void test_reset_clears_events_duration_and_transition_count() {
    MomentaryButton button({ActiveLevel::Low, 0U});
    button.reset(true, 0U);
    button.update(false, 1U);
    button.update(false, 9U);

    button.reset(true, 10U);
    const auto state = button.snapshot();

    TEST_ASSERT_FALSE(state.pressed);
    TEST_ASSERT_FALSE(state.justPressed);
    TEST_ASSERT_FALSE(state.justReleased);
    TEST_ASSERT_EQUAL_UINT32(0U, state.heldForMs);
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

    RUN_TEST(test_first_update_initializes_without_synthetic_event);
    RUN_TEST(test_active_low_press_is_accepted_after_debounce_time);
    RUN_TEST(test_contact_bounce_does_not_generate_false_transition);
    RUN_TEST(test_release_is_debounced_and_reported_once);
    RUN_TEST(test_active_high_configuration);
    RUN_TEST(test_held_duration_starts_at_accepted_press);
    RUN_TEST(test_zero_debounce_accepts_transition_immediately);
    RUN_TEST(test_timer_wraparound_is_handled_by_unsigned_arithmetic);
    RUN_TEST(test_reset_clears_events_duration_and_transition_count);

    return UNITY_END();
}

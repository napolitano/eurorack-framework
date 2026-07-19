/**
 * @file tests/native/test_framework_config/test_main.cpp
 * @brief Defines host-based tests for the central framework configuration.
 *
 * @details
 * The tests verify Eurorack defaults, voltage-range helpers, complete configuration validation,
 * project overrides, and consumption of the central button debounce setting.
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

#include <eurorack/controls/momentary_button.hpp>
#include <eurorack/core/framework_config.hpp>
#include <unity.h>

using eurorack::controls::ActiveLevel;
using eurorack::controls::MomentaryButtonConfig;
using eurorack::core::eurorackDefaults;
using eurorack::core::FrameworkConfig;
using eurorack::core::isValid;
using eurorack::core::VoltageRange;

namespace {

void test_default_framework_configuration_is_valid() {
    TEST_ASSERT_TRUE(isValid(eurorackDefaults));
}

void test_default_cv_ranges_match_documented_eurorack_limits() {
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, -10.0F, eurorackDefaults.cv.inputVolts.minimumVolts);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 10.0F, eurorackDefaults.cv.inputVolts.maximumVolts);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, -5.0F, eurorackDefaults.cv.outputVolts.minimumVolts);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 5.0F, eurorackDefaults.cv.outputVolts.maximumVolts);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 1.0F, eurorackDefaults.cv.voltsPerOctave);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, -5.0F, eurorackDefaults.cv.audioVolts.minimumVolts);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 5.0F, eurorackDefaults.cv.audioVolts.maximumVolts);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, -5.0F, eurorackDefaults.cv.pitchInputVolts.minimumVolts);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 10.0F, eurorackDefaults.cv.pitchOutputVolts.maximumVolts);
}

void test_default_gate_thresholds_have_defined_dead_band() {
    TEST_ASSERT_TRUE(eurorackDefaults.gate.inputLowMaximumVolts <
                     eurorackDefaults.gate.inputHighMinimumVolts);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 5.0F, eurorackDefaults.gate.outputHighVolts);
}

void test_voltage_range_contains_and_clamps_values() {
    constexpr VoltageRange range{-5.0F, 5.0F};

    TEST_ASSERT_TRUE(range.contains(-5.0F));
    TEST_ASSERT_TRUE(range.contains(0.0F));
    TEST_ASSERT_TRUE(range.contains(5.0F));
    TEST_ASSERT_FALSE(range.contains(5.1F));
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, -5.0F, range.clamp(-12.0F));
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 3.0F, range.clamp(3.0F));
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 5.0F, range.clamp(12.0F));
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 10.0F, range.spanVolts());
}

void test_invalid_voltage_range_rejects_configuration() {
    auto config = eurorackDefaults;
    config.cv.inputVolts = {10.0F, -10.0F};

    TEST_ASSERT_FALSE(isValid(config));
}

void test_invalid_gate_threshold_order_is_rejected() {
    auto config = eurorackDefaults;
    config.gate.inputLowMaximumVolts = 3.0F;
    config.gate.inputHighMinimumVolts = 2.0F;

    TEST_ASSERT_FALSE(isValid(config));
}

void test_consuming_project_can_override_selected_defaults() {
    auto config = eurorackDefaults;
    config.cv.outputVolts = {-5.0F, 5.0F};
    config.interaction.buttonDebounceMs = 12U;
    config.runtime.uiRefreshHz = 30U;

    TEST_ASSERT_TRUE(isValid(config));
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, -5.0F, config.cv.outputVolts.minimumVolts);
    TEST_ASSERT_EQUAL_UINT32(12U, config.interaction.buttonDebounceMs);
    TEST_ASSERT_EQUAL_UINT32(30U, config.runtime.uiRefreshHz);
}

void test_button_configuration_consumes_central_debounce_default() {
    auto config = eurorackDefaults;
    config.interaction.buttonDebounceMs = 17U;

    constexpr ActiveLevel level = ActiveLevel::High;
    const auto buttonConfig = MomentaryButtonConfig::fromFramework(config, level);

    TEST_ASSERT_EQUAL_UINT32(17U, buttonConfig.debounceTimeMs);
    TEST_ASSERT_TRUE(buttonConfig.activeLevel == ActiveLevel::High);
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
 * @brief Executes the native framework-configuration test suite.
 *
 * @return Unity process exit code. Zero indicates that every test passed.
 */
int main() {
    UNITY_BEGIN();

    RUN_TEST(test_default_framework_configuration_is_valid);
    RUN_TEST(test_default_cv_ranges_match_documented_eurorack_limits);
    RUN_TEST(test_default_gate_thresholds_have_defined_dead_band);
    RUN_TEST(test_voltage_range_contains_and_clamps_values);
    RUN_TEST(test_invalid_voltage_range_rejects_configuration);
    RUN_TEST(test_invalid_gate_threshold_order_is_rejected);
    RUN_TEST(test_consuming_project_can_override_selected_defaults);
    RUN_TEST(test_button_configuration_consumes_central_debounce_default);

    return UNITY_END();
}

#include <array>
#include <cstdint>
#include <unity.h>

#include <eurorack/controls/dip_switch.hpp>
#include <eurorack/controls/fader.hpp>
#include <eurorack/controls/illuminated_button.hpp>
#include <eurorack/controls/illuminated_fader.hpp>
#include <eurorack/controls/multicolor_led.hpp>
#include <eurorack/controls/on_off_momentary_switch.hpp>
#include <eurorack/controls/toggle_switch.hpp>
#include <eurorack/simulation/virtual_buses.hpp>

namespace {

class TestLedChannel final : public eurorack::drivers::led::LedChannel {
public:
    void setBrightness(const std::uint16_t brightness) noexcept override { value = brightness; }
    [[nodiscard]] std::uint16_t brightness() const noexcept override { return value; }
    std::uint16_t value{0U};
};

void test_toggle_switch_debounces_on_position() {
    eurorack::controls::ToggleSwitch sw({eurorack::controls::ActiveLevel::High, 5U});
    sw.reset(false, 0U);
    sw.update(true, 1U);
    TEST_ASSERT_FALSE(sw.isOn());
    sw.update(true, 6U);
    TEST_ASSERT_TRUE(sw.isOn());
    TEST_ASSERT_TRUE(sw.snapshot().justTurnedOn);
}

void test_dip_bank_builds_mask() {
    eurorack::controls::DipSwitchBank<4U> bank({eurorack::controls::ActiveLevel::High, 0U});
    bank.reset({false, true, false, true}, 0U);
    TEST_ASSERT_EQUAL_HEX32(0x0AU, bank.bitMask());
}

void test_on_off_momentary_switch_reports_positions_and_invalid() {
    eurorack::controls::OnOffMomentarySwitch sw({
        eurorack::controls::ActiveLevel::High,
        eurorack::controls::ActiveLevel::High,
        0U});
    sw.reset(false, false, 0U);
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(eurorack::controls::OnOffMomentaryPosition::Off),
        static_cast<int>(sw.position()));
    sw.update(true, false, 1U);
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(eurorack::controls::OnOffMomentaryPosition::On),
        static_cast<int>(sw.position()));
    sw.update(false, true, 2U);
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(eurorack::controls::OnOffMomentaryPosition::MomentaryOn),
        static_cast<int>(sw.position()));
    sw.update(true, true, 3U);
    TEST_ASSERT_TRUE(sw.snapshot().invalidCombination);
}

void test_multicolor_led_scales_and_applies_channels() {
    eurorack::controls::MulticolorLed led({65535U, 32768U, 0U}, 32768U);
    TEST_ASSERT_EQUAL_UINT16(32768U, led.snapshot().effective.red);
    TEST_ASSERT_UINT32_WITHIN(1U, 16384U, led.snapshot().effective.green);

    TestLedChannel red;
    TestLedChannel green;
    TestLedChannel blue;
    led.applyTo(red, green, blue);
    TEST_ASSERT_EQUAL_UINT16(led.snapshot().effective.red, red.value);
    TEST_ASSERT_EQUAL_UINT16(led.snapshot().effective.green, green.value);
    TEST_ASSERT_EQUAL_UINT16(0U, blue.value);
}

void test_illuminated_button_follows_pressed_state() {
    eurorack::controls::IlluminatedButton button({
        {eurorack::controls::ActiveLevel::High, 0U},
        eurorack::controls::IlluminatedButtonMode::LitWhilePressed,
        {65535U, 0U, 0U},
        {0U, 0U, 0U}});
    button.reset(false, 0U);
    TEST_ASSERT_EQUAL_UINT16(0U, button.led().snapshot().effective.red);
    button.update(true, 1U);
    TEST_ASSERT_EQUAL_UINT16(65535U, button.led().snapshot().effective.red);
}

void test_fader_direction_and_normalization() {
    eurorack::controls::Fader fader({
        100U, 1100U,
        eurorack::controls::FaderDirection::TopToBottom,
        0.0F, 1.0F});
    fader.reset(100U);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 1.0F, fader.snapshot().normalized);
    fader.update(1100U);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 0.0F, fader.snapshot().normalized);
}

void test_illuminated_fader_brightness_follows_position() {
    eurorack::controls::IlluminatedFader fader({
        {0U, 1000U, eurorack::controls::FaderDirection::BottomToTop, 0.0F, 1.0F},
        eurorack::controls::IlluminatedFaderMode::FollowPosition,
        {0U, 0U, 65535U},
        1000U,
        5000U});
    fader.reset(500U);
    TEST_ASSERT_UINT32_WITHIN(1U, 3000U, fader.led().snapshot().masterBrightness);
}

} // namespace

extern "C" {
void setUp() {}
void tearDown() {}
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_toggle_switch_debounces_on_position);
    RUN_TEST(test_dip_bank_builds_mask);
    RUN_TEST(test_on_off_momentary_switch_reports_positions_and_invalid);
    RUN_TEST(test_multicolor_led_scales_and_applies_channels);
    RUN_TEST(test_illuminated_button_follows_pressed_state);
    RUN_TEST(test_fader_direction_and_normalization);
    RUN_TEST(test_illuminated_fader_brightness_follows_position);
    return UNITY_END();
}

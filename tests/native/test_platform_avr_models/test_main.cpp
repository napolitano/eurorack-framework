#include <cstdint>
#include <eurorack/platform/avr/median_of_three_scanner.hpp>
#include <eurorack/platform/avr/saturating_tick_counter.hpp>
#include <eurorack/platform/avr/sticky_overflow_edge_counter.hpp>
#include <unity.h>

namespace {

void tick_counter_starts_at_zero() {
    eurorack::platform::avr::SaturatingTickCounter counter;
    TEST_ASSERT_EQUAL_UINT8(0U, counter.consumePending());
    TEST_ASSERT_EQUAL_UINT16(0U, counter.overrunCount());
}

void tick_counter_accumulates_and_resets_on_consume() {
    eurorack::platform::avr::SaturatingTickCounter counter;
    counter.onTick();
    counter.onTick();
    counter.onTick();
    TEST_ASSERT_EQUAL_UINT8(3U, counter.consumePending());
    TEST_ASSERT_EQUAL_UINT8(0U, counter.consumePending());
}

void tick_counter_saturates_pending_without_wrapping() {
    eurorack::platform::avr::SaturatingTickCounter counter;
    for (int i = 0; i < 300; ++i) {
        counter.onTick();
    }
    TEST_ASSERT_EQUAL_UINT8(255U, counter.consumePending());
}

void tick_counter_records_overruns_once_pending_saturates() {
    eurorack::platform::avr::SaturatingTickCounter counter;
    for (int i = 0; i < 255; ++i) {
        counter.onTick();
    }
    TEST_ASSERT_EQUAL_UINT16(0U, counter.overrunCount());
    counter.onTick();
    counter.onTick();
    TEST_ASSERT_EQUAL_UINT16(2U, counter.overrunCount());
    TEST_ASSERT_EQUAL_UINT8(255U, counter.consumePending());
}

void tick_counter_overrun_count_survives_consume_pending() {
    eurorack::platform::avr::SaturatingTickCounter counter;
    for (int i = 0; i < 256; ++i) {
        counter.onTick();
    }
    counter.consumePending();
    TEST_ASSERT_EQUAL_UINT16(1U, counter.overrunCount());
}

void tick_counter_reset_clears_both_counts() {
    eurorack::platform::avr::SaturatingTickCounter counter;
    for (int i = 0; i < 256; ++i) {
        counter.onTick();
    }
    counter.reset();
    TEST_ASSERT_EQUAL_UINT8(0U, counter.consumePending());
    TEST_ASSERT_EQUAL_UINT16(0U, counter.overrunCount());
}

void edge_counter_starts_at_zero_not_overflowed() {
    eurorack::platform::avr::StickyOverflowEdgeCounter counter;
    TEST_ASSERT_EQUAL_UINT8(0U, counter.consume());
    TEST_ASSERT_FALSE(counter.overflowed());
}

void edge_counter_accumulates_and_resets_on_consume() {
    eurorack::platform::avr::StickyOverflowEdgeCounter counter;
    counter.onEdge();
    counter.onEdge();
    TEST_ASSERT_EQUAL_UINT8(2U, counter.consume());
    TEST_ASSERT_EQUAL_UINT8(0U, counter.consume());
}

void edge_counter_saturates_without_wrapping() {
    eurorack::platform::avr::StickyOverflowEdgeCounter counter;
    for (int i = 0; i < 300; ++i) {
        counter.onEdge();
    }
    TEST_ASSERT_EQUAL_UINT8(255U, counter.consume());
}

void edge_counter_overflow_flag_is_sticky_across_consume() {
    eurorack::platform::avr::StickyOverflowEdgeCounter counter;
    for (int i = 0; i < 256; ++i) {
        counter.onEdge();
    }
    TEST_ASSERT_TRUE(counter.overflowed());
    counter.consume();
    TEST_ASSERT_TRUE(counter.overflowed());
}

void edge_counter_reset_clears_count_and_overflow() {
    eurorack::platform::avr::StickyOverflowEdgeCounter counter;
    for (int i = 0; i < 256; ++i) {
        counter.onEdge();
    }
    counter.reset();
    TEST_ASSERT_EQUAL_UINT8(0U, counter.consume());
    TEST_ASSERT_FALSE(counter.overflowed());
}

void scanner_discards_first_sample_after_construction() {
    eurorack::platform::avr::MedianOfThreeScanner scanner;
    const auto event = scanner.recordSample(100U);
    TEST_ASSERT_FALSE(event.changed);
    TEST_ASSERT_FALSE(scanner.snapshot().ready);
}

void scanner_reports_channel_switch_after_third_sample() {
    eurorack::platform::avr::MedianOfThreeScanner scanner;
    scanner.recordSample(999U); // discarded
    scanner.recordSample(200U);
    scanner.recordSample(300U);
    const auto event = scanner.recordSample(400U);
    TEST_ASSERT_TRUE(event.changed);
    TEST_ASSERT_EQUAL_UINT8(1U, event.channel);
    TEST_ASSERT_EQUAL_UINT16(300U, scanner.snapshot().channel[0]);
    TEST_ASSERT_FALSE(scanner.snapshot().ready);
}

void scanner_becomes_ready_and_wraps_after_full_rotation() {
    eurorack::platform::avr::MedianOfThreeScanner scanner;
    // Channel 0: discard, then 200/300/400 -> median 300, switches to channel 1.
    scanner.recordSample(999U);
    scanner.recordSample(200U);
    scanner.recordSample(300U);
    scanner.recordSample(400U);
    // Channel 1: discard, then 10/20/30 -> median 20, switches to channel 2.
    scanner.recordSample(999U);
    scanner.recordSample(10U);
    scanner.recordSample(20U);
    scanner.recordSample(30U);
    // Channel 2: discard, then 1/2/3 -> median 2, wraps back to channel 0 and becomes ready.
    scanner.recordSample(999U);
    scanner.recordSample(1U);
    scanner.recordSample(2U);
    const auto event = scanner.recordSample(3U);

    TEST_ASSERT_TRUE(event.changed);
    TEST_ASSERT_EQUAL_UINT8(0U, event.channel);

    const auto snap = scanner.snapshot();
    TEST_ASSERT_TRUE(snap.ready);
    TEST_ASSERT_EQUAL_UINT16(300U, snap.channel[0]);
    TEST_ASSERT_EQUAL_UINT16(20U, snap.channel[1]);
    TEST_ASSERT_EQUAL_UINT16(2U, snap.channel[2]);
}

void scanner_reset_returns_to_discard_first_state() {
    eurorack::platform::avr::MedianOfThreeScanner scanner;
    scanner.recordSample(999U);
    scanner.recordSample(200U);
    scanner.recordSample(300U);
    scanner.recordSample(400U);
    scanner.reset();

    const auto event = scanner.recordSample(555U);
    TEST_ASSERT_FALSE(event.changed);
    TEST_ASSERT_FALSE(scanner.snapshot().ready);
    TEST_ASSERT_EQUAL_UINT16(0U, scanner.snapshot().channel[0]);
}

} // namespace

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(tick_counter_starts_at_zero);
    RUN_TEST(tick_counter_accumulates_and_resets_on_consume);
    RUN_TEST(tick_counter_saturates_pending_without_wrapping);
    RUN_TEST(tick_counter_records_overruns_once_pending_saturates);
    RUN_TEST(tick_counter_overrun_count_survives_consume_pending);
    RUN_TEST(tick_counter_reset_clears_both_counts);
    RUN_TEST(edge_counter_starts_at_zero_not_overflowed);
    RUN_TEST(edge_counter_accumulates_and_resets_on_consume);
    RUN_TEST(edge_counter_saturates_without_wrapping);
    RUN_TEST(edge_counter_overflow_flag_is_sticky_across_consume);
    RUN_TEST(edge_counter_reset_clears_count_and_overflow);
    RUN_TEST(scanner_discards_first_sample_after_construction);
    RUN_TEST(scanner_reports_channel_switch_after_third_sample);
    RUN_TEST(scanner_becomes_ready_and_wraps_after_full_rotation);
    RUN_TEST(scanner_reset_returns_to_discard_first_state);
    return UNITY_END();
}

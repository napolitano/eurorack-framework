#include <array>
#include <cstdint>
#include <eurorack/display/widgets.hpp>
#include <unity.h>

using namespace eurorack::display;

namespace {

constexpr std::int16_t WIDTH = 128;
constexpr std::int16_t HEIGHT = 64;
constexpr std::size_t BUFFER_SIZE = MonochromeCanvas::requiredBufferSize(WIDTH, HEIGHT);

std::array<std::uint8_t, BUFFER_SIZE> buffer{};
MonochromeCanvas canvas(buffer.data(), buffer.size(), WIDTH, HEIGHT);

void resetCanvas() {
    canvas.resetClipRect();
    canvas.clear();
}

void test_visible_menu_row_count() {
    TEST_ASSERT_EQUAL_UINT32(5U, visibleMenuRowCount({0, 0, 100, 45}, 9U));
}

void test_selection_visibility_scrolls_down() {
    TEST_ASSERT_EQUAL_UINT32(3U, ensureMenuSelectionVisible(7U, 0U, 10U, 5U));
}

void test_progress_bar_fills_expected_width() {
    resetCanvas();
    drawProgressBar(canvas, {0, 0, 10, 4}, {0.5F, false, false});

    TEST_ASSERT_TRUE(canvas.pixel({0, 0}));
    TEST_ASSERT_TRUE(canvas.pixel({4, 3}));
    TEST_ASSERT_FALSE(canvas.pixel({5, 0}));
}

void test_top_bar_draws_inverted_background() {
    resetCanvas();
    drawTopBar(canvas, {0, 0, 128, 9}, {"SEION", "LIVE", true});

    TEST_ASSERT_TRUE(canvas.pixel({127, 8}));
}

void test_selected_menu_row_is_inverted() {
    resetCanvas();

    const MenuItemView items[] = {
        {"One", "", true},
        {"Two", "", true},
        {"Three", "", true},
    };

    drawMenuList(canvas, {0, 0, 64, 27}, {items, 3U, 1U, 0U, 9U, false});

    TEST_ASSERT_TRUE(canvas.pixel({63, 9}));
    TEST_ASSERT_FALSE(canvas.pixel({63, 0}));
}

void test_confirmation_overlay_draws_selected_cancel_button() {
    resetCanvas();

    drawConfirmationOverlay(canvas,
                            {10, 8, 100, 48},
                            {"Delete?", "Remove stored calibration?", ConfirmationChoice::Cancel});

    TEST_ASSERT_TRUE(canvas.pixel({107, 53}));
    TEST_ASSERT_TRUE(canvas.pixel({10, 8}));
}

void test_error_overlay_draws_frame_and_action() {
    resetCanvas();

    drawErrorOverlay(
        canvas, {10, 8, 100, 48}, {"I2C Error", "Display did not acknowledge.", "Retry"});

    TEST_ASSERT_TRUE(canvas.pixel({10, 8}));
    TEST_ASSERT_TRUE(canvas.pixel({109, 55}));
}

} // namespace

extern "C" {

void setUp() {}

void tearDown() {}

} // extern "C"

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_visible_menu_row_count);
    RUN_TEST(test_selection_visibility_scrolls_down);
    RUN_TEST(test_progress_bar_fills_expected_width);
    RUN_TEST(test_top_bar_draws_inverted_background);
    RUN_TEST(test_selected_menu_row_is_inverted);
    RUN_TEST(test_confirmation_overlay_draws_selected_cancel_button);
    RUN_TEST(test_error_overlay_draws_frame_and_action);

    return UNITY_END();
}

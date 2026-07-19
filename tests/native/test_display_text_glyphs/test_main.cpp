#include <array>
#include <cstdint>
#include <eurorack/display/glyphs.hpp>
#include <eurorack/display/text.hpp>
#include <unity.h>

using namespace eurorack::display;

namespace {

constexpr std::int16_t WIDTH = 32;
constexpr std::int16_t HEIGHT = 16;
constexpr std::size_t BUFFER_SIZE = MonochromeCanvas::requiredBufferSize(WIDTH, HEIGHT);

std::array<std::uint8_t, BUFFER_SIZE> buffer{};
MonochromeCanvas canvas(buffer.data(), buffer.size(), WIDTH, HEIGHT);

void resetCanvas() {
    canvas.resetClipRect();
    canvas.clear();
}

void test_measure_text_uses_spacing() {
    const auto metrics = measureText("ABC");
    TEST_ASSERT_EQUAL_INT16(17, metrics.width);
    TEST_ASSERT_EQUAL_INT16(7, metrics.height);
}

void test_draw_character_sets_known_pixels() {
    resetCanvas();
    drawCharacter(canvas, {0, 0}, 'A');

    TEST_ASSERT_TRUE(canvas.pixel({0, 1}));
    TEST_ASSERT_TRUE(canvas.pixel({2, 0}));
    TEST_ASSERT_TRUE(canvas.pixel({4, 1}));
    TEST_ASSERT_TRUE(canvas.pixel({2, 4}));
}

void test_centered_text_is_positioned_inside_bounds() {
    resetCanvas();

    TextStyle style{};
    style.horizontalAlignment = HorizontalAlignment::Center;
    style.verticalAlignment = VerticalAlignment::Middle;

    drawText(canvas, {0, 0, 20, 9}, "A", style);

    TEST_ASSERT_TRUE(canvas.pixel({7, 2}));
    TEST_ASSERT_TRUE(canvas.pixel({9, 1}));
}

void test_inverted_text_fills_background_and_clears_glyph() {
    resetCanvas();

    TextStyle style{};
    style.mode = TextMode::Inverted;

    drawText(canvas, {0, 0, 8, 8}, "A", style);

    TEST_ASSERT_TRUE(canvas.pixel({7, 7}));
    TEST_ASSERT_FALSE(canvas.pixel({2, 0}));
    TEST_ASSERT_TRUE(canvas.pixel({5, 0}));
}

void test_text_clips_to_bounds() {
    resetCanvas();

    TextStyle style{};
    style.clipToBounds = true;

    drawText(canvas, {0, 0, 3, 7}, "A", style);

    TEST_ASSERT_FALSE(canvas.pixel({4, 1}));
}

void test_left_arrow_has_center_and_tip() {
    resetCanvas();
    drawGlyph(canvas, {2, 2, 7, 7}, BasicGlyph::ArrowLeft);

    TEST_ASSERT_TRUE(canvas.pixel({2, 5}));
    TEST_ASSERT_TRUE(canvas.pixel({8, 5}));
    TEST_ASSERT_TRUE(canvas.pixel({5, 2}));
    TEST_ASSERT_TRUE(canvas.pixel({5, 8}));
}

void test_plus_glyph_draws_both_axes() {
    resetCanvas();
    drawGlyph(canvas, {2, 2, 7, 7}, BasicGlyph::Plus);

    TEST_ASSERT_TRUE(canvas.pixel({5, 2}));
    TEST_ASSERT_TRUE(canvas.pixel({2, 5}));
    TEST_ASSERT_TRUE(canvas.pixel({8, 5}));
    TEST_ASSERT_TRUE(canvas.pixel({5, 8}));
}

} // namespace

extern "C" {

void setUp() {}

void tearDown() {}

} // extern "C"

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_measure_text_uses_spacing);
    RUN_TEST(test_draw_character_sets_known_pixels);
    RUN_TEST(test_centered_text_is_positioned_inside_bounds);
    RUN_TEST(test_inverted_text_fills_background_and_clears_glyph);
    RUN_TEST(test_text_clips_to_bounds);
    RUN_TEST(test_left_arrow_has_center_and_tip);
    RUN_TEST(test_plus_glyph_draws_both_axes);

    return UNITY_END();
}

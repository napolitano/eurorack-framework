#include <array>
#include <cstddef>
#include <cstdint>
#include <eurorack/display/drawing.hpp>
#include <unity.h>

using namespace eurorack::display;

namespace {

std::size_t countPixels(const MonochromeCanvas& canvas) {
    std::size_t count = 0U;
    for (std::int32_t y = 0; y < canvas.height(); ++y) {
        for (std::int32_t x = 0; x < canvas.width(); ++x) {
            if (canvas.pixel({x, y})) {
                ++count;
            }
        }
    }
    return count;
}

void test_canvas_packs_pixels_msb_first() {
    std::array<std::uint8_t, 2> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 9, 1);

    TEST_ASSERT_TRUE(canvas.applyPixel({0, 0}, PixelOperation::Set));
    TEST_ASSERT_TRUE(canvas.applyPixel({8, 0}, PixelOperation::Set));

    TEST_ASSERT_EQUAL_HEX8(0x80U, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0x80U, buffer[1]);
}

void test_clip_rejects_outside_pixel() {
    std::array<std::uint8_t, 8> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 8, 8);
    canvas.setClipRect({2, 2, 3, 3});

    TEST_ASSERT_FALSE(canvas.applyPixel({1, 1}, PixelOperation::Set));
    TEST_ASSERT_TRUE(canvas.applyPixel({2, 2}, PixelOperation::Set));
    TEST_ASSERT_EQUAL_UINT32(1U, countPixels(canvas));
}

void test_horizontal_and_vertical_lines() {
    std::array<std::uint8_t, 8> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 8, 8);

    drawHorizontalLine(canvas, 1, 2, 4);
    drawVerticalLine(canvas, 6, 1, 5);

    TEST_ASSERT_EQUAL_UINT32(9U, countPixels(canvas));
}

void test_diagonal_line_has_inclusive_endpoints() {
    std::array<std::uint8_t, 8> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 8, 8);

    drawLine(canvas, {0, 0}, {7, 7});

    TEST_ASSERT_EQUAL_UINT32(8U, countPixels(canvas));
    TEST_ASSERT_TRUE(canvas.pixel({0, 0}));
    TEST_ASSERT_TRUE(canvas.pixel({7, 7}));
}

void test_dotted_line_uses_one_on_two_off() {
    std::array<std::uint8_t, 1> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 8, 1);

    drawDottedLine(canvas, {0, 0}, {7, 0});

    TEST_ASSERT_TRUE(canvas.pixel({0, 0}));
    TEST_ASSERT_FALSE(canvas.pixel({1, 0}));
    TEST_ASSERT_FALSE(canvas.pixel({2, 0}));
    TEST_ASSERT_TRUE(canvas.pixel({3, 0}));
    TEST_ASSERT_TRUE(canvas.pixel({6, 0}));
}

void test_rectangle_and_fill_rectangle() {
    std::array<std::uint8_t, 8> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 8, 8);

    drawRectangle(canvas, {1, 1, 4, 3});
    TEST_ASSERT_EQUAL_UINT32(10U, countPixels(canvas));

    canvas.clear();
    fillRectangle(canvas, {1, 1, 4, 3});
    TEST_ASSERT_EQUAL_UINT32(12U, countPixels(canvas));
}

void test_circle_is_symmetric() {
    std::array<std::uint8_t, 16> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 16, 8);

    drawCircle(canvas, {7, 3}, 3);

    TEST_ASSERT_TRUE(canvas.pixel({10, 3}));
    TEST_ASSERT_TRUE(canvas.pixel({4, 3}));
    TEST_ASSERT_TRUE(canvas.pixel({7, 0}));
    TEST_ASSERT_TRUE(canvas.pixel({7, 6}));
}

void test_fill_circle_contains_center() {
    std::array<std::uint8_t, 16> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 16, 8);

    fillCircle(canvas, {7, 3}, 2);

    TEST_ASSERT_TRUE(canvas.pixel({7, 3}));
    TEST_ASSERT_TRUE(countPixels(canvas) > 8U);
}

void test_invert_masks_unused_row_bits() {
    std::array<std::uint8_t, 2> buffer{};
    MonochromeCanvas canvas(buffer.data(), buffer.size(), 9, 1);

    canvas.invert();

    TEST_ASSERT_EQUAL_HEX8(0xFFU, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0x80U, buffer[1]);
}

} // namespace

extern "C" {
void setUp() {}
void tearDown() {}
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_canvas_packs_pixels_msb_first);
    RUN_TEST(test_clip_rejects_outside_pixel);
    RUN_TEST(test_horizontal_and_vertical_lines);
    RUN_TEST(test_diagonal_line_has_inclusive_endpoints);
    RUN_TEST(test_dotted_line_uses_one_on_two_off);
    RUN_TEST(test_rectangle_and_fill_rectangle);
    RUN_TEST(test_circle_is_symmetric);
    RUN_TEST(test_fill_circle_contains_center);
    RUN_TEST(test_invert_masks_unused_row_bits);
    return UNITY_END();
}

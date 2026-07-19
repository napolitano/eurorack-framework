/**
 * @file include/eurorack/display/drawing.hpp
 * @brief Declares primitive monochrome drawing operations.
 *
 * @details
 * Provides clipped lines, patterned lines, rectangles, and circles over MonochromeCanvas.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup display
 */

#pragma once

#include <cstdint>
#include <eurorack/display/monochrome_canvas.hpp>

namespace eurorack::display {

/**
 * @brief Repeating line pattern expressed as set and clear run lengths.
 */
struct LinePattern final {
    std::uint16_t onPixels{1U};  ///< Consecutive pixels drawn.
    std::uint16_t offPixels{0U}; ///< Consecutive pixels skipped.

    /**
     * @brief Returns a continuous line pattern.
     *
     * @return Pattern without gaps.
     */
    [[nodiscard]] static constexpr LinePattern solid() noexcept {
        return {1U, 0U};
    }

    /**
     * @brief Returns a conventional dashed pattern.
     *
     * @return Four pixels on and three pixels off.
     */
    [[nodiscard]] static constexpr LinePattern dashed() noexcept {
        return {4U, 3U};
    }

    /**
     * @brief Returns a conventional dotted pattern.
     *
     * @return One pixel on and two pixels off.
     */
    [[nodiscard]] static constexpr LinePattern dotted() noexcept {
        return {1U, 2U};
    }
};

/**
 * @brief Draws a horizontal line.
 *
 * @param canvas Target canvas.
 * @param x Left coordinate.
 * @param y Vertical coordinate.
 * @param length Number of pixels; negative values draw leftward.
 * @param operation Pixel operation.
 */
void drawHorizontalLine(MonochromeCanvas& canvas,
                        std::int32_t x,
                        std::int32_t y,
                        std::int32_t length,
                        PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws a vertical line.
 *
 * @param canvas Target canvas.
 * @param x Horizontal coordinate.
 * @param y Top coordinate.
 * @param length Number of pixels; negative values draw upward.
 * @param operation Pixel operation.
 */
void drawVerticalLine(MonochromeCanvas& canvas,
                      std::int32_t x,
                      std::int32_t y,
                      std::int32_t length,
                      PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws a line using Bresenham rasterization.
 *
 * @param canvas Target canvas.
 * @param start Start point.
 * @param end Inclusive end point.
 * @param operation Pixel operation.
 * @param pattern Repeating line pattern.
 * @param patternOffset Initial phase within the repeating pattern.
 */
void drawLine(MonochromeCanvas& canvas,
              Point start,
              Point end,
              PixelOperation operation = PixelOperation::Set,
              LinePattern pattern = LinePattern::solid(),
              std::uint32_t patternOffset = 0U) noexcept;

/**
 * @brief Draws a dashed line.
 *
 * @param canvas Target canvas.
 * @param start Start point.
 * @param end Inclusive end point.
 * @param operation Pixel operation.
 */
void drawDashedLine(MonochromeCanvas& canvas,
                    Point start,
                    Point end,
                    PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws a dotted line.
 *
 * @param canvas Target canvas.
 * @param start Start point.
 * @param end Inclusive end point.
 * @param operation Pixel operation.
 */
void drawDottedLine(MonochromeCanvas& canvas,
                    Point start,
                    Point end,
                    PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws an unfilled rectangle.
 *
 * @param canvas Target canvas.
 * @param rectangle Rectangle dimensions.
 * @param operation Pixel operation.
 */
void drawRectangle(MonochromeCanvas& canvas,
                   Rect rectangle,
                   PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws a filled rectangle.
 *
 * @param canvas Target canvas.
 * @param rectangle Rectangle dimensions.
 * @param operation Pixel operation.
 */
void fillRectangle(MonochromeCanvas& canvas,
                   Rect rectangle,
                   PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws an unfilled circle.
 *
 * @param canvas Target canvas.
 * @param center Circle center.
 * @param radius Radius in pixels.
 * @param operation Pixel operation.
 */
void drawCircle(MonochromeCanvas& canvas,
                Point center,
                std::int32_t radius,
                PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws a filled circle.
 *
 * @param canvas Target canvas.
 * @param center Circle center.
 * @param radius Radius in pixels.
 * @param operation Pixel operation.
 */
void fillCircle(MonochromeCanvas& canvas,
                Point center,
                std::int32_t radius,
                PixelOperation operation = PixelOperation::Set) noexcept;

} // namespace eurorack::display

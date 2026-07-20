/**
 * @file src/display/drawing.cpp
 * @brief Implements primitive monochrome drawing operations.
 *
 * @details
 * Uses integer rasterization and delegates clipping to MonochromeCanvas.
 *
 * Lines use an integer Bresenham error accumulator. Circles use midpoint
 * symmetry. Rectangles normalize negative extents before emitting edges or
 * fills. Final writes always pass through the canvas, so partially visible
 * shapes require no temporary clipping geometry.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#if defined(__AVR__)
#include <eurorack/compat/avr/algorithm.hpp>
#else
#include <algorithm>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdlib.hpp>
#else
#include <cstdlib>
#endif
#include <eurorack/display/drawing.hpp>

namespace eurorack::display {

namespace {

/*
 * Purpose: Tests whether a patterned line contains a foreground pixel at a step.
 * pattern: Repeating on and off pixel counts plus phase offset.
 * position: Zero-based position along the rasterized line.
 * Returns: True when the current pattern phase is inside the on interval.
 */
bool patternDraws(const LinePattern pattern, const std::uint32_t position) noexcept {
    const std::uint32_t on = pattern.onPixels;
    const std::uint32_t off = pattern.offPixels;
    const std::uint32_t period = on + off;

    if (on == 0U) {
        return false;
    }

    if (period == 0U || off == 0U) {
        return true;
    }

    return position % period < on;
}

} // namespace

void drawHorizontalLine(MonochromeCanvas& canvas,
                        std::int32_t x,
                        std::int32_t y,
                        std::int32_t length,
                        PixelOperation operation) noexcept {
    if (length == 0) {
        return;
    }

    if (length < 0) {
        x += length + 1;
        length = -length;
    }

    for (std::int32_t offset = 0; offset < length; ++offset) {
        static_cast<void>(canvas.applyPixel({x + offset, y}, operation));
    }
}

void drawVerticalLine(MonochromeCanvas& canvas,
                      const std::int32_t x,
                      std::int32_t y,
                      std::int32_t length,
                      const PixelOperation operation) noexcept {
    if (length == 0) {
        return;
    }

    if (length < 0) {
        y += length + 1;
        length = -length;
    }

    for (std::int32_t offset = 0; offset < length; ++offset) {
        static_cast<void>(canvas.applyPixel({x, y + offset}, operation));
    }
}

void drawLine(MonochromeCanvas& canvas,
              Point start,
              const Point end,
              const PixelOperation operation,
              const LinePattern pattern,
              const std::uint32_t patternOffset) noexcept {
    const std::int32_t deltaX = std::abs(end.x - start.x);
    const std::int32_t stepX = start.x < end.x ? 1 : -1;
    const std::int32_t deltaY = -std::abs(end.y - start.y);
    const std::int32_t stepY = start.y < end.y ? 1 : -1;
    // Bresenham keeps all arithmetic integral. deltaY is negative so that
    // one accumulated error term can decide both x and y advances.    // deltaY is negative. One
    // signed error term can therefore decide both horizontal and vertical movement without
    // floating-point arithmetic.

    std::int32_t error = deltaX + deltaY;
    std::uint32_t position = patternOffset;

    while (true) {
        if (patternDraws(pattern, position)) {
            static_cast<void>(canvas.applyPixel(start, operation));
        }

        if (start.x == end.x && start.y == end.y) {
            break;
        }

        const std::int32_t doubledError = error * 2;
        if (doubledError >= deltaY) {
            error += deltaY;
            start.x += stepX;
        }
        if (doubledError <= deltaX) {
            error += deltaX;
            start.y += stepY;
        }

        ++position;
    }
}

void drawDashedLine(MonochromeCanvas& canvas,
                    const Point start,
                    const Point end,
                    const PixelOperation operation) noexcept {
    drawLine(canvas, start, end, operation, LinePattern::dashed());
}

void drawDottedLine(MonochromeCanvas& canvas,
                    const Point start,
                    const Point end,
                    const PixelOperation operation) noexcept {
    drawLine(canvas, start, end, operation, LinePattern::dotted());
}

void drawRectangle(MonochromeCanvas& canvas,
                   const Rect rectangle,
                   const PixelOperation operation) noexcept {
    if (!rectangle.valid()) {
        return;
    }

    drawHorizontalLine(canvas, rectangle.x, rectangle.y, rectangle.width, operation);

    if (rectangle.height > 1) {
        drawHorizontalLine(canvas, rectangle.x, rectangle.bottom() - 1, rectangle.width, operation);
    }

    if (rectangle.height > 2) {
        drawVerticalLine(canvas, rectangle.x, rectangle.y + 1, rectangle.height - 2, operation);

        if (rectangle.width > 1) {
            drawVerticalLine(
                canvas, rectangle.right() - 1, rectangle.y + 1, rectangle.height - 2, operation);
        }
    }
}

void fillRectangle(MonochromeCanvas& canvas,
                   const Rect rectangle,
                   const PixelOperation operation) noexcept {
    if (!rectangle.valid()) {
        return;
    }

    for (std::int32_t y = rectangle.y; y < rectangle.bottom(); ++y) {
        drawHorizontalLine(canvas, rectangle.x, y, rectangle.width, operation);
    }
}

void drawCircle(MonochromeCanvas& canvas,
                const Point center,
                const std::int32_t radius,
                const PixelOperation operation) noexcept {
    if (radius < 0) {
        return;
    }

    if (radius == 0) {
        static_cast<void>(canvas.applyPixel(center, operation));
        return;
    }

    std::int32_t x = radius;
    std::int32_t y = 0;
    std::int32_t error = 1 - radius;

    // Generate one octant and reflect it into the remaining seven. Stop
    // when x and y cross, which marks completion of the first 45 degrees.    // Rasterize one
    // octant and reflect it into the remaining seven. Stop at the diagonal where the first octant
    // is complete.

    while (x >= y) {
        const Point points[8] = {{center.x + x, center.y + y},
                                 {center.x + y, center.y + x},
                                 {center.x - y, center.y + x},
                                 {center.x - x, center.y + y},
                                 {center.x - x, center.y - y},
                                 {center.x - y, center.y - x},
                                 {center.x + y, center.y - x},
                                 {center.x + x, center.y - y}};

        for (const Point point : points) {
            static_cast<void>(canvas.applyPixel(point, operation));
        }

        ++y;
        if (error < 0) {
            error += 2 * y + 1;
        } else {
            --x;
            error += 2 * (y - x) + 1;
        }
    }
}

void fillCircle(MonochromeCanvas& canvas,
                const Point center,
                const std::int32_t radius,
                const PixelOperation operation) noexcept {
    if (radius < 0) {
        return;
    }

    std::int32_t x = radius;
    std::int32_t y = 0;
    std::int32_t error = 1 - radius;

    while (x >= y) {
        drawHorizontalLine(canvas, center.x - x, center.y + y, 2 * x + 1, operation);
        drawHorizontalLine(canvas, center.x - x, center.y - y, 2 * x + 1, operation);
        drawHorizontalLine(canvas, center.x - y, center.y + x, 2 * y + 1, operation);
        drawHorizontalLine(canvas, center.x - y, center.y - x, 2 * y + 1, operation);

        ++y;
        if (error < 0) {
            error += 2 * y + 1;
        } else {
            --x;
            error += 2 * (y - x) + 1;
        }
    }
}

} // namespace eurorack::display

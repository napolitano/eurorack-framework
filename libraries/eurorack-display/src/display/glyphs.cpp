/**
 * @file src/display/glyphs.cpp
 * @brief Implements built-in monochrome UI glyphs.
 *
 * @details
 * Provides glyph lookup and fallback behavior for the built-in bitmap font.
 * Character codes outside the supported printable range resolve to the configured replacement glyph
 * rather than indexing beyond the font table. Glyph data remains immutable and is returned as
 * lightweight views; no allocation or text shaping occurs.
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
#include <eurorack/display/glyphs.hpp>

namespace eurorack::display {

namespace {

std::int32_t centerX(const Rect bounds) noexcept {
    return bounds.x + bounds.width / 2;
}

std::int32_t centerY(const Rect bounds) noexcept {
    return bounds.y + bounds.height / 2;
}

std::int32_t left(const Rect bounds) noexcept {
    return bounds.x;
}

std::int32_t right(const Rect bounds) noexcept {
    return bounds.x + std::max<std::int32_t>(bounds.width - 1, 0);
}

std::int32_t top(const Rect bounds) noexcept {
    return bounds.y;
}

std::int32_t bottom(const Rect bounds) noexcept {
    return bounds.y + std::max<std::int32_t>(bounds.height - 1, 0);
}

} // namespace

void drawGlyph(MonochromeCanvas& canvas,
               const Rect bounds,
               const BasicGlyph glyph,
               const PixelOperation operation) noexcept {
    if (bounds.width <= 0 || bounds.height <= 0) {
        return;
    }

    const std::int32_t cx = centerX(bounds);
    const std::int32_t cy = centerY(bounds);
    const std::int32_t l = left(bounds);
    const std::int32_t r = right(bounds);
    const std::int32_t t = top(bounds);
    const std::int32_t b = bottom(bounds);

    switch (glyph) {
    case BasicGlyph::ArrowLeft:
        drawLine(canvas, {r, cy}, {l, cy}, operation);
        drawLine(canvas, {l, cy}, {cx, t}, operation);
        drawLine(canvas, {l, cy}, {cx, b}, operation);
        break;

    case BasicGlyph::ArrowRight:
        drawLine(canvas, {l, cy}, {r, cy}, operation);
        drawLine(canvas, {r, cy}, {cx, t}, operation);
        drawLine(canvas, {r, cy}, {cx, b}, operation);
        break;

    case BasicGlyph::ArrowUp:
        drawLine(canvas, {cx, b}, {cx, t}, operation);
        drawLine(canvas, {cx, t}, {l, cy}, operation);
        drawLine(canvas, {cx, t}, {r, cy}, operation);
        break;

    case BasicGlyph::ArrowDown:
        drawLine(canvas, {cx, t}, {cx, b}, operation);
        drawLine(canvas, {cx, b}, {l, cy}, operation);
        drawLine(canvas, {cx, b}, {r, cy}, operation);
        break;

    case BasicGlyph::Check:
        drawLine(canvas, {l, cy}, {cx, b}, operation);
        drawLine(canvas, {cx, b}, {r, t}, operation);
        break;

    case BasicGlyph::Cross:
        drawLine(canvas, {l, t}, {r, b}, operation);
        drawLine(canvas, {l, b}, {r, t}, operation);
        break;

    case BasicGlyph::Plus:
        drawHorizontalLine(canvas, l, cy, bounds.width, operation);
        drawVerticalLine(canvas, cx, t, bounds.height, operation);
        break;

    case BasicGlyph::Minus:
        drawHorizontalLine(canvas, l, cy, bounds.width, operation);
        break;
    }
}

} // namespace eurorack::display

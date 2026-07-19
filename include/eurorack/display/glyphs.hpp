/**
 * @file include/eurorack/display/glyphs.hpp
 * @brief Declares built-in monochrome UI glyphs.
 *
 * @details
 * Provides scalable arrow and status symbols using display primitives.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup display
 */

#pragma once

#include <cstdint>
#include <eurorack/display/drawing.hpp>
#include <eurorack/display/geometry.hpp>
#include <eurorack/display/monochrome_canvas.hpp>

namespace eurorack::display {

/**
 * @brief Built-in UI glyph identifiers.
 */
enum class BasicGlyph : std::uint8_t {
    ArrowLeft,
    ArrowRight,
    ArrowUp,
    ArrowDown,
    Check,
    Cross,
    Plus,
    Minus
};

/**
 * @brief Draws one glyph centered inside a target rectangle.
 *
 * @param canvas Target canvas.
 * @param bounds Glyph bounds.
 * @param glyph Glyph identifier.
 * @param operation Pixel operation.
 */
void drawGlyph(MonochromeCanvas& canvas,
               Rect bounds,
               BasicGlyph glyph,
               PixelOperation operation = PixelOperation::Set) noexcept;

} // namespace eurorack::display

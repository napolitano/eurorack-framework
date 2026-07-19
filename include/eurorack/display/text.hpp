/**
 * @file include/eurorack/display/text.hpp
 * @brief Declares bitmap-font text rendering and alignment.
 *
 * @details
 * Provides display-independent text measurement, clipping, alignment, normal and inverted
 * rendering.
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

#include <cstddef>
#include <cstdint>
#include <eurorack/display/geometry.hpp>
#include <eurorack/display/monochrome_canvas.hpp>
#include <string_view>

namespace eurorack::display {

/**
 * @brief Horizontal text alignment within a target rectangle.
 */
enum class HorizontalAlignment : std::uint8_t { Left, Center, Right };

/**
 * @brief Vertical text alignment within a target rectangle.
 */
enum class VerticalAlignment : std::uint8_t { Top, Middle, Bottom };

/**
 * @brief Rendering mode for monochrome text.
 */
enum class TextMode : std::uint8_t { Normal, Inverted };

/**
 * @brief Monochrome bitmap font descriptor.
 *
 * Glyphs are stored as fixed-width columns, one byte per column, least
 * significant bit at the top of the glyph.
 */
struct BitmapFont final {
    const std::uint8_t* glyphData{nullptr};
    std::uint8_t firstCharacter{32U};
    std::uint8_t lastCharacter{126U};
    std::uint8_t glyphWidth{5U};
    std::uint8_t glyphHeight{7U};
    std::uint8_t horizontalSpacing{1U};
};

/**
 * @brief Text rendering options.
 */
struct TextStyle final {
    HorizontalAlignment horizontalAlignment{HorizontalAlignment::Left};
    VerticalAlignment verticalAlignment{VerticalAlignment::Top};
    TextMode mode{TextMode::Normal};
    PixelOperation foreground{PixelOperation::Set};
    PixelOperation background{PixelOperation::Clear};
    bool clipToBounds{true};
};

/**
 * @brief Width and height of rendered text.
 */
struct TextMetrics final {
    std::int32_t width{0};
    std::int32_t height{0};
};

/**
 * @brief Returns the framework's built-in 5x7 ASCII font.
 *
 * @return Constant reference to the font descriptor.
 */
[[nodiscard]] const BitmapFont& font5x7() noexcept;

/**
 * @brief Measures one text string.
 *
 * @param text Text to measure.
 * @param font Bitmap font descriptor.
 * @return Pixel dimensions.
 */
[[nodiscard]] TextMetrics measureText(std::string_view text,
                                      const BitmapFont& font = font5x7()) noexcept;

/**
 * @brief Draws one character at an explicit origin.
 *
 * @param canvas Target canvas.
 * @param origin Upper-left glyph origin.
 * @param character Character to draw.
 * @param font Bitmap font descriptor.
 * @param operation Pixel operation for foreground pixels.
 */
void drawCharacter(MonochromeCanvas& canvas,
                   Point origin,
                   char character,
                   const BitmapFont& font = font5x7(),
                   PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws text at an explicit origin.
 *
 * @param canvas Target canvas.
 * @param origin Upper-left text origin.
 * @param text Text to draw.
 * @param font Bitmap font descriptor.
 * @param operation Pixel operation for foreground pixels.
 */
void drawText(MonochromeCanvas& canvas,
              Point origin,
              std::string_view text,
              const BitmapFont& font = font5x7(),
              PixelOperation operation = PixelOperation::Set) noexcept;

/**
 * @brief Draws aligned text inside a target rectangle.
 *
 * @param canvas Target canvas.
 * @param bounds Target rectangle.
 * @param text Text to draw.
 * @param style Alignment and rendering options.
 * @param font Bitmap font descriptor.
 */
void drawText(MonochromeCanvas& canvas,
              Rect bounds,
              std::string_view text,
              const TextStyle& style,
              const BitmapFont& font = font5x7()) noexcept;

} // namespace eurorack::display

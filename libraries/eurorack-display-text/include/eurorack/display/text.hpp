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

#if defined(__AVR__)
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <cstddef>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/display/geometry.hpp>
#include <eurorack/display/monochrome_canvas.hpp>
#if defined(__AVR__)
#include <eurorack/compat/avr/string_view.hpp>
#else
#include <string_view>
#endif

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
    const std::uint8_t* glyphData{nullptr}; ///< Flat glyph table, `glyphWidth` bytes per
                                            ///< character in ascending character-code order;
                                            ///< bit `row` (from the LSB) of each byte selects
                                            ///< the pixel at that column and row. On AVR, this
                                            ///< must point into flash (`PROGMEM`), matching the
                                            ///< built-in `font5x7()` table; `drawCharacter` reads
                                            ///< it with `pgm_read_byte` on that platform rather
                                            ///< than a plain dereference.
    std::uint8_t firstCharacter{32U};       ///< Lowest character code present in `glyphData`.
    std::uint8_t lastCharacter{126U};       ///< Highest character code present in `glyphData`;
                                      ///< characters outside `[firstCharacter, lastCharacter]`
                                      ///< render as `?`, which must itself be in range.
    std::uint8_t glyphWidth{5U};        ///< Glyph width in pixels and columns per character.
    std::uint8_t glyphHeight{7U};       ///< Glyph height in pixels; must not exceed 8 because each
                                        ///< column byte encodes one bit per row.
    std::uint8_t horizontalSpacing{1U}; ///< Extra pixel gap added after each glyph when advancing
                                        ///< the cursor; excluded from the last character in
                                        ///< measured text width.
};

/**
 * @brief Text rendering options.
 */
struct TextStyle final {
    HorizontalAlignment horizontalAlignment{
        HorizontalAlignment::Left}; ///< Horizontal placement
                                    ///< of the measured text within the target rectangle.
    VerticalAlignment verticalAlignment{
        VerticalAlignment::Top};     ///< Vertical placement of the
                                     ///< measured text within the target rectangle.
    TextMode mode{TextMode::Normal}; ///< `Normal` draws glyphs with `foreground` only. `Inverted`
                                     ///< first fills the whole target rectangle with
                                     ///< `foreground`, then draws glyphs with `background`,
                                     ///< producing light-on-dark text.
    PixelOperation foreground{
        PixelOperation::Set}; ///< Operation used for glyph pixels in
                              ///< `Normal` mode, or for the filled rectangle in `Inverted` mode.
    PixelOperation background{PixelOperation::Clear}; ///< Unused in `Normal` mode. In `Inverted`
                                                      ///< mode, the operation used to draw glyph
                                                      ///< pixels over the filled rectangle.
    bool clipToBounds{true}; ///< True to temporarily clip drawing to the target rectangle and
                             ///< restore the previous clip afterward.
};

/**
 * @brief Width and height of rendered text.
 */
struct TextMetrics final {
    std::int32_t width{0};  ///< Measured pixel width, excluding trailing `horizontalSpacing`
                            ///< after the last character; `0` for empty text.
    std::int32_t height{0}; ///< Measured pixel height, equal to the font's `glyphHeight`.
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

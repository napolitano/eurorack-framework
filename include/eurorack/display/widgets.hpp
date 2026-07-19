/**
 * @file include/eurorack/display/widgets.hpp
 * @brief Declares reusable monochrome UI widgets.
 *
 * @details
 * Provides stateless rendering for parameters, knobs, encoders, bars, menus, and modal overlays.
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
#include <eurorack/display/glyphs.hpp>
#include <eurorack/display/text.hpp>
#include <string_view>

namespace eurorack::display {

/**
 * @brief Parameter presentation model.
 */
struct ParameterView final {
    std::string_view label{}; ///< Centered on the top ~8-pixel row of the widget.
    std::string_view value{}; ///< Centered on the row below the label; combined with `unit`
                              ///< (separated by a space) into one string of up to 31
                              ///< characters.
    std::string_view unit{};  ///< Appended after `value`; omitted entirely (including the
                              ///< separating space) when empty.
    bool selected{false};     ///< Draws a focus-frame rectangle around the widget, insets the
                              ///< content by one pixel, and renders the value row in inverted
                              ///< text mode.
};

/**
 * @brief Normalized potentiometer presentation model.
 */
struct PotentiometerView final {
    std::string_view label{}; ///< Shown below the knob only when `value` is empty.
    float normalized{0.0F};   ///< Clamped to [0, 1]; drives the needle angle over a 270-degree
                              ///< sweep starting at the lower-left (135 degrees) and ending at
                              ///< the lower-right (405 degrees).
    std::string_view value{}; ///< Shown below the knob; takes priority over `label` when
                              ///< non-empty.
    bool selected{false};     ///< Draws a focus-frame rectangle around the widget and insets the
                              ///< knob by one pixel.
};

/**
 * @brief Encoder presentation model.
 */
struct EncoderView final {
    std::string_view label{};  ///< Rendered below the knob; the underlying knob widget is always
                               ///< invoked without separate value text, so `label` is always
                               ///< shown.
    std::int32_t value{0};     ///< Current value; combined with `minimum`/`maximum` into a
                               ///< normalized knob position (see @ref PotentiometerView).
    std::int32_t minimum{0};   ///< Lower end of the value range used only for normalization.
    std::int32_t maximum{127}; ///< Upper end of the value range used only for normalization.
                               ///< Treated as at least `minimum + 1` to avoid division by zero.
    bool wrap{false};     ///< Draws a small right-arrow glyph in the widget's upper-right corner to
                          ///< indicate that the value wraps at its bounds.
    bool selected{false}; ///< Forwarded to the underlying knob widget's focus-frame rendering.
};

/**
 * @brief Progress bar presentation model.
 */
struct ProgressBarView final {
    float normalized{0.0F}; ///< Clamped to [0, 1]; fraction of the inner width drawn as filled.
    bool showFrame{true};   ///< Draws an outline rectangle around the full bounds and insets the
                            ///< fillable area by one pixel; when false, the fill uses the full
                            ///< bounds directly.
    bool inverted{false};   ///< When false, only the filled portion is drawn solid; the remainder
                            ///< of the inner area is left untouched. When true, the entire inner
                            ///< area is first filled solid and then everything past the filled
                            ///< portion is explicitly cleared, guaranteeing a blank remainder
                            ///< instead of leaving prior canvas content in place.
};

/**
 * @brief Top-bar presentation model.
 */
struct TopBarView final {
    std::string_view title{};  ///< Rendered left-aligned in the left half of the bar.
    std::string_view status{}; ///< Rendered right-aligned in the right half of the bar.
    bool inverted{true}; ///< Renders both halves in inverted text mode, which fills each half's
                         ///< own rectangle solid and draws the text over it; when false, text
                         ///< is drawn normally over an untouched background.
};

/**
 * @brief Footer-bar presentation model.
 */
struct FooterBarView final {
    std::string_view left{};   ///< Rendered left-aligned, overlapping the full bar width.
    std::string_view center{}; ///< Rendered horizontally centered, overlapping the full bar
                               ///< width.
    std::string_view right{};  ///< Rendered right-aligned, overlapping the full bar width.
    bool inverted{true};       ///< Explicitly fills the entire bar solid before drawing all three
                               ///< texts in inverted mode; when false, the bar background is left
                               ///< untouched and text is drawn normally.
};

/**
 * @brief One list-menu item.
 */
struct MenuItemView final {
    std::string_view label{}; ///< Rendered left-aligned within the row, inset by one pixel.
    std::string_view value{}; ///< Rendered right-aligned within the same row rectangle as
                              ///< `label`.
    bool enabled{true};       ///< False renders both `label` and `value` with the `Invert` pixel
                        ///< operation instead of `Set`, toggling existing canvas pixels rather
                        ///< than forcing them on, to visually distinguish disabled items.
};

/**
 * @brief Scrollable list-menu presentation model.
 */
struct MenuListView final {
    const MenuItemView* items{nullptr}; ///< Pointer to `itemCount` items; rendering is skipped
                                        ///< entirely when null.
    std::size_t itemCount{0U};     ///< Total number of items behind `items`; rendering is skipped
                                   ///< when zero.
    std::size_t selectedIndex{0U}; ///< Highlighted item index; clamped to the last valid index
                                   ///< internally. The highlighted row is filled solid and its
                                   ///< text drawn in inverted mode.
    std::size_t firstVisibleIndex{0U}; ///< Scroll-position hint; automatically adjusted by
                                       ///< @ref ensureMenuSelectionVisible so `selectedIndex` is
                                       ///< always within the visible rows.
    std::uint8_t rowHeight{9U};        ///< Pixel height per row; rendering is skipped when zero.
    bool drawScrollbar{true}; ///< Reserves a 3-pixel-wide column on the right whenever there is
                              ///< more than one item; the track and a proportionally sized
                              ///< thumb are actually drawn into that column only when the item
                              ///< count exceeds the number of visible rows.
};

/**
 * @brief Confirmation choice.
 */
enum class ConfirmationChoice : std::uint8_t { Yes, Cancel };

/**
 * @brief Confirmation overlay presentation model.
 */
struct ConfirmationOverlayView final {
    std::string_view title{};   ///< Rendered centered in an inverted ~9-pixel header row.
    std::string_view message{}; ///< Word-wrapped across the area between the header and the two
                                ///< buttons, using the built-in 5x7 font.
    ConfirmationChoice selected{ConfirmationChoice::Cancel}; ///< Which button is drawn in the
                                                             ///< filled/inverted (highlighted)
                                                             ///< style; see
                                                             ///< @ref drawConfirmationOverlay.
};

/**
 * @brief Error overlay presentation model.
 */
struct ErrorOverlayView final {
    std::string_view title{"Error"};    ///< Rendered next to a warning glyph in an inverted header
                                        ///< row; see @ref drawErrorOverlay.
    std::string_view message{};         ///< Word-wrapped below the header.
    std::string_view actionLabel{"OK"}; ///< Label of the single full-width action button.
};

/**
 * @brief Returns the number of menu rows that fit inside a rectangle.
 *
 * @param bounds Available menu area.
 * @param rowHeight Row height in pixels.
 * @return Number of complete visible rows.
 */
[[nodiscard]] std::size_t visibleMenuRowCount(Rect bounds, std::uint8_t rowHeight) noexcept;

/**
 * @brief Calculates the first visible menu index needed to show a selection.
 *
 * @param selectedIndex Selected menu item.
 * @param currentFirstVisible Current first visible menu item.
 * @param itemCount Total item count.
 * @param visibleRows Number of visible rows.
 * @return Adjusted first visible index.
 */
[[nodiscard]] std::size_t ensureMenuSelectionVisible(std::size_t selectedIndex,
                                                     std::size_t currentFirstVisible,
                                                     std::size_t itemCount,
                                                     std::size_t visibleRows) noexcept;

/**
 * @brief Renders a labeled value with an optional unit, in a two-row layout.
 *
 * @details
 * The label occupies a centered row at most 8 pixels tall at the top of `bounds`. The remaining
 * area below it shows `value` and `unit` centered and combined into a single string (space
 * separated, truncated to 31 characters). When `view.selected` is true, a focus-frame rectangle
 * is drawn around `bounds`, the content is inset by one pixel, and the value row uses inverted
 * text mode.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawParameter(MonochromeCanvas& canvas, Rect bounds, const ParameterView& view) noexcept;

/**
 * @brief Renders a rotary knob with a 270-degree needle sweep and a text label below it.
 *
 * @details
 * Draws a circle sized to fit `bounds` (leaving room for the text below) and a needle from the
 * center to the circle edge at an angle from 135 degrees (`normalized == 0`) to 405 degrees
 * (`normalized == 1`), sweeping through the top of the circle. Below the knob, `view.value` is
 * shown if non-empty, otherwise `view.label`. When `view.selected` is true, a focus-frame
 * rectangle is drawn around `bounds` and the knob is inset by one pixel.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawPotentiometer(MonochromeCanvas& canvas,
                       Rect bounds,
                       const PotentiometerView& view) noexcept;

/**
 * @brief Renders an encoder value using the same rotary-knob widget as `drawPotentiometer()`.
 *
 * @details
 * `value` is normalized against `minimum` and `maximum` (the span is treated as at least 1) and
 * passed to `drawPotentiometer()` together with `label` and `selected`; no separate value text
 * is shown. When `view.wrap` is true, a small right-arrow glyph is drawn in the upper-right
 * corner of `bounds` to indicate that the value wraps at its bounds.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawEncoder(MonochromeCanvas& canvas, Rect bounds, const EncoderView& view) noexcept;

/**
 * @brief Renders a horizontal progress bar.
 *
 * @details
 * Does nothing if `bounds` has zero or negative width or height. When `view.showFrame` is true,
 * an outline rectangle is drawn around `bounds` and the fillable area is inset by one pixel;
 * otherwise the fillable area is `bounds` itself. See `ProgressBarView::inverted` for how
 * `view.inverted` changes whether the unfilled remainder is left untouched or explicitly cleared.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawProgressBar(MonochromeCanvas& canvas, Rect bounds, const ProgressBarView& view) noexcept;

/**
 * @brief Renders a two-column status bar with a left-aligned title and right-aligned status.
 *
 * @details
 * Delegates to the same label/value layout as other framework widgets: `title` occupies the
 * left half of `bounds` and `status` the right half. See `TopBarView::inverted` for the
 * effect of `view.inverted` on text rendering.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawTopBar(MonochromeCanvas& canvas, Rect bounds, const TopBarView& view) noexcept;

/**
 * @brief Renders a three-column footer bar with left, center, and right-aligned text.
 *
 * @details
 * All three of `view.left`, `view.center`, and `view.right` are drawn over the full width of
 * `bounds` using left, center, and right horizontal alignment respectively, so they may overlap
 * if the combined text is too wide. See `FooterBarView::inverted` for the effect of
 * `view.inverted` on the bar background and text rendering.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawFooterBar(MonochromeCanvas& canvas, Rect bounds, const FooterBarView& view) noexcept;

/**
 * @brief Renders a scrollable list of menu items with an optional scrollbar.
 *
 * @details
 * Does nothing if `view.items` is null, `view.itemCount` is zero, or `view.rowHeight` is zero.
 * The visible row count is derived from `bounds` and `view.rowHeight` via
 * `visibleMenuRowCount()`, and the first visible item is adjusted via
 * `ensureMenuSelectionVisible()` so `view.selectedIndex` is always on screen. Each visible item
 * is drawn with `MenuItemView::label` left-aligned and `MenuItemView::value`
 * right-aligned within its row; the selected row is filled solid with inverted text. See
 * `MenuListView::drawScrollbar` for when the scrollbar column and thumb are drawn.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawMenuList(MonochromeCanvas& canvas, Rect bounds, const MenuListView& view) noexcept;

/**
 * @brief Renders a full-bounds modal dialog with a title, wrapped message, and two buttons.
 *
 * @details
 * Clears `bounds` and draws an outline rectangle around it, then an inverted title row, a
 * word-wrapped message in the remaining middle area, and "Yes" and "Cancel" buttons side by
 * side at the bottom. The button matching `view.selected` is drawn filled/inverted; the other
 * is drawn outlined. See `ConfirmationOverlayView` for the exact layout.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawConfirmationOverlay(MonochromeCanvas& canvas,
                             Rect bounds,
                             const ConfirmationOverlayView& view) noexcept;

/**
 * @brief Renders a full-bounds modal error dialog with an icon, title, message, and one button.
 *
 * @details
 * Clears `bounds` and draws an outline rectangle around it, then a 9x9 cross/warning glyph next
 * to an inverted title row, a word-wrapped message below, and a single full-width `actionLabel`
 * button spanning the bottom, always drawn in its filled/inverted (selected) style. See
 * `ErrorOverlayView` for the exact layout.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 * @param bounds Pixel rectangle allocated to the widget.
 * @param view Immutable view model used only for the duration of the call.
 */
void drawErrorOverlay(MonochromeCanvas& canvas, Rect bounds, const ErrorOverlayView& view) noexcept;

} // namespace eurorack::display

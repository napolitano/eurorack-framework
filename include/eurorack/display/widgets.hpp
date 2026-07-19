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
    std::string_view label{};
    std::string_view value{};
    std::string_view unit{};
    bool selected{false};
};

/**
 * @brief Normalized potentiometer presentation model.
 */
struct PotentiometerView final {
    std::string_view label{};
    float normalized{0.0F};
    std::string_view value{};
    bool selected{false};
};

/**
 * @brief Encoder presentation model.
 */
struct EncoderView final {
    std::string_view label{};
    std::int32_t value{0};
    std::int32_t minimum{0};
    std::int32_t maximum{127};
    bool wrap{false};
    bool selected{false};
};

/**
 * @brief Progress bar presentation model.
 */
struct ProgressBarView final {
    float normalized{0.0F};
    bool showFrame{true};
    bool inverted{false};
};

/**
 * @brief Top-bar presentation model.
 */
struct TopBarView final {
    std::string_view title{};
    std::string_view status{};
    bool inverted{true};
};

/**
 * @brief Footer-bar presentation model.
 */
struct FooterBarView final {
    std::string_view left{};
    std::string_view center{};
    std::string_view right{};
    bool inverted{true};
};

/**
 * @brief One list-menu item.
 */
struct MenuItemView final {
    std::string_view label{};
    std::string_view value{};
    bool enabled{true};
};

/**
 * @brief Scrollable list-menu presentation model.
 */
struct MenuListView final {
    const MenuItemView* items{nullptr};
    std::size_t itemCount{0U};
    std::size_t selectedIndex{0U};
    std::size_t firstVisibleIndex{0U};
    std::uint8_t rowHeight{9U};
    bool drawScrollbar{true};
};

/**
 * @brief Confirmation choice.
 */
enum class ConfirmationChoice : std::uint8_t { Yes, Cancel };

/**
 * @brief Confirmation overlay presentation model.
 */
struct ConfirmationOverlayView final {
    std::string_view title{};
    std::string_view message{};
    ConfirmationChoice selected{ConfirmationChoice::Cancel};
};

/**
 * @brief Error overlay presentation model.
 */
struct ErrorOverlayView final {
    std::string_view title{"Error"};
    std::string_view message{};
    std::string_view actionLabel{"OK"};
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
 * @brief Renders parameter.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawParameter(MonochromeCanvas& canvas, Rect bounds, const ParameterView& view) noexcept;

/**
 * @brief Renders potentiometer.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawPotentiometer(MonochromeCanvas& canvas,
                       Rect bounds,
                       const PotentiometerView& view) noexcept;

/**
 * @brief Renders encoder.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawEncoder(MonochromeCanvas& canvas, Rect bounds, const EncoderView& view) noexcept;

/**
 * @brief Renders progress bar.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawProgressBar(MonochromeCanvas& canvas, Rect bounds, const ProgressBarView& view) noexcept;

/**
 * @brief Renders top bar.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawTopBar(MonochromeCanvas& canvas, Rect bounds, const TopBarView& view) noexcept;

/**
 * @brief Renders footer bar.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawFooterBar(MonochromeCanvas& canvas, Rect bounds, const FooterBarView& view) noexcept;

/**
 * @brief Renders menu list.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawMenuList(MonochromeCanvas& canvas, Rect bounds, const MenuListView& view) noexcept;

/**
 * @brief Renders confirmation overlay.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawConfirmationOverlay(MonochromeCanvas& canvas,
                             Rect bounds,
                             const ConfirmationOverlayView& view) noexcept;

/**
 * @brief Renders error overlay.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param bounds Pixel rectangle allocated to the operation.
 *
 * @param view Immutable view model used only for the duration of the call.
 */
void drawErrorOverlay(MonochromeCanvas& canvas, Rect bounds, const ErrorOverlayView& view) noexcept;

} // namespace eurorack::display

/**
 * @file src/display/widgets.cpp
 * @brief Implements reusable monochrome UI widgets.
 *
 * @details
 * Renders stateless widgets using canvas primitives, text, alignment, and glyphs.
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

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <eurorack/display/drawing.hpp>
#include <eurorack/display/widgets.hpp>

namespace eurorack::display {

namespace {

/*
 * Purpose: Clamps a floating-point value to the normalized range.
 * value: Input value.
 * Returns: `value` limited to the inclusive range 0.0 through 1.0.
 */
float clampNormalized(const float value) noexcept {
    return std::clamp(value, 0.0F, 1.0F);
}

/*
 * Purpose: Narrows a coordinate after saturating it to signed 16-bit range.
 * value: Coordinate or dimension to narrow.
 * Returns: Saturated signed 16-bit representation.
 */
std::int16_t toInt16(const std::int32_t value) noexcept {
    return static_cast<std::int16_t>(std::clamp<std::int32_t>(value, -32768, 32767));
}

/*
 * Purpose: Insets a rectangle without producing negative dimensions.
 * bounds: Source rectangle.
 * amount: Pixels removed from every edge.
 * Returns: Inset rectangle with width and height clamped to zero.
 */
Rect inset(const Rect bounds, const std::int16_t amount) noexcept {
    return {
        bounds.x + amount,
        bounds.y + amount,
        toInt16(std::max<std::int32_t>(0, bounds.width - static_cast<std::int32_t>(amount) * 2)),
        toInt16(std::max<std::int32_t>(0, bounds.height - static_cast<std::int32_t>(amount) * 2))};
}

/*
 * Purpose: Draws a focus frame when a widget is selected.
 * canvas: Destination canvas.
 * bounds: Widget rectangle.
 * selected: True to draw the frame.
 */
void drawSelectionFrame(MonochromeCanvas& canvas, const Rect bounds, const bool selected) noexcept {
    if (selected) {
        drawRectangle(canvas, bounds);
    }
}

/*
 * Purpose: Draws left-aligned label text and right-aligned value text.
 * canvas: Destination canvas.
 * bounds: Shared row rectangle.
 * label: Left-side label.
 * value: Right-side value.
 * inverted: True to render both fields in inverted mode.
 */
void drawLabelAndValue(MonochromeCanvas& canvas,
                       const Rect bounds,
                       const std::string_view label,
                       const std::string_view value,
                       const bool inverted) noexcept {
    const std::int16_t half = static_cast<std::int16_t>(bounds.width / 2);

    TextStyle leftStyle{};
    leftStyle.verticalAlignment = VerticalAlignment::Middle;
    leftStyle.mode = inverted ? TextMode::Inverted : TextMode::Normal;

    TextStyle rightStyle = leftStyle;
    rightStyle.horizontalAlignment = HorizontalAlignment::Right;

    drawText(canvas, {bounds.x, bounds.y, half, bounds.height}, label, leftStyle);

    drawText(
        canvas, {bounds.x + half, bounds.y, bounds.width - half, bounds.height}, value, rightStyle);
}

/*
 * Purpose: Draws a centered text button with selected or framed styling.
 * canvas: Destination canvas.
 * bounds: Button rectangle.
 * label: Button label.
 * selected: True to render the selected style.
 */
void drawButton(MonochromeCanvas& canvas,
                const Rect bounds,
                const std::string_view label,
                const bool selected) noexcept {
    TextStyle style{};
    style.horizontalAlignment = HorizontalAlignment::Center;
    style.verticalAlignment = VerticalAlignment::Middle;
    style.mode = selected ? TextMode::Inverted : TextMode::Normal;

    if (!selected) {
        drawRectangle(canvas, bounds);
    }

    drawText(canvas, bounds, label, style);
}

/*
 * Purpose: Draws a space-wrapped message using the built-in fixed-width font.
 * canvas: Destination canvas.
 * bounds: Text rectangle.
 * message: Message to wrap and render.
 */
void drawWrappedMessage(MonochromeCanvas& canvas,
                        const Rect bounds,
                        const std::string_view message) noexcept {
    const std::size_t columns = bounds.width > 0 ? static_cast<std::size_t>(bounds.width / 6) : 0U;

    if (columns == 0U) {
        return;
    }

    std::size_t start = 0U;
    std::int16_t y = toInt16(bounds.y);

    while (start < message.size() && y + 7 <= bounds.y + bounds.height) {
        std::size_t length = std::min(columns, message.size() - start);

        if (start + length < message.size()) {
            const std::size_t space = message.substr(start, length).find_last_of(' ');
            if (space != std::string_view::npos && space > 0U) {
                length = space;
            }
        }

        drawText(canvas, {bounds.x, y}, message.substr(start, length));

        start += length;
        while (start < message.size() && message[start] == ' ') {
            ++start;
        }

        y = static_cast<std::int16_t>(y + 8);
    }
}

} // namespace

std::size_t visibleMenuRowCount(const Rect bounds, const std::uint8_t rowHeight) noexcept {
    if (bounds.height <= 0 || rowHeight == 0U) {
        return 0U;
    }

    return static_cast<std::size_t>(bounds.height / static_cast<std::int16_t>(rowHeight));
}

std::size_t ensureMenuSelectionVisible(const std::size_t selectedIndex,
                                       const std::size_t currentFirstVisible,
                                       const std::size_t itemCount,
                                       const std::size_t visibleRows) noexcept {
    if (itemCount == 0U || visibleRows == 0U) {
        return 0U;
    }

    const std::size_t selected = std::min(selectedIndex, itemCount - 1U);
    const std::size_t maximumFirst = itemCount > visibleRows ? itemCount - visibleRows : 0U;
    std::size_t first = std::min(currentFirstVisible, maximumFirst);

    if (selected < first) {
        first = selected;
    } else if (selected >= first + visibleRows) {
        first = selected - visibleRows + 1U;
    }

    return std::min(first, maximumFirst);
}

void drawParameter(MonochromeCanvas& canvas,
                   const Rect bounds,
                   const ParameterView& view) noexcept {
    drawSelectionFrame(canvas, bounds, view.selected);

    const Rect inner = inset(bounds, view.selected ? 1 : 0);
    const std::int16_t labelHeight = toInt16(std::min<std::int32_t>(8, inner.height));
    const Rect labelBounds{inner.x, inner.y, inner.width, labelHeight};
    const Rect valueBounds{inner.x,
                           inner.y + labelHeight,
                           inner.width,
                           toInt16(std::max<std::int32_t>(0, inner.height - labelHeight))};

    TextStyle labelStyle{};
    labelStyle.horizontalAlignment = HorizontalAlignment::Center;

    drawText(canvas, labelBounds, view.label, labelStyle);

    char combined[32]{};
    if (view.unit.empty()) {
        std::snprintf(combined,
                      sizeof(combined),
                      "%.*s",
                      static_cast<int>(view.value.size()),
                      view.value.data());
    } else {
        std::snprintf(combined,
                      sizeof(combined),
                      "%.*s %.*s",
                      static_cast<int>(view.value.size()),
                      view.value.data(),
                      static_cast<int>(view.unit.size()),
                      view.unit.data());
    }

    TextStyle valueStyle{};
    valueStyle.horizontalAlignment = HorizontalAlignment::Center;
    valueStyle.verticalAlignment = VerticalAlignment::Middle;
    valueStyle.mode = view.selected ? TextMode::Inverted : TextMode::Normal;

    drawText(canvas, valueBounds, combined, valueStyle);
}

void drawPotentiometer(MonochromeCanvas& canvas,
                       const Rect bounds,
                       const PotentiometerView& view) noexcept {
    drawSelectionFrame(canvas, bounds, view.selected);

    const Rect inner = inset(bounds, view.selected ? 1 : 0);
    const std::int16_t diameter = toInt16(std::max<std::int32_t>(
        5, std::min<std::int32_t>(inner.width, std::max<std::int32_t>(5, inner.height - 9))));
    const std::int16_t radius = static_cast<std::int16_t>(diameter / 2);
    const Point center{static_cast<std::int16_t>(inner.x + inner.width / 2),
                       static_cast<std::int16_t>(inner.y + radius + 1)};

    drawCircle(canvas, center, radius);

    const float normalized = clampNormalized(view.normalized);
    const float angle = 2.35619449F + normalized * 4.71238898F;
    const std::int16_t endpointX = static_cast<std::int16_t>(
        std::lround(static_cast<float>(center.x) +
                    std::cos(angle) * static_cast<float>(std::max<std::int16_t>(1, radius - 2))));
    const std::int16_t endpointY = static_cast<std::int16_t>(
        std::lround(static_cast<float>(center.y) +
                    std::sin(angle) * static_cast<float>(std::max<std::int16_t>(1, radius - 2))));

    drawLine(canvas, center, {endpointX, endpointY});

    const Rect textBounds{
        inner.x,
        static_cast<std::int16_t>(center.y + radius + 1),
        inner.width,
        toInt16(std::max<std::int32_t>(0, inner.y + inner.height - (center.y + radius + 1)))};

    TextStyle style{};
    style.horizontalAlignment = HorizontalAlignment::Center;
    style.verticalAlignment = VerticalAlignment::Bottom;
    drawText(canvas, textBounds, view.value.empty() ? view.label : view.value, style);
}

void drawEncoder(MonochromeCanvas& canvas, const Rect bounds, const EncoderView& view) noexcept {
    const std::int32_t span = std::max<std::int32_t>(1, view.maximum - view.minimum);
    const float normalized =
        clampNormalized(static_cast<float>(view.value - view.minimum) / static_cast<float>(span));

    drawPotentiometer(canvas, bounds, {view.label, normalized, {}, view.selected});

    if (view.wrap) {
        const Rect glyphBounds{
            static_cast<std::int16_t>(bounds.x + bounds.width - 7), bounds.y, 7, 7};
        drawGlyph(canvas, glyphBounds, BasicGlyph::ArrowRight);
    }
}

void drawProgressBar(MonochromeCanvas& canvas,
                     const Rect bounds,
                     const ProgressBarView& view) noexcept {
    if (bounds.width <= 0 || bounds.height <= 0) {
        return;
    }

    if (view.showFrame) {
        drawRectangle(canvas, bounds);
    }

    const Rect inner = view.showFrame ? inset(bounds, 1) : bounds;
    const float normalized = clampNormalized(view.normalized);
    const std::int16_t filled =
        static_cast<std::int16_t>(std::lround(normalized * static_cast<float>(inner.width)));

    if (view.inverted) {
        fillRectangle(canvas, inner);
        if (filled < inner.width) {
            fillRectangle(canvas,
                          {static_cast<std::int16_t>(inner.x + filled),
                           inner.y,
                           static_cast<std::int16_t>(inner.width - filled),
                           inner.height},
                          PixelOperation::Clear);
        }
    } else if (filled > 0) {
        fillRectangle(canvas, {inner.x, inner.y, filled, inner.height});
    }
}

void drawTopBar(MonochromeCanvas& canvas, const Rect bounds, const TopBarView& view) noexcept {
    drawLabelAndValue(canvas, bounds, view.title, view.status, view.inverted);
}

void drawFooterBar(MonochromeCanvas& canvas,
                   const Rect bounds,
                   const FooterBarView& view) noexcept {
    if (view.inverted) {
        fillRectangle(canvas, bounds);
    }

    TextStyle leftStyle{};
    leftStyle.verticalAlignment = VerticalAlignment::Middle;
    leftStyle.mode = view.inverted ? TextMode::Inverted : TextMode::Normal;

    TextStyle centerStyle = leftStyle;
    centerStyle.horizontalAlignment = HorizontalAlignment::Center;

    TextStyle rightStyle = leftStyle;
    rightStyle.horizontalAlignment = HorizontalAlignment::Right;

    drawText(canvas, bounds, view.left, leftStyle);
    drawText(canvas, bounds, view.center, centerStyle);
    drawText(canvas, bounds, view.right, rightStyle);
}

void drawMenuList(MonochromeCanvas& canvas, const Rect bounds, const MenuListView& view) noexcept {
    if (view.items == nullptr || view.itemCount == 0U || view.rowHeight == 0U) {
        return;
    }

    const std::int16_t scrollbarWidth = view.drawScrollbar && view.itemCount > 1U ? 3 : 0;
    const Rect listBounds{bounds.x,
                          bounds.y,
                          static_cast<std::int16_t>(bounds.width - scrollbarWidth),
                          bounds.height};

    const std::size_t visibleRows = visibleMenuRowCount(listBounds, view.rowHeight);
    if (visibleRows == 0U) {
        return;
    }

    const std::size_t first = ensureMenuSelectionVisible(
        view.selectedIndex, view.firstVisibleIndex, view.itemCount, visibleRows);
    const std::size_t end = std::min(view.itemCount, first + visibleRows);

    for (std::size_t index = first; index < end; ++index) {
        const std::size_t row = index - first;
        const Rect rowBounds{listBounds.x,
                             static_cast<std::int16_t>(
                                 listBounds.y + static_cast<std::int16_t>(row * view.rowHeight)),
                             listBounds.width,
                             static_cast<std::int16_t>(view.rowHeight)};

        const MenuItemView& item = view.items[index];
        const bool selected = index == view.selectedIndex;

        TextStyle labelStyle{};
        labelStyle.verticalAlignment = VerticalAlignment::Middle;
        labelStyle.mode = selected ? TextMode::Inverted : TextMode::Normal;

        TextStyle valueStyle = labelStyle;
        valueStyle.horizontalAlignment = HorizontalAlignment::Right;

        if (selected) {
            fillRectangle(canvas, rowBounds);
        }

        const PixelOperation foreground =
            item.enabled ? PixelOperation::Set : PixelOperation::Invert;
        labelStyle.foreground = foreground;
        valueStyle.foreground = foreground;

        drawText(canvas, inset(rowBounds, 1), item.label, labelStyle);
        drawText(canvas, inset(rowBounds, 1), item.value, valueStyle);
    }

    if (view.drawScrollbar && view.itemCount > visibleRows) {
        const Rect track{
            static_cast<std::int16_t>(bounds.x + bounds.width - 2), bounds.y, 2, bounds.height};
        drawRectangle(canvas, track);

        const std::int16_t thumbHeight = std::max<std::int16_t>(
            2,
            static_cast<std::int16_t>(static_cast<std::size_t>(track.height) * visibleRows /
                                      view.itemCount));
        const std::size_t maximumFirst = view.itemCount - visibleRows;
        const std::int16_t travel = toInt16(std::max<std::int32_t>(0, track.height - thumbHeight));
        const std::int16_t thumbOffset =
            maximumFirst == 0U ? 0
                               : toInt16(static_cast<std::int32_t>(
                                     static_cast<std::size_t>(travel) * first / maximumFirst));

        fillRectangle(
            canvas,
            {track.x, static_cast<std::int16_t>(track.y + thumbOffset), track.width, thumbHeight});
    }
}

void drawConfirmationOverlay(MonochromeCanvas& canvas,
                             const Rect bounds,
                             const ConfirmationOverlayView& view) noexcept {
    fillRectangle(canvas, bounds, PixelOperation::Clear);
    drawRectangle(canvas, bounds);

    const Rect inner = inset(bounds, 2);
    TextStyle titleStyle{};
    titleStyle.horizontalAlignment = HorizontalAlignment::Center;
    titleStyle.mode = TextMode::Inverted;

    drawText(canvas, {inner.x, inner.y, inner.width, 9}, view.title, titleStyle);

    const std::int16_t buttonsHeight = 10;
    drawWrappedMessage(canvas,
                       {inner.x,
                        static_cast<std::int16_t>(inner.y + 10),
                        inner.width,
                        toInt16(std::max<std::int32_t>(0, inner.height - 10 - buttonsHeight))},
                       view.message);

    const std::int16_t gap = 2;
    const std::int16_t buttonWidth = static_cast<std::int16_t>((inner.width - gap) / 2);
    const std::int16_t buttonY = static_cast<std::int16_t>(inner.y + inner.height - buttonsHeight);

    drawButton(canvas,
               {inner.x, buttonY, buttonWidth, buttonsHeight},
               "Yes",
               view.selected == ConfirmationChoice::Yes);

    drawButton(canvas,
               {static_cast<std::int16_t>(inner.x + buttonWidth + gap),
                buttonY,
                buttonWidth,
                buttonsHeight},
               "Cancel",
               view.selected == ConfirmationChoice::Cancel);
}

void drawErrorOverlay(MonochromeCanvas& canvas,
                      const Rect bounds,
                      const ErrorOverlayView& view) noexcept {
    fillRectangle(canvas, bounds, PixelOperation::Clear);
    drawRectangle(canvas, bounds);

    const Rect inner = inset(bounds, 2);
    const Rect iconBounds{inner.x, inner.y, 9, 9};
    drawGlyph(canvas, iconBounds, BasicGlyph::Cross);

    TextStyle titleStyle{};
    titleStyle.verticalAlignment = VerticalAlignment::Middle;
    titleStyle.mode = TextMode::Inverted;

    drawText(canvas,
             {static_cast<std::int16_t>(inner.x + 11),
              inner.y,
              static_cast<std::int16_t>(inner.width - 11),
              9},
             view.title,
             titleStyle);

    const std::int16_t buttonHeight = 10;
    drawWrappedMessage(canvas,
                       {inner.x,
                        static_cast<std::int16_t>(inner.y + 11),
                        inner.width,
                        toInt16(std::max<std::int32_t>(0, inner.height - 11 - buttonHeight))},
                       view.message);

    drawButton(canvas,
               {inner.x,
                static_cast<std::int16_t>(inner.y + inner.height - buttonHeight),
                inner.width,
                buttonHeight},
               view.actionLabel,
               true);
}

} // namespace eurorack::display

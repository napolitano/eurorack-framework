/**
 * @file include/eurorack/display/geometry.hpp
 * @brief Declares integer geometry types for display rendering.
 *
 * @details
 * Provides points, sizes, rectangles, clipping, and containment without platform dependencies.
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

namespace eurorack::display {

/**
 * @brief Integer point in display coordinates.
 */
struct Point final {
    std::int32_t x{0}; ///< Horizontal coordinate.
    std::int32_t y{0}; ///< Vertical coordinate.
};

/**
 * @brief Integer display size.
 */
struct Size final {
    std::int32_t width{0};  ///< Width in pixels.
    std::int32_t height{0}; ///< Height in pixels.
};

/**
 * @brief Integer rectangle with an exclusive right and bottom edge.
 */
struct Rect final {
    std::int32_t x{0};      ///< Left coordinate.
    std::int32_t y{0};      ///< Top coordinate.
    std::int32_t width{0};  ///< Width in pixels.
    std::int32_t height{0}; ///< Height in pixels.

    /**
     * @brief Returns the exclusive right edge.
     *
     * @return x plus width.
     */
    [[nodiscard]] constexpr std::int32_t right() const noexcept {
        return x + width;
    }

    /**
     * @brief Returns the exclusive bottom edge.
     *
     * @return y plus height.
     */
    [[nodiscard]] constexpr std::int32_t bottom() const noexcept {
        return y + height;
    }

    /**
     * @brief Reports whether the rectangle has a positive area.
     *
     * @return True when width and height are positive.
     */
    [[nodiscard]] constexpr bool valid() const noexcept {
        return width > 0 && height > 0;
    }

    /**
     * @brief Reports whether a point lies inside the rectangle.
     *
     * @param point Point to test.
     * @return True when the point lies within the rectangle.
     */
    [[nodiscard]] constexpr bool contains(const Point point) const noexcept {
        return valid() && point.x >= x && point.y >= y && point.x < right() && point.y < bottom();
    }
};

/**
 * @brief Calculates the intersection of two rectangles.
 *
 * @param first First rectangle.
 * @param second Second rectangle.
 * @return Intersection rectangle or an empty rectangle.
 */
[[nodiscard]] constexpr Rect intersect(const Rect first, const Rect second) noexcept {
    const std::int32_t left = first.x > second.x ? first.x : second.x;
    const std::int32_t top = first.y > second.y ? first.y : second.y;
    const std::int32_t right = first.right() < second.right() ? first.right() : second.right();
    const std::int32_t bottom = first.bottom() < second.bottom() ? first.bottom() : second.bottom();

    return {left, top, right > left ? right - left : 0, bottom > top ? bottom - top : 0};
}

} // namespace eurorack::display

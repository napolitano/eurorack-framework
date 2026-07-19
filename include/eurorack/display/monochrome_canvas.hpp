/**
 * @file include/eurorack/display/monochrome_canvas.hpp
 * @brief Declares a non-owning one-bit monochrome canvas.
 *
 * @details
 * Uses a row-major, MSB-first packed pixel buffer and explicit clipping without dynamic allocation.
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

namespace eurorack::display {

/**
 * @brief Operation applied to a monochrome pixel.
 */
enum class PixelOperation : std::uint8_t {
    Clear, ///< Force the pixel off.
    Set,   ///< Force the pixel on.
    Invert ///< Toggle the current pixel.
};

/**
 * @brief Non-owning one-bit canvas over a caller-provided buffer.
 *
 * Pixels are packed row-major and MSB-first. Each row occupies
 * `(width + 7) / 8` bytes. Unused bits at the end of a row are ignored.
 */
class MonochromeCanvas final {
  public:
    /**
     * @brief Calculates the minimum buffer size for a canvas.
     *
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @return Required bytes, or zero for invalid dimensions.
     */
    [[nodiscard]] static constexpr std::size_t
    requiredBufferSize(const std::int32_t width, const std::int32_t height) noexcept {
        return width > 0 && height > 0
                   ? static_cast<std::size_t>((width + 7) / 8) * static_cast<std::size_t>(height)
                   : 0U;
    }

    /**
     * @brief Constructs a canvas over caller-owned memory.
     *
     * @param buffer Writable pixel buffer.
     * @param bufferSize Available bytes in the buffer.
     * @param width Width in pixels.
     * @param height Height in pixels.
     */
    MonochromeCanvas(std::uint8_t* buffer,
                     std::size_t bufferSize,
                     std::int32_t width,
                     std::int32_t height) noexcept;

    /**
     * @brief Reports whether the canvas has a valid buffer and dimensions.
     *
     * @return True when drawing operations are available.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Returns the canvas width.
     *
     * @return Width in pixels.
     */
    [[nodiscard]] std::int32_t width() const noexcept;

    /**
     * @brief Returns the canvas height.
     *
     * @return Height in pixels.
     */
    [[nodiscard]] std::int32_t height() const noexcept;

    /**
     * @brief Returns the complete canvas bounds.
     *
     * @return Rectangle from zero to width and height.
     */
    [[nodiscard]] Rect bounds() const noexcept;

    /**
     * @brief Returns the active clipping rectangle.
     *
     * @return Current clipping rectangle.
     */
    [[nodiscard]] Rect clipRect() const noexcept;

    /**
     * @brief Restricts drawing to the intersection with canvas bounds.
     *
     * @param rectangle Requested clipping rectangle.
     */
    void setClipRect(Rect rectangle) noexcept;

    /**
     * @brief Restores clipping to the complete canvas.
     */
    void resetClipRect() noexcept;

    /**
     * @brief Clears every pixel in the backing buffer.
     */
    void clear() noexcept;

    /**
     * @brief Sets every valid pixel in the backing buffer.
     */
    void fill() noexcept;

    /**
     * @brief Inverts every valid pixel in the backing buffer.
     */
    void invert() noexcept;

    /**
     * @brief Applies an operation to one pixel when it lies inside the clip.
     *
     * @param point Pixel coordinate.
     * @param operation Operation to apply.
     * @return True when the pixel was inside the active clip and modified.
     */
    bool applyPixel(Point point, PixelOperation operation) noexcept;

    /**
     * @brief Reads one pixel independent of the active clipping rectangle.
     *
     * @param point Pixel coordinate.
     * @return True when the pixel is on; false when off or out of bounds.
     */
    [[nodiscard]] bool pixel(Point point) const noexcept;

    /**
     * @brief Returns the mutable packed buffer.
     *
     * @return Pointer to caller-owned storage.
     */
    [[nodiscard]] std::uint8_t* data() noexcept;

    /**
     * @brief Returns the immutable packed buffer.
     *
     * @return Pointer to caller-owned storage.
     */
    [[nodiscard]] const std::uint8_t* data() const noexcept;

    /**
     * @brief Returns the number of active bytes used by the canvas.
     *
     * @return Packed buffer size in bytes.
     */
    [[nodiscard]] std::size_t dataSize() const noexcept;

    /**
     * @brief Returns bytes occupied by one packed row.
     *
     * @return Row stride in bytes.
     */
    [[nodiscard]] std::size_t rowStride() const noexcept;

  private:
    /**
     * @brief Returns packed storage coordinates for one valid point.
     *
     * @param point Point known to be inside canvas bounds.
     * @param byteIndex Receives byte index.
     * @param mask Receives bit mask.
     */
    void locate(Point point, std::size_t& byteIndex, std::uint8_t& mask) const noexcept;

    std::uint8_t* buffer_{nullptr}; ///< Caller-owned packed pixel storage; never allocated or
                                      ///< freed by this class.
    std::size_t bufferSize_{0U};    ///< Bytes available at `buffer_`, as supplied to the
                                      ///< constructor.
    std::int32_t width_{0};  ///< Canvas width in pixels; forced to `0` when the constructor
                               ///< rejects the buffer as too small or null.
    std::int32_t height_{0}; ///< Canvas height in pixels; forced to `0` under the same condition
                               ///< as `width_`.
    std::size_t rowStride_{0U}; ///< Bytes per packed row, `(width_ + 7) / 8`; `0` when invalid.
    Rect clip_{};                ///< Active clipping rectangle, always contained within canvas
                                   ///< bounds.
    bool valid_{false}; ///< True when `buffer_` is non-null and `bufferSize_` is at least
                         ///< `requiredBufferSize(width, height)` as evaluated by the constructor.
};

} // namespace eurorack::display

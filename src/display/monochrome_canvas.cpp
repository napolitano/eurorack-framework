/**
 * @file src/display/monochrome_canvas.cpp
 * @brief Implements the one-bit monochrome canvas.
 *
 * @details
 * Provides packed pixel access, clipping, clearing, filling, and inversion.
 *
 * The canvas is a non-owning view over caller-provided storage. Pixels are
 * packed horizontally with the most-significant bit representing the leftmost
 * pixel of each byte. Every coordinate-based operation validates and clips
 * before calculating unsigned offsets.
 *
 * Construction never clears or allocates the supplied buffer. The caller must
 * keep it alive for the complete canvas lifetime.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <algorithm>
#include <cstring>
#include <eurorack/display/monochrome_canvas.hpp>

namespace eurorack::display {

MonochromeCanvas::MonochromeCanvas(std::uint8_t* const buffer,
                                   const std::size_t bufferSize,
                                   const std::int32_t width,
                                   const std::int32_t height) noexcept
    : buffer_(buffer), bufferSize_(bufferSize), width_(width), height_(height),
      rowStride_(width > 0 ? static_cast<std::size_t>((width + 7) / 8) : 0U) {
    const std::size_t required = requiredBufferSize(width, height);
    valid_ = buffer_ != nullptr && required > 0U && bufferSize_ >= required;

    if (!valid_) {
        width_ = 0;
        height_ = 0;
        rowStride_ = 0U;
    }

    resetClipRect();
}

bool MonochromeCanvas::valid() const noexcept {
    return valid_;
}

std::int32_t MonochromeCanvas::width() const noexcept {
    return width_;
}

std::int32_t MonochromeCanvas::height() const noexcept {
    return height_;
}

Rect MonochromeCanvas::bounds() const noexcept {
    return {0, 0, width_, height_};
}

Rect MonochromeCanvas::clipRect() const noexcept {
    return clip_;
}

void MonochromeCanvas::setClipRect(const Rect rectangle) noexcept {
    clip_ = intersect(bounds(), rectangle);
}

void MonochromeCanvas::resetClipRect() noexcept {
    clip_ = bounds();
}

void MonochromeCanvas::clear() noexcept {
    if (valid_) {
        std::memset(buffer_, 0, dataSize());
    }
}

void MonochromeCanvas::fill() noexcept {
    if (!valid_) {
        return;
    }

    std::memset(buffer_, 0xFF, dataSize());

    const std::uint8_t usedBits = static_cast<std::uint8_t>(width_ % 8);
    if (usedBits == 0U) {
        return;
    }

    const std::uint8_t validMask =
        static_cast<std::uint8_t>(0xFFU << static_cast<std::uint8_t>(8U - usedBits));

    for (std::int32_t y = 0; y < height_; ++y) {
        const std::size_t lastByte = static_cast<std::size_t>(y) * rowStride_ + rowStride_ - 1U;
        buffer_[lastByte] = validMask;
    }
}

void MonochromeCanvas::invert() noexcept {
    if (!valid_) {
        return;
    }

    for (std::size_t index = 0U; index < dataSize(); ++index) {
        buffer_[index] = static_cast<std::uint8_t>(~buffer_[index]);
    }

    const std::uint8_t usedBits = static_cast<std::uint8_t>(width_ % 8);
    if (usedBits == 0U) {
        return;
    }

    const std::uint8_t validMask =
        static_cast<std::uint8_t>(0xFFU << static_cast<std::uint8_t>(8U - usedBits));

    for (std::int32_t y = 0; y < height_; ++y) {
        const std::size_t lastByte = static_cast<std::size_t>(y) * rowStride_ + rowStride_ - 1U;
        buffer_[lastByte] = static_cast<std::uint8_t>(buffer_[lastByte] & validMask);
    }
}

bool MonochromeCanvas::applyPixel(const Point point, const PixelOperation operation) noexcept {
    if (!valid_ || !clip_.contains(point)) {
        return false;
    }

    std::size_t byteIndex = 0U;
    std::uint8_t mask = 0U;
    locate(point, byteIndex, mask);

    switch (operation) {
    case PixelOperation::Clear:
        buffer_[byteIndex] = static_cast<std::uint8_t>(buffer_[byteIndex] & ~mask);
        break;
    case PixelOperation::Set:
        buffer_[byteIndex] = static_cast<std::uint8_t>(buffer_[byteIndex] | mask);
        break;
    case PixelOperation::Invert:
        buffer_[byteIndex] = static_cast<std::uint8_t>(buffer_[byteIndex] ^ mask);
        break;
    }

    return true;
}

bool MonochromeCanvas::pixel(const Point point) const noexcept {
    if (!valid_ || !bounds().contains(point)) {
        return false;
    }

    std::size_t byteIndex = 0U;
    std::uint8_t mask = 0U;
    locate(point, byteIndex, mask);
    return (buffer_[byteIndex] & mask) != 0U;
}

std::uint8_t* MonochromeCanvas::data() noexcept {
    return buffer_;
}

const std::uint8_t* MonochromeCanvas::data() const noexcept {
    return buffer_;
}

std::size_t MonochromeCanvas::dataSize() const noexcept {
    return valid_ ? rowStride_ * static_cast<std::size_t>(height_) : 0U;
}

std::size_t MonochromeCanvas::rowStride() const noexcept {
    return rowStride_;
}

void MonochromeCanvas::locate(const Point point,
                              std::size_t& byteIndex,
                              std::uint8_t& mask) const noexcept {
    byteIndex =
        static_cast<std::size_t>(point.y) * rowStride_ + static_cast<std::size_t>(point.x / 8);
    const std::uint8_t bit = static_cast<std::uint8_t>(point.x % 8);
    mask = static_cast<std::uint8_t>(0x80U >> bit);
}

} // namespace eurorack::display

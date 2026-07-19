/**
 * @file include/eurorack/compat/avr/string_view.hpp
 * @brief Provides a minimal, non-owning `std::string_view` on AVR.
 *
 * @details
 * Covers construction from a null-terminated C string or from nothing
 * (empty view), `size()`, `empty()`, `data()`, `operator[]`, pointer-based
 * `begin()`/`end()` for range-based `for`, `substr()`, and `find_last_of()`,
 * matching every framework call site (`eurorack::display::text`,
 * `eurorack::display::widgets`) at the time this header was written. This is
 * not a general-purpose `std::string_view` replacement: comparison operators,
 * hashing, and most other members are intentionally omitted. See
 * `eurorack/compat/avr/cstdint.hpp` for background. Do not include this
 * header directly; include `<string_view>` as usual.
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
 * @ingroup compat_avr
 */

#pragma once

#include <eurorack/compat/avr/cstddef.hpp>

namespace std {

/**
 * @brief Minimal non-owning view over a contiguous character sequence.
 *
 * @details
 * Covers exactly the operations the framework's display subsystem uses. The referenced
 * character sequence must outlive the view; this class never copies or owns text.
 */
class string_view final {
  public:
    /** @brief Sentinel returned by `find_last_of` when no match exists. */
    static constexpr size_t npos = static_cast<size_t>(-1);

    /** @brief Constructs an empty view. */
    constexpr string_view() noexcept = default;

    /**
     * @brief Constructs a view over a null-terminated string.
     *
     * @param text Null-terminated string; may be null, which constructs an empty view.
     */
    constexpr string_view(const char* text) noexcept : data_(text), size_(lengthOf(text)) {}

    /**
     * @brief Constructs a view over an explicit character range.
     *
     * @param text Pointer to the first character.
     * @param length Number of characters in the view.
     */
    constexpr string_view(const char* text, const size_t length) noexcept
        : data_(text), size_(length) {}

    /**
     * @brief Returns a pointer to the viewed characters.
     *
     * @return Pointer to the first character; not necessarily null-terminated.
     */
    [[nodiscard]] constexpr const char* data() const noexcept {
        return data_;
    }

    /**
     * @brief Returns the number of viewed characters.
     *
     * @return Character count.
     */
    [[nodiscard]] constexpr size_t size() const noexcept {
        return size_;
    }

    /**
     * @brief Reports whether the view has zero length.
     *
     * @return True when `size() == 0`.
     */
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size_ == 0U;
    }

    /**
     * @brief Accesses one character without bounds checking.
     *
     * @param index Zero-based character index.
     * @return The character at `index`.
     */
    constexpr char operator[](const size_t index) const noexcept {
        return data_[index];
    }

    /**
     * @brief Returns an iterator to the first character.
     *
     * @return Pointer to the first character.
     */
    constexpr const char* begin() const noexcept {
        return data_;
    }

    /**
     * @brief Returns an iterator one past the last character.
     *
     * @return Pointer one past the last character.
     */
    constexpr const char* end() const noexcept {
        return data_ + size_;
    }

    /**
     * @brief Returns a view over a sub-range of this view.
     *
     * @param position Starting character index; a value at or beyond `size()` yields an empty
     * view.
     * @param length Maximum number of characters in the result; clamped to the characters
     * actually available from `position`. Defaults to the remainder of the view.
     * @return View over the requested sub-range.
     */
    [[nodiscard]] constexpr string_view substr(const size_t position,
                                               const size_t length = npos) const noexcept {
        if (position >= size_) {
            return string_view(data_ + size_, 0U);
        }

        const size_t available = size_ - position;
        const size_t clampedLength = length < available ? length : available;

        return string_view(data_ + position, clampedLength);
    }

    /**
     * @brief Finds the last occurrence of a character.
     *
     * @param character Character to search for.
     * @return Index of the last matching character, or `npos` if none is found.
     */
    [[nodiscard]] constexpr size_t find_last_of(const char character) const noexcept {
        for (size_t index = size_; index > 0U; --index) {
            if (data_[index - 1U] == character) {
                return index - 1U;
            }
        }

        return npos;
    }

  private:
    /**
     * @brief Computes the length of a null-terminated string.
     *
     * @param text Null-terminated string; may be null.
     * @return Number of characters before the terminating null, or `0` if `text` is null.
     */
    static constexpr size_t lengthOf(const char* text) noexcept {
        size_t length = 0U;

        if (text != nullptr) {
            while (text[length] != '\0') {
                ++length;
            }
        }

        return length;
    }

    const char* data_{nullptr}; ///< Pointer to the first viewed character, or null when empty.
    size_t size_{0U};           ///< Number of viewed characters.
};

} // namespace std

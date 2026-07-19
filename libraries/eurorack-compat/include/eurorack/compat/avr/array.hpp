/**
 * @file include/eurorack/compat/avr/array.hpp
 * @brief Provides a minimal fixed-size `std::array` on AVR.
 *
 * @details
 * Covers construction, `operator[]`, `data()`, `size()`, and pointer-based
 * `begin()`/`end()` iteration, matching every framework call site at the time
 * this header was written. Zero-length arrays are given one element of
 * backing storage, matching the standard's requirement that `data()` remain a
 * valid, distinct pointer even when `size() == 0`. See
 * `eurorack/compat/avr/cstdint.hpp` for background. Do not include this
 * header directly; include `<array>` as usual.
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
 * @brief Minimal fixed-size array with `std::array`'s common interface.
 *
 * @tparam T Element type.
 * @tparam N Element count. `N == 0` still reserves one element of backing storage so `data()`
 * remains a valid, distinct pointer.
 */
template <typename T, size_t N> struct array {
    T elements[N == 0 ? 1 : N]; ///< Backing storage.

    /**
     * @brief Accesses one element without bounds checking.
     *
     * @param index Zero-based element index.
     * @return Reference to the element.
     */
    constexpr T& operator[](const size_t index) noexcept {
        return elements[index];
    }

    /**
     * @brief Accesses one element without bounds checking.
     *
     * @param index Zero-based element index.
     * @return Constant reference to the element.
     */
    constexpr const T& operator[](const size_t index) const noexcept {
        return elements[index];
    }

    /**
     * @brief Returns a pointer to the backing storage.
     *
     * @return Pointer to the first element.
     */
    constexpr T* data() noexcept {
        return elements;
    }

    /**
     * @brief Returns a pointer to the backing storage.
     *
     * @return Constant pointer to the first element.
     */
    constexpr const T* data() const noexcept {
        return elements;
    }

    /**
     * @brief Returns the element count.
     *
     * @return `N`.
     */
    constexpr size_t size() const noexcept {
        return N;
    }

    /**
     * @brief Returns an iterator to the first element.
     *
     * @return Pointer to the first element.
     */
    constexpr T* begin() noexcept {
        return elements;
    }

    /**
     * @brief Returns an iterator to the first element.
     *
     * @return Constant pointer to the first element.
     */
    constexpr const T* begin() const noexcept {
        return elements;
    }

    /**
     * @brief Returns an iterator one past the last element.
     *
     * @return Pointer one past the last element.
     */
    constexpr T* end() noexcept {
        return elements + N;
    }

    /**
     * @brief Returns an iterator one past the last element.
     *
     * @return Constant pointer one past the last element.
     */
    constexpr const T* end() const noexcept {
        return elements + N;
    }
};

} // namespace std

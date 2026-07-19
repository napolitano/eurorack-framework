/**
 * @file include/eurorack/compat/avr/cstring.hpp
 * @brief Provides `std::memset`, `std::memcpy`, and `std::memcmp` on AVR.
 *
 * @details
 * Covers exactly the functions used across the framework's implementation files. See
 * `eurorack/compat/avr/cstdint.hpp` for background. Do not include this header directly; include
 * `<cstring>` as usual.
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
#include <string.h>

namespace std {

/**
 * @brief Fills a memory region with a byte value.
 *
 * @param destination Start of the region to fill.
 * @param value Byte value to write, truncated to `unsigned char`.
 * @param count Number of bytes to write.
 * @return `destination`.
 */
inline void* memset(void* const destination, const int value, const size_t count) noexcept {
    return ::memset(destination, value, count);
}

/**
 * @brief Copies a block of memory between two non-overlapping regions.
 *
 * @param destination Destination region; must not overlap `source`.
 * @param source Source region.
 * @param count Number of bytes to copy.
 * @return `destination`.
 */
inline void*
memcpy(void* const destination, const void* const source, const size_t count) noexcept {
    return ::memcpy(destination, source, count);
}

/**
 * @brief Compares two memory regions byte by byte.
 *
 * @param first First region.
 * @param second Second region.
 * @param count Number of bytes to compare.
 * @return Zero when the regions are identical for `count` bytes; a negative or positive value
 * matching the sign of the first differing byte pair otherwise.
 */
inline int memcmp(const void* const first, const void* const second, const size_t count) noexcept {
    return ::memcmp(first, second, count);
}

} // namespace std

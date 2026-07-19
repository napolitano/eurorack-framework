/**
 * @file include/eurorack/compat/avr/cstdio.hpp
 * @brief Provides `std::snprintf` on AVR.
 *
 * @details
 * Covers exactly the function used across the framework's implementation files. avr-libc's
 * `vsnprintf` backend does not format floating-point values unless the firmware links an
 * explicit `libprintf_flt` variant; the framework's only `snprintf` call site formats strings
 * only, so this is not a limitation for current framework code. See
 * `eurorack/compat/avr/cstdint.hpp` for background. Do not include this header directly; include
 * `<cstdio>` as usual.
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
#include <stdarg.h>
#include <stdio.h>

namespace std {

/**
 * @brief Writes a formatted string into a bounded buffer.
 *
 * @param buffer Destination buffer.
 * @param bufferSize Destination buffer capacity in bytes, including the terminating null.
 * @param format `printf`-style format string.
 * @param ... Arguments matching `format`.
 * @return Number of characters that would have been written, excluding the terminating null,
 * had `bufferSize` been unlimited; matches `::snprintf`'s return value.
 */
inline int
snprintf(char* const buffer, const size_t bufferSize, const char* const format, ...) noexcept {
    va_list arguments;
    va_start(arguments, format);
    const int result = ::vsnprintf(buffer, bufferSize, format, arguments);
    va_end(arguments);
    return result;
}

} // namespace std

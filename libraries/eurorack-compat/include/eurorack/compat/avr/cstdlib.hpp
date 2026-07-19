/**
 * @file include/eurorack/compat/avr/cstdlib.hpp
 * @brief Provides `std::abs` for integers on AVR.
 *
 * @details
 * Covers exactly the overload used across the framework's implementation files
 * (`std::int32_t`). See `eurorack/compat/avr/cstdint.hpp` for background. Do not include this
 * header directly; include `<cstdlib>` as usual.
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

#include <eurorack/compat/avr/cstdint.hpp>
#include <stdlib.h>

namespace std {

/**
 * @brief Returns the absolute value of a 32-bit signed integer.
 *
 * @param value Input value.
 * @return `value` with its sign removed.
 */
inline int32_t abs(const int32_t value) noexcept {
    return value < 0 ? -value : value;
}

} // namespace std

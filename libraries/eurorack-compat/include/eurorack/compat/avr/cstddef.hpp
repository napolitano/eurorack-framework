/**
 * @file include/eurorack/compat/avr/cstddef.hpp
 * @brief Provides `std::size_t`, `std::ptrdiff_t`, and `std::nullptr_t` on AVR.
 *
 * @details
 * See `eurorack/compat/avr/cstdint.hpp` for why this exists. Do not include
 * this header directly; include `<cstddef>` as usual.
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

#include <stddef.h>

namespace std {

using ::ptrdiff_t;
using ::size_t;

/** @brief Type of the null pointer literal `nullptr`. */
using nullptr_t = decltype(nullptr);

} // namespace std

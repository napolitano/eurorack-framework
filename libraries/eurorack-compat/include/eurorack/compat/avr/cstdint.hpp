/**
 * @file include/eurorack/compat/avr/cstdint.hpp
 * @brief Provides `std::` fixed-width integer aliases on AVR toolchains.
 *
 * @details
 * The AVR-GCC toolchain (as packaged for Arduino, PlatformIO, and plain
 * avr-gcc) ships no C++ standard library headers at all, including
 * `<cstdint>`, even though the compiler itself accepts `-std=gnu++17`. This
 * header re-exposes the C `<stdint.h>` typedefs already provided by avr-libc
 * under the `std::` namespace, which is all the framework actually needs from
 * `<cstdint>`.
 *
 * Do not include this header directly from application or framework code;
 * include `<cstdint>` as usual and let the framework's internal
 * `EURORACK_CSTDINT` guard select this file automatically on `__AVR__`.
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

#include <stdint.h>

namespace std {

using ::int16_t;
using ::int32_t;
using ::int64_t;
using ::int8_t;
using ::uint16_t;
using ::uint32_t;
using ::uint64_t;
using ::uint8_t;

using ::int_least16_t;
using ::int_least32_t;
using ::int_least64_t;
using ::int_least8_t;
using ::uint_least16_t;
using ::uint_least32_t;
using ::uint_least64_t;
using ::uint_least8_t;

using ::int_fast16_t;
using ::int_fast32_t;
using ::int_fast64_t;
using ::int_fast8_t;
using ::uint_fast16_t;
using ::uint_fast32_t;
using ::uint_fast64_t;
using ::uint_fast8_t;

using ::intmax_t;
using ::intptr_t;
using ::uintmax_t;
using ::uintptr_t;

} // namespace std

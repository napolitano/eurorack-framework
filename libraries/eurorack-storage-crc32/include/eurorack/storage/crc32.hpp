/**
 * @file include/eurorack/storage/crc32.hpp
 * @brief Declares CRC-32 calculation for persisted records.
 *
 * @details
 * Uses the standard reflected polynomial 0xEDB88320.
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
 * @ingroup storage
 */

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <cstddef>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif

namespace eurorack::storage {

/**
 * @brief Calculates a CRC-32 checksum using the reflected polynomial 0xEDB88320.
 *
 * @details
 * Processes each byte bit-by-bit without a lookup table. `initial` seeds the running
 * accumulator before processing; the returned value is the one's complement of the accumulator
 * after all bytes are processed, matching the standard CRC-32 algorithm when `initial` is left
 * at its default. Returns `0` if `data` is null while `size` is non-zero.
 *
 * @param data Source byte sequence; may be null only if `size` is zero.
 * @param size Number of bytes to process.
 * @param initial Accumulator seed before processing; defaults to the standard CRC-32 initial
 * value.
 * @return CRC-32 value.
 */
[[nodiscard]] std::uint32_t
crc32(const std::uint8_t* data, std::size_t size, std::uint32_t initial = 0xFFFFFFFFU) noexcept;

} // namespace eurorack::storage

/**
 * @file src/storage/crc32.cpp
 * @brief Implements CRC-32 calculation.
 *
 * @details
 * Uses the standard reflected polynomial 0xEDB88320 without lookup tables.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/storage/crc32.hpp>

namespace eurorack::storage {

std::uint32_t crc32(const std::uint8_t* const data,
                    const std::size_t size,
                    const std::uint32_t initial) noexcept {
    if (data == nullptr && size > 0U) {
        return 0U;
    }

    std::uint32_t crc = initial;

    for (std::size_t index = 0U; index < size; ++index) {
        crc ^= data[index];

        for (std::uint8_t bit = 0U; bit < 8U; ++bit) {
            const std::uint32_t mask = 0U - static_cast<std::uint32_t>(crc & 1U);
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }

    return ~crc;
}

} // namespace eurorack::storage

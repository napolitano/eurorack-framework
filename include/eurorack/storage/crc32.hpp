/**
 * @file include/eurorack/storage/crc32.hpp
 * @brief Declares CRC-32 calculation for persisted records.
 *
 * @details
 * Uses the standard reflected polynomial 0xEDB88320.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup storage
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace eurorack::storage {

/**
 * @brief Calculates CRC-32 over a byte sequence.
 *
 * @param data Source byte sequence.
 * @param size Number of bytes.
 * @param initial Initial CRC value before final inversion.
 * @return CRC-32 value.
 */
[[nodiscard]] std::uint32_t
/**
 * @brief Calculates a CRC-32 checksum.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param data Byte buffer used by the operation.
 *
 * @param size Number of bytes or elements involved.
 *
 * @param initial Initial CRC accumulator value.
 */
crc32(const std::uint8_t* data, std::size_t size, std::uint32_t initial = 0xFFFFFFFFU) noexcept;

} // namespace eurorack::storage

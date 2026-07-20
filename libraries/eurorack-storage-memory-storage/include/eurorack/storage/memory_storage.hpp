/**
 * @file include/eurorack/storage/memory_storage.hpp
 * @brief Declares an in-memory persistent-storage backend.
 *
 * @details
 * Provides deterministic storage for native tests, simulators, and fault injection.
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
#include <eurorack/storage/persistent_storage.hpp>
#include <vector>

namespace eurorack::storage {

/**
 * @brief Memory-backed persistent storage for tests and simulation.
 */
class MemoryStorage final : public PersistentStorage {
  public:
    /**
     * @brief Constructs a memory-backed storage region.
     *
     * @param capacityBytes Number of addressable bytes.
     * @param erasedValue Byte value used after erase.
     */
    explicit MemoryStorage(std::size_t capacityBytes, std::uint8_t erasedValue = 0xFFU);

    /**
     * @brief Returns total addressable storage capacity.
     *
     * @return Capacity in bytes.
     */
    [[nodiscard]] std::size_t capacity() const noexcept override;

    /**
     * @brief Reads bytes from storage.
     *
     * @param address Starting byte address.
     * @param destination Destination buffer.
     * @param size Number of bytes to read.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    eurorack::io::IoResult
    read(std::size_t address, std::uint8_t* destination, std::size_t size) const noexcept override;

    /**
     * @brief Writes bytes to storage.
     *
     * @param address Starting byte address.
     * @param source Source buffer.
     * @param size Number of bytes to write.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    eurorack::io::IoResult
    write(std::size_t address, const std::uint8_t* source, std::size_t size) noexcept override;

    /**
     * @brief Erases bytes to the configured erase value.
     *
     * @param address Starting byte address.
     * @param size Number of bytes to erase.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    eurorack::io::IoResult erase(std::size_t address, std::size_t size) noexcept override;

    /**
     * @brief Commits pending writes.
     *
     * @return Always Success because memory writes are immediate.
     */
    eurorack::io::IoResult commit() noexcept override;

    /**
     * @brief Returns the configured erase value.
     *
     * @return Erased byte value.
     */
    [[nodiscard]] std::uint8_t erasedValue() const noexcept override;

    /**
     * @brief Returns direct read-only access to stored bytes.
     *
     * @return Constant reference to the backing buffer.
     */
    [[nodiscard]] const std::vector<std::uint8_t>& bytes() const noexcept;

  private:
    /**
     * @brief Validates a requested byte range.
     *
     * @param address Starting byte address.
     * @param size Number of bytes.
     * @return True when the complete range is valid.
     */
    [[nodiscard]] bool validRange(std::size_t address, std::size_t size) const noexcept;

    std::vector<std::uint8_t> bytes_{}; ///< Backing storage; sized to `capacityBytes` at
                                        ///< construction.
    std::uint8_t erasedValue_{0xFFU};   ///< Byte value written by `erase` and returned by
                                        ///< `erasedValue`.
};

} // namespace eurorack::storage
